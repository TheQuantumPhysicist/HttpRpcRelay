cmake_minimum_required(VERSION 2.8)

project(HttpRpcRelay)

find_package(Threads REQUIRED)

include_directories(.)
include_directories(src)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.14/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP
                BUILD missing
                SETTINGS jsoncpp:compiler=gcc
                PROFILE_AUTO ALL # detects all cmake settings
                )

add_subdirectory(3rdparty)

add_library(http_rpc_relay_lib
    src/Server/RelayServer.cpp
    src/Server/RelaySession.cpp
    src/Client/ClientSession.cpp
    src/Filters/JsonRPCFilter.cpp
    )

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME}
    http_rpc_relay_lib
    Threads::Threads
    )

include_directories(${SPDLOG_PATH}/include)

enable_testing()
add_subdirectory(tests)

macro(ENFORCE_CLANG)
    if (CMAKE_CXX_COMPILER MATCHES ".*clang.*")
    else()
        MESSAGE(FATAL_ERROR "You must use the clang compilers to use the sanitizers")
    endif()
endmacro()

option(SANITIZE_THREAD "Enable clang thread-sanitizer (only for clang-compiler)" OFF)
if(SANITIZE_THREAD)
    ENFORCE_CLANG()
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -g -fsanitize=thread")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=thread")
endif()

option(SANITIZE_UNDEFINED "Enable clang undefined-behavior-sanitizer (only for clang-compiler)" OFF)
if(SANITIZE_UNDEFINED)
    ENFORCE_CLANG()
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined")
endif()

option(SANITIZE_ADDRESS "Enable clang address-sanitizer (only for clang-compiler)" OFF)
if(SANITIZE_ADDRESS)
    ENFORCE_CLANG()
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -g -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
endif()

option(SANITIZE_LEAK "Enable clang memory-sanitizer (only for clang-compiler)" OFF)
if(SANITIZE_LEAK)
    ENFORCE_CLANG()
    set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} -g -fsanitize=leak")
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=leak")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=leak")
endif()
