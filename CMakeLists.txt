cmake_minimum_required(VERSION 3.16)

project(untitled6 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(untitled6
        main.cpp
        mainwindow.cpp
        mainwindow.h
        bot.h
        bot.cpp
)

target_link_libraries(untitled6
        PRIVATE Qt6::Widgets
)