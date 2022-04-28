QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

CONFIG += c++11

DEFINES +=__STDC_LIMIT_MACROS

UI_DIR = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj

#将输出文件放到源码目录下的bin目录中
contains(QT_ARCH,i386){
    message("32-bit")
    DESTDIR = $${PWD}/bin32
} else{
    message("64-bit")
    DESTDIR = $${PWD}/bin64
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ASongAudio.cpp \
    ASongPlayer.cpp \
    ASongVideo.cpp \
    MainWindow.cpp \
    Main.cpp

HEADERS += \
    ASongAudio.h \
    ASongPlayer.h \
    ASongVideo.h \
    MainWindow.h

FORMS += \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32{
    contains(QT_ARCH,i386){
        message("32-bit")
        INCLUDEPATH += $$PWD/lib/win32/third-party/include \
                       $$PWD/src
        LIBS += -L$$PWD/lib/win32/third-party/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale
    }else{
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/win64/third-party/include \
                       $$PWD/src
        LIBS += -L$$PWD/lib/win64/third-party/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale
    }
}
