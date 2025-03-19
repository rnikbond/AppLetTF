QT     += core gui widgets
TARGET  = AppLetTFS
CONFIG += c++17

INCLUDEPATH += src                \
               src/tf_request     \
               src/config         \
               src/settings       \
               src/projects_tree  \
               src/history        \
               src/history_detail \
               src/workspaces     \
               src/azure_tree     \
               src/changes

HEADERS += src/AppLetTF.h                           \
           src/azure_tree/AzureTreeDialog.h         \
           src/changes/ChangesWidget.h              \
           src/common.h                             \
           src/config/Config.h                      \
           src/methods.h                            \
           src/tf_request/TFRequest.h               \
           src/settings/SettingsDialog.h            \
           src/projects_tree/ProjectsTree.h         \
           src/history/HistoryWidget.h              \
           src/history_detail/HistoryDetailWidget.h \
           src/workspaces/NewWorkspaceDialog.h      \
           src/workspaces/WorkspacesDialog.h

SOURCES += main.cpp                                   \
           src/AppLetTF.cpp                           \
           src/azure_tree/AzureTreeDialog.cpp         \
           src/changes/ChangesWidget.cpp              \
           src/changes/ChangesWidget_detected.cpp     \
           src/changes/ChangesWidget_exclude.cpp      \
           src/changes/ChangesWidget_prepared.cpp     \
           src/tf_request/TFRequest.cpp               \
           src/config/Config.cpp                      \
           src/methods.cpp                            \
           src/settings/SettingsDialog.cpp            \
           src/settings/SettingsDialog_azure.cpp      \
           src/settings/SettingsDialog_common.cpp     \
           src/projects_tree/ProjectsTree.cpp         \
           src/history/HistoryWidget.cpp              \
           src/history_detail/HistoryDetailWidget.cpp \
           src/workspaces/NewWorkspaceDialog.cpp      \
           src/workspaces/WorkspacesDialog.cpp

FORMS   += src/AppLetTF.ui                           \
           src/azure_tree/AzureTreeDialog.ui         \
           src/changes/ChangesWidget.ui              \
           src/settings/SettingsDialog.ui            \
           src/projects_tree/ProjectsTree.ui         \
           src/history/HistoryWidget.ui              \
           src/history_detail/HistoryDetailWidget.ui \
           src/workspaces/NewWorkspaceDialog.ui      \
           src/workspaces/WorkspacesDialog.ui

RESOURCES += res/res.qrc

