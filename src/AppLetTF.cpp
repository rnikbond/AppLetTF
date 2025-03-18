//----------------------------------------
#include <QDebug>
#include <QScreen>
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
    setupUI();

    m_tf = new TFRequest( this );
    m_tf->setAsync( true );

    connect( m_tf, &TFRequest::executed, this, &AppLetTF::reactOnCmdExecuted );

    connect( ui->pushButton, &QPushButton::clicked, [this]{
        m_tf->workspaces();
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

        SettingsDialog settings;
        settings.setConfig( m_config );
        if( settings.exec() == QDialog::Accepted ) {
            m_config = settings.config();
            m_config.save( geometry() );
        }
    }

    m_tf->setConfig( m_config );
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::setupUI() {

    setWindowTitle( tr("AppLet TF") );
    moveToCenterScreen();
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
