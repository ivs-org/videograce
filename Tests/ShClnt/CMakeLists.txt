file(GLOB SOURCES 
	Player/*.cpp
	Processor/*.cpp
	*.cpp)

include_directories(.)

add_executable(ShClnt ${SOURCES})

target_link_libraries(ShClnt
	Engine
	wui
	db
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
)
