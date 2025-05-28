QT     += core gui widgets
TARGET  = AppLetTF
CONFIG += c++17

INCLUDEPATH += $$PWD/src                \
               $$PWD/src/tf_request     \
               $$PWD/src/config         \
               $$PWD/src/settings       \
               $$PWD/src/projects_tree  \
               $$PWD/src/history        \
               $$PWD/src/history_detail \
               $$PWD/src/workspaces     \
               $$PWD/src/azure_tree     \
               $$PWD/src/changes

HEADERS += $$PWD/src/AppLetTF.h                           \
           $$PWD/src/azure_tree/AzureTreeDialog.h         \
           $$PWD/src/changes/ChangesWidget.h              \
           $$PWD/src/common.h                             \
           $$PWD/src/config/Config.h                      \
           $$PWD/src/methods.h                            \
           $$PWD/src/tf_request/TFRequest.h               \
           $$PWD/src/settings/SettingsDialog.h            \
           $$PWD/src/projects_tree/ProjectsTree.h         \
           $$PWD/src/history/HistoryWidget.h              \
           $$PWD/src/history_detail/HistoryDetailWidget.h \
           $$PWD/src/workspaces/NewWorkspaceDialog.h      \
           $$PWD/src/workspaces/WorkspacesDialog.h

SOURCES += $$PWD/main.cpp                                   \
           $$PWD/src/AppLetTF.cpp                           \
           $$PWD/src/azure_tree/AzureTreeDialog.cpp         \
           $$PWD/src/changes/ChangesWidget.cpp              \
           $$PWD/src/changes/ChangesWidget_detected.cpp     \
           $$PWD/src/changes/ChangesWidget_exclude.cpp      \
           $$PWD/src/changes/ChangesWidget_prepared.cpp     \
           $$PWD/src/tf_request/TFRequest.cpp               \
           $$PWD/src/config/Config.cpp                      \
           $$PWD/src/methods.cpp                            \
           $$PWD/src/settings/SettingsDialog.cpp            \
           $$PWD/src/settings/SettingsDialog_azure.cpp      \
           $$PWD/src/settings/SettingsDialog_common.cpp     \
           $$PWD/src/projects_tree/ProjectsTree.cpp         \
           $$PWD/src/history/HistoryWidget.cpp              \
           $$PWD/src/history_detail/HistoryDetailWidget.cpp \
           $$PWD/src/workspaces/NewWorkspaceDialog.cpp      \
           $$PWD/src/workspaces/WorkspacesDialog.cpp

FORMS   += $$PWD/src/AppLetTF.ui                           \
           $$PWD/src/azure_tree/AzureTreeDialog.ui         \
           $$PWD/src/changes/ChangesWidget.ui              \
           $$PWD/src/settings/SettingsDialog.ui            \
           $$PWD/src/projects_tree/ProjectsTree.ui         \
           $$PWD/src/history/HistoryWidget.ui              \
           $$PWD/src/history_detail/HistoryDetailWidget.ui \
           $$PWD/src/workspaces/NewWorkspaceDialog.ui      \
           $$PWD/src/workspaces/WorkspacesDialog.ui

RESOURCES += $$PWD/res/res.qrc

