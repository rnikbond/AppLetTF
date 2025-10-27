//----------------------------------------
#ifndef APPLETTF_H
#define APPLETTF_H
//----------------------------------------
#include <QMainWindow>
#include <QSystemTrayIcon>
//----------------------------------------
#include "Config.h"
#include "ChangesetCache.h"
//----------------------------------------
namespace Ui { class AppLetTF; }
//----------------------------------------

class AppLetTF : public QMainWindow {

    Q_OBJECT

public:

    AppLetTF( QWidget* parent = nullptr );
    ~AppLetTF();

    void init();

private: // Config

    Config m_config;

    void changeSettings();

private: // Cache

    ChangesetCache* m_cache;

private: // Log

    void appendOutput( bool isErr, int code, const QString& err, const QStringList& response );

private: // Tray

    bool     m_tray_message;
    QMenu  * m_trayMenu  ;
    QAction* m_quitAction;
    QSystemTrayIcon* m_tray;

    void setupTray();
    void reactOnTray( QSystemTrayIcon::ActivationReason reason );

private: // Actions

    QAction* m_projectsAction;
    QAction* m_changesAction ;
    QAction* m_settingsAction;
    QAction* m_logAction     ;
    QAction* m_maximizeAction;

    void setupActions();

private: // UI

    Ui::AppLetTF* ui;

    void setupUI  ();
    void setupArgs();
    void moveToCenterScreen();

    void showProjects ();
    void showChanges  ();
    void reloadActions();

    void changeOutput( bool state );
    void changeToolBarText( bool state );

protected:

    void closeEvent( QCloseEvent* event ) override;
};
//----------------------------------------------------------------------------------------------------------

#endif // APPLETTF_H
