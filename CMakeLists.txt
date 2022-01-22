cmake_minimum_required(VERSION 2.6)
project(vnx-addons)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -pthread -Wall -Wno-unused-variable -Wno-sign-compare")
add_compile_options(-DNOGDI)

file(GLOB GENERATED_SOURCE_FILES generated/src/*.cpp)
file(GLOB URL_CPP_SOURCE_FILES lib/url-cpp/src/*.cpp)
file(GLOB LLHTTP_SOURCE_FILES lib/llhttp/src/native/*.c)

add_library(url_cpp SHARED
	${URL_CPP_SOURCE_FILES}
)

add_library(llhttp SHARED
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
target_link_libraries(vnx_addons vnx_base url_cpp llhttp z)

add_executable(test_deflate test/test_deflate.cpp)
target_link_libraries(test_deflate vnx_addons)

add_executable(test_http_server test/test_http_server.cpp)
target_link_libraries(test_http_server vnx_addons)

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY generated/include/ DESTINATION include)
install(DIRECTORY interface/ DESTINATION interface/vnx/addons)

install(TARGETS url_cpp DESTINATION lib)
install(TARGETS llhttp DESTINATION lib)
install(TARGETS vnx_addons DESTINATION lib)
