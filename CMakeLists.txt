cmake_minimum_required(VERSION 3.24)
set(GCC_COMPILE_FLAGS "-g -fdiagnostics-color=always -Wno-narrowing -std=c++20")

set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeFiles/modules")
project(chess)

ADD_DEFINITIONS(${GCC_COMPILE_FLAGS})

set(SRC main.cpp)

add_executable(chess ${SRC})
include_directories(classes/model
                    classes/view 
                    classes/logger 
                    ../libview/include
                    .)
add_subdirectory(classes/model)
add_subdirectory(classes/view)
add_subdirectory(classes/logger)

add_library(view STATIC IMPORTED)
set_target_properties(view PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../libview/build/libview.a"
)

set(LIBS view chess_view model logger)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})

target_link_libraries(chess ${LIBS} ${SDL2_LIBRARIES} "-mconsole" ${SDL2_IMAGE_LIBRARIES} SDL2_ttf::SDL2_ttf)