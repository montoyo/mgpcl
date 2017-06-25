cmake_minimum_required(VERSION 2.8.11)
project(MGPCL)

#GUI and SSL
option(MGPCL_ENABLE_GUI "Enable GUI features (requires GTK on Linux)" ON)
option(MGPCL_ENABLE_SSL "Enable SSL and crypto features (requires OpenSSL)" ON)

#Configuration
if(NOT MGPCL_ENABLE_GUI)
	set(MGPCL_NO_GUI ON)
endif()

if(NOT MGPCL_ENABLE_SSL)
	set(MGPCL_NO_SSL ON)
endif()

configure_file("include/mgpcl/Config.h.in" "include/mgpcl/Config.h")

#Other things
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(MGPCL_OUTPUT_DIR ${CMAKE_BINARY_DIR})
set(MGPCL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

if(UNIX AND MGPCL_ENABLE_GUI)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
endif()

add_subdirectory(src)
add_subdirectory(test)

