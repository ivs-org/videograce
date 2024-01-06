file(GLOB SOURCES 
	Audio/*.cpp
	AEC/*.cpp
	API/*.cpp
	Common/*.cpp
	Config/*.cpp
	Controller/*.cpp
	Crypto/*.cpp
	Storage/*.cpp
	JitterBuffer/*.cpp
	JitterBuffer/thirdparty/*.cpp
	License/*.cpp
	Localization/*.cpp
	Localization/impl/*.cpp
	NetTester/*.cpp
	Proto/*.cpp
	Record/*.cpp
	Transport/*.cpp
	Transport/URI/*.cpp
	Transport/HTTP/*.cpp
	Transport/RTP/*.cpp
	Transport/TCP/*.cpp
	Transport/WS/*.cpp
	Transport/WSM/*.cpp
	Video/*.cpp
	webrtc/base/*.cc
	webrtc/common_audio/*.c
	webrtc/common_audio/*.cc
	webrtc/common_audio/signal_processing/*.c
	webrtc/common_audio/signal_processing/*.cc
	webrtc/modules/audio_processing/*.cc
	webrtc/modules/audio_processing/aec/*.cc
	webrtc/modules/audio_processing/agc/legacy/*.c
	webrtc/modules/audio_processing/logging/*.cc
	webrtc/modules/audio_processing/ns/*.c
	webrtc/modules/audio_processing/utility/*.cc
	webrtc/system_wrappers/source/*.cc)

add_definitions(-DWEBRTC_LINUX -DWEBRTC_POSIX -DSQLITE_THREADSAFE=2)

include_directories(.)

add_library(Engine STATIC ${SOURCES})

