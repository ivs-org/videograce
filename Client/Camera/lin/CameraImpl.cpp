/**
* Camera.cpp - Contains camera's impl
*
* Author: Anton (ud) Golovkov, udattsk@gmail.com
* Copyright (C), Infinity Video Soft LLC, 2014
*/

#include <Camera/lin/CameraImpl.h>

#include <Transport/RTP/RTPPacket.h>
#include <assert.h>

#include <wui/config/config.hpp>

#include <Common/ShortSleep.h>

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <ippcc.h>
#include <ippi.h>

#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

namespace Camera
{

CameraImpl::CameraImpl(Common::TimeMeter &timeMeter_, Transport::ISocket &receiver_)
	: timeMeter(timeMeter_), receiver(receiver_),
    buffer(), tmpBuffer(),
    packetDuration(40000),
	deviceNotifyCallback(),
	name(),
	deviceId(0),
	resolution(Video::rVGA),
	ssrc(0), seq(0),
	runned(false),
	thread(),
	processTime(0),
	overTimeCount(0),
	fd(0),
	buffers(),
	n_buffers(0),
	flipHorizontal(false),
	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{

}

CameraImpl::~CameraImpl()
{
	Stop();
}

void CameraImpl::SetDeviceNotifyCallback(Client::DeviceNotifyCallback deviceNotifyCallback_)
{
    deviceNotifyCallback_ = deviceNotifyCallback_;
}

void CameraImpl::SetName(std::string_view name_)
{
	name = name_;
	if (runned)
	{
		Stop();
		Start(Video::ColorSpace::YUY2, ssrc);
	}
}

void CameraImpl::SetDeviceId(uint32_t id)
{
	deviceId = id;
}

void CameraImpl::Move(MoveAxis axis, MoveType type, int value)
{

}

void CameraImpl::Zoom(int value)
{

}

void CameraImpl::Start(Video::ColorSpace, ssrc_t ssrc_)
{
	if (!runned)
	{
		ssrc = ssrc_;
		seq = 0;

		flipHorizontal = wui::config::get_int("Camera", "FlipHorizontal", 0) != 0;

		Video::ResolutionValues rv = Video::GetValues(resolution);
		size_t bufferSize = rv.width * rv.height * 1.5;
		buffer = std::unique_ptr<uint8_t[]>(new (std::nothrow) uint8_t[bufferSize]);

		if (flipHorizontal)
		{
			tmpBuffer = std::unique_ptr<uint8_t[]>(new (std::nothrow) uint8_t[rv.width * rv.height * 2]);
		}

		runned = true;
		thread = std::thread(&CameraImpl::run, this);
	}
}

void CameraImpl::Stop()
{
	if (runned)
	{
		runned = false;
		thread.join();

		buffer.reset(nullptr);
		tmpBuffer.reset(nullptr);
	}
}

bool CameraImpl::SetResolution(Video::Resolution resolution_)
{
	if (runned)
	{
		Stop();

		resolution = resolution_;

		Start(Video::ColorSpace::YUY2, ssrc);
	}

	resolution = resolution_;

	return true;
}

void CameraImpl::SetFrameRate(uint32_t rate)
{
    packetDuration = (1000 / rate) * 1000;
}

void CameraImpl::run()
{
	int ret = open_device();
	if (ret != 0)
	{
		return;
	}
	ret = init_device();
	if (ret != 0)
	{
		return;
	}

	Video::ResolutionValues rv = Video::GetValues(resolution);

	ret = start_capturing();
	if (ret != 0)
	{
		return;
	}

	const uint64_t APPROACH = 200;

	while (runned)
	{
		uint32_t startTime = timeMeter.Measure();
        while (runned && processTime + APPROACH < packetDuration && timeMeter.Measure() - startTime < packetDuration - processTime - APPROACH)
        {
            Common::ShortSleep();
        }

		fd_set fds;
		int r = 0;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select(fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r)
		{
			if (EINTR == errno)
			{
				continue;
			}
		}

		if (0 == r)
		{
			continue;
		}

		uint32_t sendTime = timeMeter.Measure();

		int idx = read_frame();
		if (idx != -1)
		{
			Postprocess((uint8_t*)buffers[idx].data, rv.width * rv.height * 2);
			Transport::RTPPacket packet;
			packet.rtpHeader.ts = timeMeter.Measure() / 1000;
			packet.rtpHeader.ssrc = ssrc;
			packet.rtpHeader.seq = ++seq;
            packet.payload = buffer.get();
            packet.payloadSize = (rv.width * rv.height) + ((rv.width * rv.height) / 2);
            receiver.Send(packet);
        }

		processTime = timeMeter.Measure() - sendTime;

        if (processTime > packetDuration + 5000)
        {
            ++overTimeCount;
        }
		else if (overTimeCount > 0)
		{
			--overTimeCount;
		}
		if (overTimeCount == 100)
		{
			overTimeCount = 10000; // prevent duplicates
			if (deviceNotifyCallback)
			{
				deviceNotifyCallback(name, Client::DeviceNotifyType::OvertimeCoding, Proto::DeviceType::Camera, 0, 0);
			}
		}
	}

	stop_capturing();
	uninit_device();
	close_device();
}

void CameraImpl::Postprocess(const uint8_t* data, int len)
{
	// convert to I420
	const Video::ResolutionValues rv = Video::GetValues(resolution);

	const IppiSize  sz = { rv.width, rv.height };

	if (!flipHorizontal)
	{
		Ipp8u*          dst[3] = { buffer.get(), buffer.get() + (rv.width * rv.height) + ((rv.width * rv.height) / 4), buffer.get() + (rv.width * rv.height) };
		int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };
		ippiYCbCr422ToYCrCb420_8u_C2P3R(data, rv.width * 2, dst, dstStep, sz);
	}
	else
	{
		FlipHorizontal(data);

		Ipp8u*          dst[3] = { buffer.get(), buffer.get() + (rv.width * rv.height) + ((rv.width * rv.height) / 4), buffer.get() + (rv.width * rv.height) };
		int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };
		ippiYCbCr422ToYCrCb420_8u_C2P3R(tmpBuffer.get(), rv.width * 2, dst, dstStep, sz);
	}
}

void CameraImpl::FlipHorizontal(const uint8_t* data)
{
	const Video::ResolutionValues rv = Video::GetValues(resolution);

	int x = 0;
	for (int n = rv.width * rv.height * 2 - 1; n >= 3; n -= 4)
	{
		tmpBuffer.get()[x] = data[n - 1]; // Y1->Y0
		tmpBuffer.get()[x + 1] = data[n - 2]; // U
		tmpBuffer.get()[x + 2] = data[n - 3]; // Y0->Y1
		tmpBuffer.get()[x + 3] = data[n]; // V

	    x += 4;
	}
}

static int xioctl(int fh, unsigned long int request, void *arg)
{
	int r;

	do
	{
		r = ioctl(fh, request, arg);
	}
	while (-1 == r && EINTR == errno);

	return r;
}

bool CameraImpl::read_frame()
{
	struct v4l2_buffer buf;

	CLEAR(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
	{
		switch (errno)
		{
			case EAGAIN:
				errLog->critical("Camera {0} read_frame() error EAGAIN", name);
				return -1;
			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */
			default:
				errLog->critical("Camera {0} read_frame() error {1}", name, errno);
				return -1;
			break;
		}
	}

	assert(buf.index < n_buffers);

	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
	{
		errLog->critical("Camera {0} read_frame() error -1 == xioctl(fd, VIDIOC_QBUF, &buf)", name);
		return -1;
	}

	return buf.index;
}

int CameraImpl::open_device()
{
	struct stat st;

	if (-1 == stat(name.c_str(), &st))
	{
		errLog->critical("Camera {0} open_device() error -1 == stat(name, &st", name);
		return -1;
	}

	if (!S_ISCHR(st.st_mode))
	{
		errLog->critical("Camera {0} open_device() error !S_ISCHR(st.st_mode)", name);
		return -2;
	}

	fd = open(name.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == fd)
	{
		errLog->critical("Camera {0} open_device() error -1 == fd", name);
		return -3;
	}

	return 0;
}

void CameraImpl::close_device()
{
	if (-1 == close(fd))
	{
		errLog->critical("Camera {0} close_device() error", name);
	}

	fd = -1;
}

int CameraImpl::init_device()
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			errLog->critical("Camera {0} init_device() error EINVAL", name);
			return -1;
		}
		else
		{
			errLog->critical("Camera {0} init_device() error {1}", name, errno);
			return -2;
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		errLog->critical("Camera {0} init_device() error cap.capabilities & V4L2_CAP_VIDEO_CAPTURE", name);
		return -3;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		errLog->critical("Camera {0} init_device() error cap.capabilities & V4L2_CAP_STREAMING", name);
		return -4;
	}

	/* Select video input, video standard and tune here. */

	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
		{
			switch (errno)
			{
				case EINVAL:
					/* Cropping not supported. */
					break;
				default:
					/* Errors ignored. */
					break;
			}
		}
	}
	else
	{
		/* Errors ignored. */
	}

	CLEAR(fmt);

	Video::ResolutionValues rv = Video::GetValues(resolution);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	fmt.fmt.pix.width       = rv.width;
	fmt.fmt.pix.height      = rv.height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
	{
		errLog->critical("Camera {0} init_device() error -1 == xioctl(fd, VIDIOC_S_FMT, &fmt)", name);
		return -5;
	}

	if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV)
	{
		errLog->critical("Camera {0} init_device() error fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV", name);
		return -6;
		// note that libv4l2 (look for 'v4l-utils') provides helpers
		// to manage conversions
	}

	return init_mmap();
}

int CameraImpl::init_mmap()
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 2;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			errLog->critical("Camera {0} init_mmap() error EINVAL", name);
			return -1;
		}
		else
		{
			errLog->critical("Camera {0} init_mmap() error %d", name, errno);
			return -2;
		}
	}

	if (req.count < 2)
	{
		errLog->critical("Camera {0} init_mmap() error req.count < 2", name);
		return -3;
	}

	buffers = (Buffer*) calloc(req.count, sizeof(*buffers));
	if (!buffers)
	{
		errLog->critical("Camera {0} init_mmap() error buffers calloc", name);
		return -4;
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
	{
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;
		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
		{
			return -5;
		}

		buffers[n_buffers].size = buf.length;
		buffers[n_buffers].data =
			mmap(NULL /* start anywhere */,
			buf.length,
			PROT_READ | PROT_WRITE /* required */,
			MAP_SHARED /* recommended */,
			fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].data)
		{
			errLog->critical("Camera {0} init_mmap() error MAP_FAILED == buffers[n_buffers].data", name);
			return -6;
		}
	}

	return 0;
}

void CameraImpl::uninit_device()
{
	unsigned int i;

	for (i = 0; i < n_buffers; ++i)
	{
		if (-1 == munmap(buffers[i].data, buffers[i].size))
		{
			errLog->critical("Camera {0} uninit_device() error -1 == munmap(buffers[i].data, buffers[i].size)", name);
		}
	}

	free(buffers);
}

int CameraImpl::start_capturing()
{
	unsigned int i;
	enum v4l2_buf_type type;

	for (i = 0; i < n_buffers; ++i)
	{
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
		{
			errLog->critical("Camera {0} start_capturing() error -1 == xioctl(fd, VIDIOC_QBUF, &buf)", name);
			return -1;
		}
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
	{
		errLog->critical("Camera {0} start_capturing() error -1 == xioctl(fd, VIDIOC_STREAMON, &type)", name);
		return -2;
	}
	return 0;
}

void CameraImpl::stop_capturing()
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
	{
		errLog->critical("Camera {0} stop_capturing() error -1 == xioctl(fd, VIDIOC_STREAMOFF, &type)", name);
	}
}

}
