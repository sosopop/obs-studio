cmake_minimum_required(VERSION 3.5)

project(livetest LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

# find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
# find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

find_package(Qt5Widgets REQUIRED)

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/libobs")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
	window-basic-preview.hpp
	window-basic-preview.cpp
	qt-display.hpp
	qt-display.cpp
	qt-wrappers.hpp
	qt-wrappers.cpp
	display-helpers.hpp
	platform.hpp
	platform-windows.cpp
	window-basic-main-outputs.hpp
	window-basic-main-outputs.cpp
	audio-encoders.hpp
	audio-encoders.cpp
)

add_executable(livetest ${PROJECT_SOURCES})
target_link_libraries(livetest
    libobs Qt${QT_VERSION_MAJOR}::Widgets)
set_target_properties(livetest PROPERTIES FOLDER "frontend")

if(WIN32)
	set_property(TARGET livetest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/${OBS_EXECUTABLE_DESTINATION}/")
	set_property(TARGET livetest PROPERTY VS_DEBUGGER_COMMAND "${CMAKE_BINARY_DIR}/rundir/$<CONFIGURATION>/${OBS_EXECUTABLE_DESTINATION}/livetest.exe")
endif()

install_obs_core(livetest)
