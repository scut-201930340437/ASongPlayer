QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

CONFIG += c++11

DEFINES +=__STDC_LIMIT_MACROS

UI_DIR = obj/Gui
MOC_DIR = obj/Moc
OBJECTS_DIR = obj/Obj

#release版本可调试
QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#release版也将生成“.pdb”后缀的调试信息文件
QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /DEBUG
#调用库
LIBS += -lDbgHelp

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ASongAudio.cpp \
    ASongAudioOutput.cpp \
    ASongFFmpeg.cpp \
    ASongVideo.cpp \
    DataSink.cpp \
    Main.cpp \
    MainWindow.cpp \
    MessageTitleBar.cpp \
    MyMessageWidget.cpp \
    MySlider.cpp \
    MyTitleBar.cpp \
    PlayTable.cpp \
    SDLPaint.cpp \
    VideoPreview.cpp \
    VolumeSlider.cpp

HEADERS += \
    ASongAudio.h \
    ASongAudioOutput.h \
    ASongFFmpeg.h \
    ASongVideo.h \
    DataSink.h \
    MainWindow.h \
    MessageTitleBar.h \
    MyMessageWidget.h \
    MySlider.h \
    MyTitleBar.h \
    PlayTable.h \
    SDLPaint.h \
    VideoPreview.h \
    VolumeSlider.h

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
        LIBS += -L$$PWD/lib/win32/third-party/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale -lSDL2 -lSDL2main -lSoundTouchDll_x64
    }else{
        message("64-bit")
        INCLUDEPATH += $$PWD/lib/win64/third-party/include \
                       $$PWD/src
        LIBS += -L$$PWD/lib/win64/third-party/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale -lSDL2 -lSDL2main -lSoundTouchDll_x64
    }
}

RESOURCES += \
    src.qrc
