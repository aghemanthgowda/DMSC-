cmake_minimum_required(VERSION 3.16)
project(driver_monitoring_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Only the base OpenCV modules are required. The `face` module (68-point
# landmarks) ships with opencv-contrib and is OPTIONAL: when present we enable
# landmark mode, otherwise the program still builds and runs in Haar fallback
# mode. This lets Windows users use the official prebuilt OpenCV (no contrib,
# no compiling) and get a working demo.
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs objdetect highgui videoio calib3d)

option(DMS_USE_FACE "Enable 68-point landmark mode (needs the OpenCV contrib 'face' module)" ON)

set(DMS_FACE_AVAILABLE FALSE)
if(DMS_USE_FACE)
    if(TARGET opencv_face OR "opencv_face" IN_LIST OpenCV_LIBS
       OR "opencv_face" IN_LIST OpenCV_LIB_COMPONENTS)
        set(DMS_FACE_AVAILABLE TRUE)
    endif()
endif()

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

if(DMS_FACE_AVAILABLE)
    target_compile_definitions(dms PRIVATE DMS_HAVE_FACE)
    target_link_libraries(dms PRIVATE opencv_face)
    message(STATUS "DMS: landmark mode ENABLED (OpenCV 'face' module found)")
else()
    message(STATUS "DMS: landmark mode DISABLED - Haar fallback only "
                   "(install OpenCV contrib for 68-point landmarks)")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(dms PRIVATE -Wall -Wextra)
endif()
