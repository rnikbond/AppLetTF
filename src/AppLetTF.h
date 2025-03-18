//----------------------------------------
#ifndef APPLETTF_H
#define APPLETTF_H
//----------------------------------------
#include <QMainWindow>
#include <QSystemTrayIcon>
//----------------------------------------
#include "Config.h"
#include "TFRequest.h"
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

private: // TF

    TFRequest* m_tf;

    void setupTF();
    void reactOnCmdExecuted();

private: // Tray

    bool     m_tray_message;
    QMenu  * m_trayMenu  ;
    QAction* m_quitAction;
    QSystemTrayIcon* m_tray;

    void setupTray();
    void reactOnTray( QSystemTrayIcon::ActivationReason reason );

private: // UI

    Ui::AppLetTF* ui;

    void setupUI  ();
    void setupArgs();
    void moveToCenterScreen();

protected:

    void closeEvent( QCloseEvent* event );
};
//----------------------------------------------------------------------------------------------------------

#endif // APPLETTF_H
