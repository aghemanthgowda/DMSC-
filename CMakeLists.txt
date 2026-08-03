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

option(DMS_USE_FACE "Enable OpenCV-contrib 68-point landmark mode ('face' module)" ON)
option(DMS_USE_DLIB "Enable dlib 68-point landmark mode (recommended, most accurate)" ON)

set(DMS_FACE_AVAILABLE FALSE)
if(DMS_USE_FACE)
    if(TARGET opencv_face OR "opencv_face" IN_LIST OpenCV_LIBS
       OR "opencv_face" IN_LIST OpenCV_LIB_COMPONENTS)
        set(DMS_FACE_AVAILABLE TRUE)
    endif()
endif()

# dlib is optional and found three ways, in order of convenience:
#   1) an installed/vcpkg dlib via its CMake config (find_package), OR
#   2) a local dlib SOURCE checkout compiled as part of this build - the most
#      reliable path on Windows (no vcpkg, no toolchain file):
#         git clone https://github.com/davisking/dlib.git
#      then just build; CMake compiles dlib automatically.
# If none is present the build still works in Haar mode.
set(DMS_DLIB_AVAILABLE FALSE)
set(DMS_DLIB_TARGET "")
if(DMS_USE_DLIB)
    find_package(dlib CONFIG QUIET)
    if(dlib_FOUND)
        set(DMS_DLIB_AVAILABLE TRUE)
        set(DMS_DLIB_TARGET dlib::dlib)
        message(STATUS "DMS: dlib found via find_package")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/dlib/dlib/CMakeLists.txt")
        add_subdirectory("${CMAKE_SOURCE_DIR}/dlib/dlib" dlib_build)
        set(DMS_DLIB_AVAILABLE TRUE)
        set(DMS_DLIB_TARGET dlib::dlib)
        message(STATUS "DMS: dlib compiled from source at ./dlib")
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/dlib/CMakeLists.txt")
        add_subdirectory("${CMAKE_SOURCE_DIR}/dlib" dlib_build)
        set(DMS_DLIB_AVAILABLE TRUE)
        set(DMS_DLIB_TARGET dlib::dlib)
        message(STATUS "DMS: dlib compiled from source at ./dlib")
    endif()
endif()

add_executable(dms
    src/main.cpp
    src/Types.cpp
    src/ConfigManager.cpp
    src/FaceTracker.cpp
    src/GazeEstimator.cpp
    src/MonitoringQuality.cpp
    src/DrowsinessDetector.cpp
    src/DistractionDetector.cpp
    src/ObjectDetector.cpp
    src/RiskEngine.cpp
    src/AlertManager.cpp
    src/EventLogger.cpp
    src/Dashboard.cpp
)

target_include_directories(dms PRIVATE ${CMAKE_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(dms PRIVATE ${OpenCV_LIBS})

if(DMS_DLIB_AVAILABLE)
    target_compile_definitions(dms PRIVATE DMS_HAVE_DLIB)
    target_link_libraries(dms PRIVATE ${DMS_DLIB_TARGET})
    message(STATUS "DMS: dlib 68-point landmark mode ENABLED")
else()
    message(STATUS "DMS: dlib landmark mode DISABLED (dlib not found)")
endif()

if(DMS_FACE_AVAILABLE)
    target_compile_definitions(dms PRIVATE DMS_HAVE_FACE)
    target_link_libraries(dms PRIVATE opencv_face)
    message(STATUS "DMS: OpenCV 'face' landmark mode ENABLED")
endif()

if(NOT DMS_DLIB_AVAILABLE AND NOT DMS_FACE_AVAILABLE)
    message(STATUS "DMS: no landmark backend - Haar fallback only "
                   "(install dlib for 68-point landmarks)")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(dms PRIVATE -Wall -Wextra)
endif()
