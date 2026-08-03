cmake_minimum_required(VERSION 3.16)
project(driver_monitoring_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# OpenCV core modules plus the `face` module (facemark landmarks). The `face`
# module ships with opencv-contrib; if it is missing the build still succeeds
# and the program runs in Haar fallback mode.
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs objdetect highgui videoio calib3d face)

add_executable(dms
    src/main.cpp
    src/FaceTracker.cpp
    src/DrowsinessDetector.cpp
    src/DistractionDetector.cpp
    src/AlertManager.cpp
    src/Dashboard.cpp
)

target_include_directories(dms PRIVATE ${CMAKE_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(dms PRIVATE ${OpenCV_LIBS})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(dms PRIVATE -Wall -Wextra)
endif()
