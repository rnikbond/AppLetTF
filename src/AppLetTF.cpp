//----------------------------------------
#include <QMenu>
#include <QDebug>
#include <QScreen>
#include <QCloseEvent>
//----------------------------------------
#include "methods.h"
#include "SettingsDialog.h"
//----------------------------------------
#include "AppLetTF.h"
#include "ui_AppLetTF.h"
//----------------------------------------

AppLetTF::AppLetTF( QWidget* parent ) : QMainWindow(parent), ui(new Ui::AppLetTF)
{
    ui->setupUi(this);
    setupUI  ();
    setupTray();

    m_tf = new TFRequest( this );
    m_tf->setAsync( true );

    connect( m_tf, &TFRequest::executed, this, &AppLetTF::reactOnCmdExecuted );

    connect( ui->pushButton, &QPushButton::clicked, [this]{
        changeSettings();
    } );
}
//----------------------------------------------------------------------------------------------------------

AppLetTF::~AppLetTF() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::reactOnCmdExecuted() {

    if( m_tf->m_isErr ) {
        qDebug() << m_tf->m_errCode << m_tf->m_errText;
        return;
    }

    switch( m_tf->m_cmd ) {

        case TFRequest::CommandWorkspaces: {
            auto workspaces = parseWorkspaces( m_tf->m_response );
            foreach(auto workspace, workspaces) {
                qDebug() << workspace.name << workspace.owner << workspace.computer << workspace.comment;
            }
            break;
        }

        default: break;
    }
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::reactOnTray( QSystemTrayIcon::ActivationReason reason ) {

    Q_UNUSED( reason );

    switch (reason){
        case QSystemTrayIcon::Trigger:setVisible( !isVisible() ); break;
        default: break;
    }
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::changeSettings() {

    SettingsDialog settings;
    settings.setConfig( m_config );
    if( settings.exec() != QDialog::Accepted ) {
        return;
    }

    m_config = settings.config();
    m_config.save( geometry() );
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::init() {

    QString cfgPath;

    foreach( const QString& arg, qApp->arguments() ) {

        QStringList argParts = arg.split("=");
        if( argParts.count() != 2 ) {
            continue;
        }

        if( argParts.at(0) == "-c" || argParts.at(0) == "--config" ) {
            cfgPath = argParts.at(1);
            break;
        }
    }

    m_config.init( cfgPath );
    m_config.restore();

    if( !m_config.isIncomplete() ) {
        changeSettings();
    }

    m_tf->setConfig( m_config );
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::setupTray() {

    m_tray_message = true;
    m_trayMenu     = new QMenu(this);
    m_quitAction   = new QAction( QIcon(":/exit.png"), tr("Закрыть"), this);
    m_trayMenu->addAction( m_quitAction );
    connect( m_quitAction, &QAction::triggered, qApp, &QApplication::quit );

    m_tray = new QSystemTrayIcon( this );
    m_tray->setIcon( QIcon(":/branch_vsc.png") );
    m_tray->setToolTip("AppLet TF");
    m_tray->setContextMenu( m_trayMenu );
    m_tray->show();

    connect( m_tray, &QSystemTrayIcon::activated, this, &AppLetTF::reactOnTray );
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::setupUI() {

    setWindowTitle( tr("AppLet TF") );
    moveToCenterScreen();
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::closeEvent( QCloseEvent* event ) {

    m_config.save( geometry() );

    if( !m_config.m_tray ) {
        QMainWindow::closeEvent( event );
        return;
    }

    event->ignore();
    hide();

    if( m_tray_message ) {
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
        m_tray->showMessage("AppLet TF", tr("Приложение свернуто в трей"), icon, 2000 );
        m_tray_message = false;
    }
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::moveToCenterScreen() {

    QRect rect  = QGuiApplication::primaryScreen()->geometry();
    QPoint center = rect.center();
    center.setX( center.x() - (width ()/2) );
    center.setY( center.y() - (height()/2) );
    move( center );
}
//----------------------------------------------------------------------------------------------------------
