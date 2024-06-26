QT       += core gui multimedia sensors gamepad

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogoPashka

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += ONB

CONFIG += c++14

COMPONENTS = d:/projects/qt/components5
include($$COMPONENTS/panel3d/panel3d.pri)
include($$COMPONENTS/megawidgets/megawidgets.pri)
contains (DEFINES, ONB) {
    include($$COMPONENTS/onb/onb.pri)
}

win32: {
    include($$COMPONENTS/usbhid/usbhid.pri)
    DESTDIR = $$PWD/$$TARGET
    CONFIG(release,debug|release) {
        QMAKE_POST_LINK += windeployqt --no-translations $$DESTDIR
    }
}

SOURCES += \
    consoleedit.cpp \
    font8x8.c \
    logointerpreter.cpp \
    logoprocedure.cpp \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    programcontext.cpp \
    robotmodel.cpp \
    scene.cpp \
    sound.cpp

HEADERS += \
    consoleedit.h \
    logointerpreter.h \
    logoprocedure.h \
    mainwindow.h \
    codeeditor.h \
    programcontext.h \
    robotmodel.h \
    scene.h \
    sound.h \
    turtleinterface.h

contains (DEFINES, ONB) {
    SOURCES += onbturtle.cpp
    HEADERS += onbturtle.h
}

#win32: {
#SOURCES += spacemouse.cpp
#HEADERS += spacemouse.h
#}

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RC_ICONS = icon.ico

RESOURCES += \
    res.qrc

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
