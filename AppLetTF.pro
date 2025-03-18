QT += core gui widgets
TARGET = AppLetTFS
CONFIG += c++17

INCLUDEPATH += src            \
               src/tf_request \
               src/config

SOURCES += main.cpp

HEADERS += src/AppLetTF.h              \
           src/common.h                \
           src/config/Config.h         \
           src/methods.h               \
           src/tf_request/TFRequest.h

SOURCES += src/AppLetTF.cpp             \
           src/tf_request/TFRequest.cpp \
           src/config/Config.cpp        \
           src/methods.cpp

FORMS   += src/AppLetTF.ui

