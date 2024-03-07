cmake_minimum_required (VERSION 2.8.11)

project (VideoGrace CXX C)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -static-libstdc++ -static-libgcc -Wno-unused-command-line-argument")

set(CMAKE_MAKE_PROGRAM "make -j 3")

#set(CMAKE_CXX_FLAGS:STRING= -fsanitize=address  -fsanitize=leak -g)
#set(CMAKE_C_FLAGS:STRING=-fsanitize=address  -fsanitize=leak -g)
#set(CMAKE_EXE_LINKER_FLAGS:STRING=-fsanitize=address  -fsanitize=leak)
#set(CMAKE_MODULE_LINKER_FLAGS:STRING=-fsanitize=address  -fsanitize=leak)

include_directories(
	Lib/boost_1_83_0/include
	Lib/libOpus
	Lib/OpenSSL/include
	Lib/libWebM
	Lib/libVPX
	Lib/ipp/include
	Lib/lame/include
	Lib/wui/include
	Lib/db/include
	Engine)

link_directories(Lib/boost_1_83_0/lib/lin_x64
	Lib/libOpus/libs/x64
	Lib/OpenSSL/lib/lin_x64
	Lib/libWebM/lib/lin/x64
	Lib/ipp/lib/intel64_lin
	Lib/libVPX/linux/x64
	Lib/lame/lib/lin/x64
	Lib/db/lin/x64
	Lib/wui/lin/x64)

add_subdirectory (Engine)
add_subdirectory (Client)
add_subdirectory (Tests/ShClnt)
