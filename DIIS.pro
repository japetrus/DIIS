QMAKE_CFLAGS = -m32
QMAKE_CXXFLAGS = -m32
QMAKE_LFLAGS = -m32

LIBS += -L/Users/japetrus/PhD/Programs/DIIS/src/qwt-5.2.2/lib -lqwt

INCLUDEPATH += -I../qwt-5.2.2/src
QMAKE_CXXFLAGS += -O3 \
    -DHAVE_STD \
    -DHAVE_NAMESPACES
QT += network
ICON = resources/icon.icns

HEADERS = ConfigFile/ConfigFile.h \
    TwoThetaPlot.h \
    FilmWidget.h \
    AppWindow.h \
    AppConfig.h \
    LogWidget.h \
    TwoThetaWindow.h
SOURCES = main.cpp \
    ConfigFile/ConfigFile.cpp \
    TwoThetaPlot.cpp \
    AppWindow.cpp \
    FilmWidget.cpp \
    AppConfig.cpp \
    TwoThetaWindow.cpp
DESTDIR = ../bin

# install
TARGET = DIIS
target.path = ../bin
sources.path = ./
sources.files = $$SOURCES \
    $$HEADERS \
    DIIS.pro
INSTALLS += target \
    sources
FORMS += PreferencesDialog.ui \
    GeometryDialog.ui \
    MineralSearchDialog.ui \
    ImageDialog.ui \
    MacroDialog.ui
RESOURCES += AppResources.qrc
OTHER_FILES += TODO.txt
