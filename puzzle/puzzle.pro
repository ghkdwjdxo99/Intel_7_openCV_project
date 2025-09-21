QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    draggablelistwidget.cpp \
    main.cpp \
    make_puzzle_image.cpp \
    playpage.cpp \
    puzzle.cpp \
    webcam_capture.cpp

HEADERS += \
    make_puzzle_image.h \
    draggablelistwidget.h \
    playpage.h \
    puzzle.h \
    webcam_capture.h

FORMS += \
    playpage.ui \
    make_puzzle_image.ui \
    puzzle.ui \
    webcam_capture.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$OUT_PWD
