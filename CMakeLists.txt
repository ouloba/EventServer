cmake_minimum_required(VERSION 2.6)

PROJECT(gameserver) 

if(WIN32)
AUX_SOURCE_DIRECTORY(. SRC_LIST1)
endif(WIN32)

if(UNIX)
set(SRC_LIST1 EventServer.cpp gameserver.cpp LuaThread.cpp LuaThreadMgr.cpp https-common.c)
endif(UNIX)

AUX_SOURCE_DIRECTORY(./lae/include SRC_LIST2)
AUX_SOURCE_DIRECTORY(./lae/source SRC_LIST3)
AUX_SOURCE_DIRECTORY(./lua/lua_and_tolua++/src/tolua++ SRC_LIST4)


include_directories(./lae/include)
include_directories(./lua/lua_and_tolua++/include)
include_directories(./lua/include)

if(WIN32)
include_directories(./lae/libevent/include)
include_directories(./lae/libevent/WIN32-Code/nmake)
link_directories(../../Lib)
endif(WIN32)


if(UNIX)
include_directories(/usr/include)
include_directories(/usr/local/include)
include_directories(/usr/local/ssl/include)
link_directories(/usr/local/ssl/lib)
link_directories(/usr/local/lib)
link_directories(/usr/lib)
link_directories(/usr/local/lib/lua/5.1/socket)
endif(UNIX)



link_directories(../../lua/lib)
ADD_DEFINITIONS(-DUSE_INTERNAL_ISINF)  
ADD_DEFINITIONS(-DEVENT__HAVE_OPENSSL)  


set(CMAKE_CXX_FLAGS "-g -Wall -fstack-protector ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "-g -Wall -fstack-protector ${CMAKE_C_FLAGS}")

ADD_EXECUTABLE(gameserver ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4})

if(WIN32)
target_link_libraries(gameserver libevent libevent_core libevent_extras ws2_32 lua51 libcrypto_static libssl_static libevent_openssl)
endif(WIN32)

if(UNIX)
target_link_libraries(gameserver event event_core event_extra event_pthreads ssl crypto event_openssl  luajit-5.1 pthread)
endif(UNIX)
