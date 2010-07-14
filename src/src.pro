LIBS += -lXcomposite -lXrender -lXdamage

isEmpty(PREFIX):PREFIX=/usr/local

HEADERS += client.h main.h utils.h workspace.h debug.h extensions.h atoms.h
SOURCES += client.cpp main.cpp workspace.cpp debug.cpp extensions.cpp atoms.cpp

TARGET = compmgr
target.path = $$PREFIX/bin

INSTALLS += target
