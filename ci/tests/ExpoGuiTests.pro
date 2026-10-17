include(HeadTracker.pro)
QT += testlib
SOURCES -= main.cpp
SOURCES += expo_gui_tests.cpp
TARGET = ExpoGuiTests
CONFIG += console testcase
CONFIG -= app_bundle
