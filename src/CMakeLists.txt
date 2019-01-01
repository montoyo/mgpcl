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
set(MGPCL_LIB_HEADERS Allocator.h Assert.h Atomic.h BasicLogger.h BasicParser.h Bitfield.h BufferedOStream.h BufferIOStream.h ByteBuf.h Complex.h Cond.h Config.h ConsoleUtils.h CPUInfo.h CRC32_Poly.h DataIOStream.h DataSerializer.h Date.h Enums.h FFT.h File.h FileIOStream.h FlatMap.h GUI.h Hasher.h HashMap.h HMAC.h HTTPCookieJar.h HTTPRequest.h INet.h IOStream.h IPv4Address.h JSON.h LineReader.h List.h Logger.h Math.h Matrix3.h Matrix4.h Mem.h MsgBox.h Mutex.h NetLogger.h NiftyCounter.h Packet.h Process.h ProgramArgs.h Quaternion.h Queue.h Random.h Ray.h ReadWriteLock.h RefCounter.h SerialIO.h SHA.h Shape.h SharedObject.h SharedPtr.h SignalSlot.h Singleton.h SSE.h SSLContext.h SSLSocket.h STDIOStream.h String.h StringIOStream.h TCPClient.h TCPServer.h TCPSocket.h TextIOStream.h TextSerializer.h Thread.h Time.h URL.h Util.h VAList.h Variant.h Vector2.h Vector3.h Version.h BigNumber.h RSA.h SimpleConfig.h AES.h LineOStream.h MathConstants.h Color.h Future.h Pattern.h HTTPCommons.h HTTPServer.h LinuxSpecific.h ThreadLocal.h)
set(MGPCL_LIB_SOURCE Assert.cpp ProgramArgs.cpp Date.cpp File.cpp FileIOStream.cpp ReadWriteLock.cpp Thread.cpp Time.cpp Util.cpp Variant.cpp SharedObject.cpp INet.cpp IPv4Address.cpp TCPSocket.cpp URL.cpp HTTPCookieJar.cpp HTTPRequest.cpp StringIOStream.cpp TCPClient.cpp NetLogger.cpp Process.cpp BasicLogger.cpp Logger.cpp Version.cpp Random.cpp JSON.cpp MsgBox.cpp GUI.cpp CPUInfo.cpp TCPServer.cpp SerialIO.cpp FFT.cpp ConsoleUtils.cpp TextSerializer.cpp SSLContext.cpp SSLSocket.cpp SHA.cpp HMAC.cpp BigNumber.cpp RSA.cpp AES.cpp SimpleConfig.cpp Pattern.cpp HTTPCommons.cpp HTTPServer.cpp LinuxSpecific.cpp)
foreach(f ${MGPCL_LIB_HEADERS})
    list(APPEND MGPCL_LIB_SOURCE ../include/mgpcl/${f})
endforeach(f)

#Link and include directories
if(UNIX AND MGPCL_ENABLE_GUI)
	include_directories(${GTK3_INCLUDE_DIRS})
endif()

include_directories(${MGPCL_INCLUDE_DIR})

#Actual library
add_library(mgpcl STATIC ${MGPCL_LIB_SOURCE})
