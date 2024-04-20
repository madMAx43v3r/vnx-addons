cmake_minimum_required(VERSION 3.10)
project(vnx-addons)

if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_ENABLE_EXPORTS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-sign-compare")
endif()

add_compile_definitions(NOGDI NOMINMAX)

find_package(ZLIB REQUIRED)
if(WIN32)
	find_package(zstd CONFIG REQUIRED)
	add_library(zstd ALIAS zstd::libzstd_shared)
endif()

file(GLOB GENERATED_SOURCE_FILES generated/src/*.cpp)
file(GLOB URL_CPP_SOURCE_FILES lib/url-cpp/src/*.cpp)
file(GLOB LLHTTP_SOURCE_FILES lib/llhttp/src/native/*.c)

add_library(url_cpp STATIC
	${URL_CPP_SOURCE_FILES}
)

add_library(llhttp STATIC
	${LLHTTP_SOURCE_FILES}
	lib/src/llhttp.c
)

target_include_directories(url_cpp PUBLIC lib/url-cpp/include)

target_include_directories(llhttp PUBLIC include lib/llhttp/src/native)

if(WIN32)
	list(APPEND PLATFORM_SOURCES
		src/win32/HttpServer.cpp
	)
else()
	list(APPEND PLATFORM_SOURCES
		src/linux/HttpServer.cpp
	)
endif()

add_library(vnx_addons SHARED
	${GENERATED_SOURCE_FILES}
	${PLATFORM_SOURCES}
	src/CompressedValue.cpp
	src/DeflatedValue.cpp
	src/DeflateInputStream.cpp
	src/DeflateOutputStream.cpp
	src/HttpRequest.cpp
	src/HttpResponse.cpp
	src/HttpServer.cpp
	src/TcpServer.cpp
	src/FileServer.cpp
	src/MsgServer.cpp
)

target_include_directories(vnx_addons
	PUBLIC
		include
		generated/include
)

target_link_libraries(vnx_addons vnx_base url_cpp llhttp zstd ZLIB::ZLIB)

if(MSVC)
	set_target_properties(vnx_addons PROPERTIES LINK_OPTIONS "/NODEFAULTLIB:LIBCMT")
endif()

if(VNX_ADDONS_BUILD_TESTS)
	add_executable(test_zstd test/test_zstd.cpp)
	target_link_libraries(test_zstd vnx_addons)
	
	add_executable(test_deflate test/test_deflate.cpp)
	target_link_libraries(test_deflate vnx_addons)
	
	add_executable(test_http_server test/test_http_server.cpp)
	target_link_libraries(test_http_server vnx_addons)
endif()

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY generated/include/ DESTINATION include)
install(DIRECTORY interface/ DESTINATION interface/vnx/addons)

install(TARGETS url_cpp DESTINATION lib)
install(TARGETS llhttp DESTINATION lib)
install(TARGETS vnx_addons DESTINATION lib)
