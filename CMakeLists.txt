
cmake_minimum_required(VERSION 3.24)

project(aqua-runtime VERSION 0.1.0 LANGUAGES CXX)

add_library(aqua-runtime
	src/application.cpp
	src/event_queue.cpp
	src/runtime.cpp
)

target_include_directories(aqua-runtime
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(aqua-runtime PUBLIC cxx_std_23)

if (MSVC)
	target_compile_options(aqua-runtime PRIVATE /W4)
else()
	target_compile_options(aqua-runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(AQUA_RUNTIME_BUILD_TESTS "Build Aqua Runtime tests" OFF)
option(AQUA_RUNTIME_BUILD_EXAMPLES "Build Aqua Runtime examples" OFF)

if (AQUA_RUNTIME_BUILD_TESTS)
	enable_testing()
	add_executable(aqua-runtime-tests tests/runtime_tests.cpp)
	target_link_libraries(aqua-runtime-tests PRIVATE aqua-runtime)
	add_test(NAME aqua-runtime-tests COMMAND aqua-runtime-tests)
endif()

if (AQUA_RUNTIME_BUILD_EXAMPLES)
	add_subdirectory(examples/minimal_app)
endif()

