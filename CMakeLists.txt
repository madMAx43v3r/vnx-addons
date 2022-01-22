cmake_minimum_required(VERSION 3.11)
project(vnx-addons)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -pthread -Wall -Wno-unused-variable -Wno-sign-compare")
endif()

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

if(MSVC)
	target_include_directories(vnx_addons PUBLIC ${VNXBASE_BINARY_DIR})

	#include(GenerateExportHeader)

	#GENERATE_EXPORT_HEADER(url_cpp)
	#include_directories("${CMAKE_CURRENT_BINARY_DIR}")	

	set_target_properties(url_cpp PROPERTIES ENABLE_EXPORTS 1)
	set_target_properties(url_cpp PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)	

	#GENERATE_EXPORT_HEADER(llhttp)
	#include_directories("${CMAKE_CURRENT_BINARY_DIR}")	

	set_target_properties(llhttp PROPERTIES ENABLE_EXPORTS 1)
	set_target_properties(llhttp PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)	

	#GENERATE_EXPORT_HEADER(vnx_addons)
	#include_directories("${CMAKE_CURRENT_BINARY_DIR}")	

	set_target_properties(vnx_addons PROPERTIES ENABLE_EXPORTS 1)
	set_target_properties(vnx_addons PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)


	find_library(ZLIB_LIBRARY zlib HINTS ${ZLIB_LIBRARIES})
	add_library(z INTERFACE IMPORTED)
	set_target_properties(z PROPERTIES INTERFACE_LINK_LIBRARIES "${ZLIB_LIBRARY}")
	
	set_target_properties(vnx_addons PROPERTIES LINK_OPTIONS "/NODEFAULTLIB:LIBCMT")
	target_include_directories(vnx_addons PRIVATE ${ZLIB_INCLUDE_DIR})

endif()

target_link_libraries(vnx_addons vnx_base url_cpp llhttp z)

if(VNX_ADDONS_BUILD_TESTS)

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
