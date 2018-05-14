cmake_minimum_required(VERSION 3.7)
project(gemian-leds)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

set(CMAKE_CXX_STANDARD 14)


include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GIO REQUIRED gio-2.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pthread -std=c11 -Werror -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=c++14 -Werror -Wall -Wnon-virtual-dtor -Wextra -pedantic")


add_subdirectory(src)

add_subdirectory(tests)
