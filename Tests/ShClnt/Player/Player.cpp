/**
 * Player.cpp - Contains impl of shell client's media player
 *
 * Author: Anton (ud) Golovkov, udattsk@gmail.com
 * Copyright (C), Infinity Video Soft LLC, 2020
 */

#include <Player/Player.h>

#include <Transport/RTP/RTPPacket.h>
#include <Transport/RTP/RTCPPacket.h>
#include <Transport/RTP/RTPPayloadType.h>

#include <Common/CRC32.h>

#include <Common/ShortSleep.h>

#include <ippcc.h>
#include <ippi.h>

#include <fstream>

namespace Player
{

Player::Player()
	: runned(false),
	cameraStarted(false), microphoneStarted(false),

	fileName(),

    timeMeter(),

	reader(),

	cameraSocket(), microphoneSocket(),

	videoEncoder(),
	videoSplitter(),

    cameraEncryptor(), microphoneEncryptor(),

	playMode(PlayMode::undefined),

    cameraId(0), microphoneId(0),

	cameraSSRC(0), cameraFrameIndex(0),
	microphoneSSRC(0), microphoneFrameIndex(0),

    videoSendTime(0), audioSendTime(0),

	outputBuffer(), tmpBuffer(),

	sysLog(spdlog::get("System")), errLog(spdlog::get("Error"))
{
	videoSplitter.SetReceiver(&cameraSocket);

    cameraEncryptor.SetReceiver(&cameraSocket);
    microphoneEncryptor.SetReceiver(&microphoneSocket);

	videoEncoder.SetReceiver(&videoSplitter);
	cameraSocket.SetReceiver(nullptr, this);
}

Player::~Player()
{
}

void Player::SetFileName(std::string_view fileName_)
{
	playMode = PlayMode::undefined;

	fileName = fileName_;

	if (fileName.find(".mkv") != std::string::npos)
	{
		playMode = PlayMode::mkv_av;
		sysLog->info("Player file is: {0}, mode mkv_av", fileName);
	}
	else if (fileName.find(".bmp") != std::string::npos)
	{
		playMode = PlayMode::rgb24_show;
		sysLog->info("Player file is: {0}, mode rgb24_show", fileName);
	}
	else if (fileName.find(".mp3") != std::string::npos)
	{
		playMode = PlayMode::mp3_play;
		sysLog->info("Player file is: {0}, mode mp3", fileName);
	}
	else
	{
		
		errLog->critical("[Player] Filename {0} is invalid or error while opening", fileName);
	}
}

Video::Resolution Player::GetResolution()
{
	switch (playMode)
	{
		case PlayMode::mkv_av:     return GetResolutionMKV();   break;
		case PlayMode::rgb24_show: return GetResolutionRGB24(); break;
		case PlayMode::mp3_play:   return GetResolutionMP3();   break;
	}

	return Video::GetResolution(Video::ResolutionValues(0, 0));
}

Video::Resolution Player::GetResolutionMKV()
{
	using namespace mkvparser;

	MkvReader reader;

	if (reader.Open(fileName.c_str()))
	{
		errLog->critical("[Player] Filename {0} is invalid or error while opening", fileName);
		return Video::rUndefined;
	}

	long long pos = 0;

	EBMLHeader ebmlHeader;

	ebmlHeader.Parse(&reader, pos);

	mkvparser::Segment* pSegment;

	long long ret = mkvparser::Segment::CreateInstance(&reader, pos, pSegment);
	if (ret)
	{
		errLog->critical("Segment::CreateInstance() failed");
		return Video::rUndefined;
	}

	ret = pSegment->Load();
	if (ret < 0)
	{
		errLog->critical("Segment::Load() failed");
		return Video::rUndefined;
	}

	const SegmentInfo* const pSegmentInfo = pSegment->GetInfo();

	const mkvparser::Tracks* pTracks = pSegment->GetTracks();

	unsigned long i = 0;
	const unsigned long j = pTracks->GetTracksCount();

	enum { VIDEO_TRACK = 1, AUDIO_TRACK = 2 };

	while (i < j)
	{
		const Track* const pTrack = pTracks->GetTrackByIndex(i++);

		if (pTrack == NULL)
			continue;

		const long long trackType = pTrack->GetType();
		const long long trackNumber = pTrack->GetNumber();
		const unsigned long long trackUid = pTrack->GetUid();

		if (trackType == VIDEO_TRACK)
		{
			const VideoTrack* const pVideoTrack = static_cast<const VideoTrack*>(pTrack);

			return Video::GetResolution(Video::ResolutionValues((uint16_t)pVideoTrack->GetWidth(), (uint16_t)pVideoTrack->GetHeight()));
		}
	}

	return Video::rUndefined;
}

Video::Resolution Player::GetResolutionRGB24()
{
	return Video::GetResolution(Video::ResolutionValues(1280, 720)); 
}

Video::Resolution Player::GetResolutionMP3()
{
	return Video::GetResolution(Video::ResolutionValues(0, 0));
}

void Player::SetCameraRTPParams(const char* addr, uint16_t rtpPort)
{
	cameraSocket.SetDefaultAddress(addr, rtpPort);
}

void Player::SetMicrophoneRTPParams(const char* addr, uint16_t rtpPort)
{
	microphoneSocket.SetDefaultAddress(addr, rtpPort);
}

void Player::StartCamera(uint32_t ssrc, std::string_view secureKey)
{
	if (!cameraStarted)
	{
		cameraStarted = true;

        cameraSSRC = ssrc;

		if (playMode == PlayMode::rgb24_show)
		{
			videoEncoder.Start(Video::CodecType::VP8);
		}

        if (!secureKey.empty())
        {
            videoSplitter.SetReceiver(&cameraEncryptor);
            cameraEncryptor.Start(secureKey);
        }
        else
        {
            videoSplitter.SetReceiver(&cameraSocket);
        }

		cameraSocket.Start();
	}
}

void Player::StopCamera()
{
	cameraStarted = false;
	
	cameraSocket.Stop();

    cameraEncryptor.Stop();

	videoEncoder.Stop();
}

void Player::StartMicrophone(uint32_t ssrc, std::string_view secureKey)
{
	if (!microphoneStarted)
	{
		microphoneStarted = true;

        microphoneSSRC = ssrc;
        if (!secureKey.empty())
        {
            microphoneEncryptor.Start(secureKey);
        }

		microphoneSocket.Start();
	}
}

void Player::StopMicrophone()
{
	microphoneStarted = false;

	microphoneSocket.Stop();

    microphoneEncryptor.Stop();
}

void Player::Start()
{
	if (runned || playMode == PlayMode::undefined)
	{
		return;
	}

    timeMeter.Reset();

	runned = true;

	if (playMode == PlayMode::rgb24_show)
	{
		Video::ResolutionValues rv = Video::GetValues(GetResolution());

		size_t bufferSize = rv.width * rv.height * 3;
		try
		{
			outputBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
			tmpBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
		}
		catch (std::bad_alloc &)
		{
			return errLog->critical("Player::Start no memory");
		}
	}

    worker = std::thread([this](){ 
		while(runned)
		{
			switch (playMode)
			{
				case PlayMode::mkv_av:     PlayMKV();   break;
				case PlayMode::rgb24_show: ShowRGB24(); break;
				case PlayMode::mp3_play:   PlayMP3();   break;
			}
		}
	});
}

void Player::Stop()
{
	runned = false;

	if (worker.joinable())
	{
        worker.join();
	}

	StopCamera();
	StopMicrophone();

	outputBuffer.reset(nullptr);
	tmpBuffer.reset(nullptr);
}

bool Player::IsCameraStarted()
{
	return cameraStarted;
}

bool Player::IsMicrophoneStarted()
{
	return microphoneStarted;
}

void Player::PlayMKV()
{
	using namespace mkvparser;

	MkvReader reader;

	if (reader.Open(fileName.c_str()))
	{
		errLog->critical("[Player] Filename {0} is invalid or error while opening", fileName);
		return;
	}

	long long pos = 0;

	EBMLHeader ebmlHeader;

	ebmlHeader.Parse(&reader, pos);

	mkvparser::Segment* pSegment;

	long long ret = mkvparser::Segment::CreateInstance(&reader, pos, pSegment);
	if (ret)
	{
		return errLog->critical("Segment::CreateInstance() failed");
	}

	ret = pSegment->Load();
	if (ret < 0)
	{
		return errLog->critical("Segment::Load() failed");
	}

	const SegmentInfo* const pSegmentInfo = pSegment->GetInfo();

	const mkvparser::Tracks* pTracks = pSegment->GetTracks();

	const unsigned long clusterCount = pSegment->GetCount();
	if (clusterCount == 0)
	{
		errLog->critical("Segment has no clusters");
		delete pSegment;
		return;
	}

	const mkvparser::Cluster* pCluster = pSegment->GetFirst();

	enum { VIDEO_TRACK = 1, AUDIO_TRACK = 2 };

	while (runned && pCluster != NULL && !pCluster->EOS())
	{
		const BlockEntry* pBlockEntry;

		long status = pCluster->GetFirst(pBlockEntry);

		if (status < 0)  //error
		{
			errLog->critical("Error parsing first block of cluster");
			goto done;
		}

		while (runned && pBlockEntry != NULL && !pBlockEntry->EOS())
		{
			const Block* const pBlock = pBlockEntry->GetBlock();
			const long long trackNum = pBlock->GetTrackNumber();
			const unsigned long tn = static_cast<unsigned long>(trackNum);
			const Track* const pTrack = pTracks->GetTrackByNumber(tn);
			const long long trackType = pTrack->GetType();
			const int frameCount = pBlock->GetFrameCount();

			for (int i = 0; i < frameCount; ++i)
			{
				const Block::Frame& theFrame = pBlock->GetFrame(i);
				const long size = theFrame.len;
				const long long offset = theFrame.pos;

                Transport::OwnedRTPPacket packet(Transport::RTPPacket::RTPHeader(),
                    nullptr,
                    size,
                    trackType == VIDEO_TRACK ? Transport::RTPPayloadType::ptVP8 : Transport::RTPPayloadType::ptOpus);

				theFrame.Read(&reader, packet.data);

				if (trackType == VIDEO_TRACK && cameraStarted)
				{
					packet.header.pt = static_cast<uint8_t>(Transport::RTPPayloadType::ptVP8);
					packet.header.seq = ++cameraFrameIndex;
					packet.header.ssrc = cameraSSRC;
                    
                    SendVideo(packet);
					std::this_thread::sleep_for(std::chrono::milliseconds(40));
				}
				else if (trackType == AUDIO_TRACK && microphoneStarted)
				{
					packet.header.pt = static_cast<uint8_t>(Transport::RTPPayloadType::ptOpus);
					packet.header.seq = ++microphoneFrameIndex;
					packet.header.ssrc = microphoneSSRC;
                    packet.header.x = 1;
					packet.header.eXLength = 1;
					packet.header.eX[0] = Common::crc32(0, packet.data, size);

                    SendAudio(packet);
					std::this_thread::sleep_for(std::chrono::milliseconds(40));
				}
			}

			status = pCluster->GetNext(pBlockEntry, pBlockEntry);

			if (status < 0)
			{
				errLog->critical("Error parsing next block of cluster");
				goto done;
			}
		}

		pCluster = pSegment->GetNext(pCluster);
	}

done:
	delete pSegment;
}

void Player::ConvertFromRGB24(unsigned char* data_, size_t *len_)
{
	const Video::ResolutionValues rv = Video::GetValues(GetResolution());

	const IppiSize  sz = { rv.width, rv.height };
	Ipp8u*          dst[3] = { outputBuffer.get(), outputBuffer.get() + (rv.width * rv.height), outputBuffer.get() + (rv.width * rv.height) + ((rv.width * rv.height)/4) };
	int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };
	
	ippiMirror_8u_C3R(data_, rv.width * 3, tmpBuffer.get(), rv.width * 3, sz, ippAxsHorizontal);
	ippiBGRToYCbCr420_8u_C3P3R(tmpBuffer.get(), rv.width * 3, dst, dstStep, sz);
		
	*len_ = (rv.width * rv.height) + ((rv.width * rv.height)/2);
}

void Player::ConvertFromRGB32(unsigned char* data_, size_t *len_)
{
	const Video::ResolutionValues rv = Video::GetValues(GetResolution());

	const IppiSize  sz = { rv.width, rv.height };
	Ipp8u*          dst[3] = { outputBuffer.get(), outputBuffer.get() + (rv.width * rv.height), outputBuffer.get() + (rv.width * rv.height) + ((rv.width * rv.height) / 4) };
	int             dstStep[3] = { rv.width, rv.width / 2, rv.width / 2 };

	ippiBGRToYCbCr420_8u_AC4P3R(data_, rv.width * 4, dst, dstStep, sz);

	*len_ = (rv.width * rv.height) + ((rv.width * rv.height) / 2);
}

void Player::ShowRGB24()
{
	std::ifstream f(fileName, std::ios::binary | std::ios::ate);
	if (!f)
	{
		return errLog->critical("[Player] Filename {0} is invalid or error while opening", fileName);
	}

	auto size = f.tellg();
	f.seekg(0, std::ios::beg);
	
	std::vector<uint8_t> buffer(size);
	if (f.read((char *)buffer.data(), size))
	{
		size_t sz = size;
    	ConvertFromRGB24(buffer.data(), &sz);

		auto sendTime = timeMeter.Measure();
		
		Transport::RTPPacket outPacket;
		outPacket.rtpHeader.ts = (uint32_t)(sendTime / 1000);
		outPacket.rtpHeader.pt = static_cast<uint8_t>(Transport::RTPPayloadType::ptVP8);
		
		outPacket.rtpHeader.seq = ++cameraFrameIndex;
		outPacket.rtpHeader.ssrc = cameraSSRC;
		
		outPacket.payload = outputBuffer.get();
		outPacket.payloadSize = sz;
		
		videoEncoder.Send(outPacket);
		
		//sysLog->trace("Sended packet, size: {0}", sz);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
}

void Player::PlayMP3()
{
	std::ifstream f(fileName);
	if (!f)
	{
		return errLog->critical("[Player] Filename {0} is invalid or error while opening", fileName);
	}
}

void Player::SendAudio(const Transport::OwnedRTPPacket &packet_)
{
    audioSendTime = timeMeter.Measure();

    Transport::RTPPacket outPacket;
    outPacket.rtpHeader = packet_.header;
    outPacket.rtpHeader.ts = packet_.header.ts;
    outPacket.payload = packet_.data;
    outPacket.payloadSize = packet_.size;
            
    if (microphoneEncryptor.Started())
    {
        microphoneEncryptor.Send(outPacket);
    }
    else
    {
        microphoneSocket.Send(outPacket);
    }
}

void Player::SendVideo(const Transport::OwnedRTPPacket &packet_)
{
    videoSendTime = timeMeter.Measure();

    Transport::RTPPacket outPacket;

    outPacket.rtpHeader = packet_.header;
    outPacket.rtpHeader.ts = packet_.header.ts;
    outPacket.payload = packet_.data;
	outPacket.payloadSize = packet_.size;

    videoSplitter.Send(outPacket);
}

void Player::Send(const Transport::IPacket &packet_, const Transport::Address *)
{
	/// Receive RTCP to force key frame and ARC

	const Transport::RTCPPacket &packet = *static_cast<const Transport::RTCPPacket*>(&packet_);
	if (packet.rtcp_common.pt == Transport::RTCPPacket::RTCP_APP)
	{
		switch (packet.rtcps[0].r.app.messageType)
		{
			case Transport::RTCPPacket::amtForceKeyFrame:
				videoEncoder.ForceKeyFrame(ntohl(*reinterpret_cast<const uint32_t*>(packet.rtcps[0].r.app.payload)));
			break;
			case Transport::RTCPPacket::amtReduceComplexity:
				//ReduceComplexity();
			break;
			case Transport::RTCPPacket::amtSetFrameRate:
				//SetFrameRate(ntohl(*reinterpret_cast<const uint32_t*>(packet.rtcps[0].r.app.payload)));
			break;
			case Transport::RTCPPacket::amtRemoteControl:
			break;
			default:
			break;
		}
	}
}

}
