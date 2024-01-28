file(GLOB SOURCES 
	AudioRenderer/*.cpp
	AudioRenderer/lin/*.cpp
	Camera/*.cpp
	Camera/lin/*.cpp
	CaptureSession/*.cpp
	Device/*.cpp
	Microphone/*.cpp
	Microphone/lin/*.cpp
	RendererSession/*.cpp
	UI/*.cpp
	VideoRenderer/*.cpp
	Registrator/*.cpp
	*.cpp)

add_definitions(-DWEBRTC_LINUX -DWEBRTC_POSIX -DSQLITE_THREADSAFE=2)

include_directories(.)

add_executable(Client ${SOURCES})

target_link_libraries(Client
	Engine
	wui
	db
	xcb
	xcb-cursor
	xcb-ewmh
	xcb-icccm
	xcb-image
	X11
	X11-xcb
	cairo
	rt
	boost_system
	ssl
	crypto
	pthread
	dl
	opus
	webm
	vpx
	ippi
	ipps
	ippcc
	ippdc
	ippcore
	pulse-simple
	pulse
	asound
	mp3lame
)
