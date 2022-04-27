cmake_minimum_required(VERSION 3.2)

project(THREAD VERSION 1.0 LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)

target_include_directories(${PROJECT_NAME} INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>)

# Tests and examples
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	if (MSVC)
		add_compile_options(/permissive- /W4 /wd4172 /wd4324 /wd4530)
	else()
		add_compile_options(-Wall -Wextra -Wpedantic)
	endif()

	find_package(Threads REQUIRED)

	add_executable(test_queue src/test_queue.cpp)
	target_link_libraries(test_queue THREAD Threads::Threads)

	add_executable(test_threadpool src/test_threadpool.cpp)
	target_link_libraries(test_threadpool THREAD Threads::Threads)

	enable_testing()
	add_test(test_threadpool test_threadpool)
endif()