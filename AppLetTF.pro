QT += core gui widgets
TARGET = AppLetTFS
CONFIG += c++17

INCLUDEPATH += src            \
               src/tf_request \
               src/config     \
               src/settings

SOURCES += main.cpp

HEADERS += src/AppLetTF.h                \
           src/common.h                  \
           src/config/Config.h           \
           src/methods.h                 \
           src/tf_request/TFRequest.h    \
           src/settings/SettingsDialog.h

SOURCES += src/AppLetTF.cpp                      \
           src/tf_request/TFRequest.cpp          \
           src/config/Config.cpp                 \
           src/methods.cpp                       \
           src/settings/SettingsDialog.cpp       \
           src/settings/SettingsDialog_azure.cpp

FORMS   += src/AppLetTF.ui                \
           src/settings/SettingsDialog.ui

