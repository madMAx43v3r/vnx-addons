cmake_minimum_required(VERSION 2.6)
project(vnx-addons)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -pthread -Wall -Wno-unused-variable -Wno-sign-compare")

file(GLOB GENERATED_SOURCE_FILES generated/src/*.cpp)

add_library(vnx_addons SHARED
	${GENERATED_SOURCE_FILES}
	src/CompressedValue.cpp
	src/DeflatedValue.cpp
	src/DeflateInputStream.cpp
	src/DeflateOutputStream.cpp
	src/HttpRequest.cpp
	src/HttpResponse.cpp
	src/HttpServer.cpp
)

target_include_directories(vnx_addons PUBLIC include generated/include)
target_link_libraries(vnx_addons vnx_base microhttpd z)

add_executable(test_deflate test/test_deflate.cpp)
target_link_libraries(test_deflate vnx_addons)

add_executable(test_http_server test/test_http_server.cpp)
target_link_libraries(test_http_server vnx_addons)

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY generated/include/ DESTINATION include)
install(DIRECTORY interface/ DESTINATION interface/vnx/keyvalue)

install(TARGETS vnx_addons DESTINATION lib)
