QT     += core gui widgets
TARGET  = AppLetTFS
CONFIG += c++17

INCLUDEPATH += src                \
               src/tf_request     \
               src/config         \
               src/settings       \
               src/projects_tree  \
               src/history        \
               src/history_detail

HEADERS += src/AppLetTF.h                           \
           src/common.h                             \
           src/config/Config.h                      \
           src/methods.h                            \
           src/tf_request/TFRequest.h               \
           src/settings/SettingsDialog.h            \
           src/projects_tree/ProjectsTree.h         \
           src/history/HistoryWidget.h              \
           src/history_detail/HistoryDetailWidget.h

SOURCES += main.cpp                                   \
           src/AppLetTF.cpp                           \
           src/tf_request/TFRequest.cpp               \
           src/config/Config.cpp                      \
           src/methods.cpp                            \
           src/settings/SettingsDialog.cpp            \
           src/settings/SettingsDialog_azure.cpp      \
           src/settings/SettingsDialog_common.cpp     \
           src/projects_tree/ProjectsTree.cpp         \
           src/history/HistoryWidget.cpp              \
           src/history_detail/HistoryDetailWidget.cpp

FORMS   += src/AppLetTF.ui                           \
           src/settings/SettingsDialog.ui            \
           src/projects_tree/ProjectsTree.ui         \
           src/history/HistoryWidget.ui              \
           src/history_detail/HistoryDetailWidget.ui

RESOURCES += res/res.qrc

