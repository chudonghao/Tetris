cmake_minimum_required(VERSION 3.16)
project(Tetris)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 COMPONENTS Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(
  Tetris
  main.cpp
  MainWindow.cpp
  MainWindow.h
  MainWindow.ui
  GameView.cpp
  GameView.h
  GameScene.cpp
  GameScene.h
)

target_link_libraries(Tetris PRIVATE Qt5::Widgets)
