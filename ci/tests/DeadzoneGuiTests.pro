include(HeadTracker.pro)
QT += testlib
SOURCES -= main.cpp
SOURCES += deadzone_gui_tests.cpp
TARGET = DeadzoneGuiTests
CONFIG += console testcase
CONFIG -= app_bundle
