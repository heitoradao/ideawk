# ideawk - a simple awk ide
# Heitor Adao Junior <heitor.adao@gmail.com>
# 2017-04

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++14

HEADERS       = mainwindow.h awksyntaxhighlighter.h
SOURCES       = main.cpp mainwindow.cpp awksyntaxhighlighter.cpp
RESOURCES     = sdi.qrc

TRANSLATIONS += pt_br.ts

FORMS += mainwindow.ui
