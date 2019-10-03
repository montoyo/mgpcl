cmake_minimum_required(VERSION 2.8.11)
project(mgpcl-test)

#Declare _DEBUG if debug build type
if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
	add_definitions(-D_DEBUG)
endif()

#Enable C++0x
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++0x)
endif()

#Source files
set(MGPCL_TEST_SOURCE List.cpp String.cpp IO.cpp HashMap.cpp ProgramArgs.cpp FS.cpp Threading.cpp Net.cpp Logging.cpp Processes.cpp Random.cpp JSON.cpp Main.cpp TestAPI.cpp StackIntegrityChecker.cpp TestObject.cpp GUI.cpp Misc.cpp HTTPServerTest.cpp)

#Link and include directories
if(UNIX AND MGPCL_ENABLE_GUI)
	include_directories(${GTK3_INCLUDE_DIRS})
endif()

link_directories(${MGPCL_OUTPUT_DIR})
include_directories(${MGPCL_INCLUDE_DIR})

#Executable
add_executable(mgpcl-test ${MGPCL_TEST_SOURCE})

#Libraries
target_link_libraries(mgpcl-test mgpcl)

