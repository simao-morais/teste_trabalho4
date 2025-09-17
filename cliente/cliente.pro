QT += widgets
SOURCES += Cliente.cpp ClientWindow.cpp
HEADERS += ClientWindow.h
# Incluir as bibliotecas necessárias para a sua lógica HTTP e JSON
LIBS += -ljsoncpp -lpthread