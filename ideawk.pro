# ideawk - a simple awk ide
# march, 2017
# Heitor Adao Junior <heitor.adao@gmail.com>

QT += widgets

HEADERS       = mainwindow.h awksyntaxhighlighter.h
SOURCES       = main.cpp \
                mainwindow.cpp awksyntaxhighlighter.cpp
RESOURCES     = sdi.qrc

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/sdi
#INSTALLS += target
