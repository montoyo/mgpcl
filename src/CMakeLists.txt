cmake_minimum_required(VERSION 2.8.11)
project(mgpcl)

#Declare _DEBUG if debug build type
if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	add_definitions(-D_DEBUG)
endif()

#Enable C++0x
if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-std=gnu++0x)
endif()

#Source files
set(MGPCL_LIB_HEADERS ../include/mgpcl/Allocator.h ../include/mgpcl/Assert.h ../include/mgpcl/Atomic.h ../include/mgpcl/BasicLogger.h ../include/mgpcl/BasicParser.h ../include/mgpcl/Bitfield.h ../include/mgpcl/BufferedOStream.h ../include/mgpcl/BufferIOStream.h ../include/mgpcl/ByteBuf.h ../include/mgpcl/Complex.h ../include/mgpcl/Cond.h ../include/mgpcl/Config.h ../include/mgpcl/ConsoleUtils.h ../include/mgpcl/CPUInfo.h ../include/mgpcl/CRC32_Poly.h ../include/mgpcl/DataIOStream.h ../include/mgpcl/DataSerializer.h ../include/mgpcl/Date.h ../include/mgpcl/Enums.h ../include/mgpcl/FFT.h ../include/mgpcl/File.h ../include/mgpcl/FileIOStream.h ../include/mgpcl/FlatMap.h ../include/mgpcl/GUI.h ../include/mgpcl/Hasher.h ../include/mgpcl/HashMap.h ../include/mgpcl/HMAC.h ../include/mgpcl/HTTPCookieJar.h ../include/mgpcl/HTTPRequest.h ../include/mgpcl/INet.h ../include/mgpcl/IOStream.h ../include/mgpcl/IPv4Address.h ../include/mgpcl/JSON.h ../include/mgpcl/Keyboard.h ../include/mgpcl/LineReader.h ../include/mgpcl/List.h ../include/mgpcl/Logger.h ../include/mgpcl/Math.h ../include/mgpcl/Matrix3.h ../include/mgpcl/Matrix4.h ../include/mgpcl/Mem.h ../include/mgpcl/MsgBox.h ../include/mgpcl/Mutex.h ../include/mgpcl/NetLogger.h ../include/mgpcl/NiftyCounter.h ../include/mgpcl/Packet.h ../include/mgpcl/Process.h ../include/mgpcl/ProgramArgs.h ../include/mgpcl/Quaternion.h ../include/mgpcl/Queue.h ../include/mgpcl/Random.h ../include/mgpcl/Ray.h ../include/mgpcl/ReadWriteLock.h ../include/mgpcl/RefCounter.h ../include/mgpcl/SerialIO.h ../include/mgpcl/SHA.h ../include/mgpcl/Shape.h ../include/mgpcl/SharedObject.h ../include/mgpcl/SharedPtr.h ../include/mgpcl/SignalSlot.h ../include/mgpcl/Singleton.h ../include/mgpcl/SSE.h ../include/mgpcl/SSLContext.h ../include/mgpcl/SSLSocket.h ../include/mgpcl/STDIOStream.h ../include/mgpcl/String.h ../include/mgpcl/StringIOStream.h ../include/mgpcl/TCPClient.h ../include/mgpcl/TCPServer.h ../include/mgpcl/TCPSocket.h ../include/mgpcl/TextIOStream.h ../include/mgpcl/TextSerializer.h ../include/mgpcl/Thread.h ../include/mgpcl/Time.h ../include/mgpcl/URL.h ../include/mgpcl/Util.h ../include/mgpcl/VAList.h ../include/mgpcl/Variant.h ../include/mgpcl/Vector2.h ../include/mgpcl/Vector3.h ../include/mgpcl/Version.h ../include/mgpcl/Window.h ../include/mgpcl/BigNumber.h ../include/mgpcl/RSA.h ../include/mgpcl/SimpleConfig.h ../include/mgpcl/AES.h ../include/mgpcl/LineOStream.h ../include/mgpcl/MathConstants.h)
set(MGPCL_LIB_SOURCE ${MGPCL_LIB_HEADERS} Assert.cpp ProgramArgs.cpp Date.cpp File.cpp FileIOStream.cpp ReadWriteLock.cpp Thread.cpp Time.cpp Util.cpp Variant.cpp SharedObject.cpp INet.cpp IPv4Address.cpp TCPSocket.cpp URL.cpp HTTPCookieJar.cpp HTTPRequest.cpp StringIOStream.cpp TCPClient.cpp NetLogger.cpp Process.cpp BasicLogger.cpp Logger.cpp Version.cpp Random.cpp JSON.cpp MsgBox.cpp GUI.cpp CPUInfo.cpp TCPServer.cpp Window.cpp Keyboard.cpp SerialIO.cpp FFT.cpp ConsoleUtils.cpp TextSerializer.cpp SSLContext.cpp SSLSocket.cpp SHA.cpp HMAC.cpp BigNumber.cpp RSA.cpp AES.cpp SimpleConfig.cpp)

#Link and include directories
if(UNIX AND MGPCL_ENABLE_GUI)
	include_directories(${GTK3_INCLUDE_DIRS})
endif()

include_directories(${MGPCL_INCLUDE_DIR})

#Actual library
add_library(mgpcl STATIC ${MGPCL_LIB_SOURCE})
