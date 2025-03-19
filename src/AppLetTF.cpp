//----------------------------------------
#include <QTime>
#include <QMenu>
#include <QDebug>
#include <QScreen>
#include <QCloseEvent>
#include <QMessageBox>
//----------------------------------------
#include "methods.h"
#include "SettingsDialog.h"
//----------------------------------------
#include "AppLetTF.h"
#include "ui_AppLetTF.h"
//----------------------------------------

AppLetTF::AppLetTF( QWidget* parent ) : QMainWindow(parent), ui(new Ui::AppLetTF)
{
    createWorkDir();

    setupActions();
    setupUI     ();
    setupArgs   ();
    setupTray   ();
    setupTF     ();
    setupArgs   ();
}
//----------------------------------------------------------------------------------------------------------

AppLetTF::~AppLetTF() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка выполненой команды TF
 */
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

/*!
 * \brief Обработка клика по иконке в трее
 * \param reason Не используется
 */
void AppLetTF::reactOnTray( QSystemTrayIcon::ActivationReason reason ) {

    Q_UNUSED( reason );

    switch (reason){
        case QSystemTrayIcon::Trigger:setVisible( !isVisible() ); break;
        default: break;
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение настроек
 *
 * Если в окне настроек нажата кнопка "ОК", конфигурация будет сохранена.
 */
void AppLetTF::changeSettings() {

    SettingsDialog settings;
    connect( &settings, &SettingsDialog::commandExecuted, this, &AppLetTF::appendOutput );

    settings.setConfig( m_config );
    if( settings.exec() != QDialog::Accepted ) {
        return;
    }

    m_config = settings.config();
    m_config.save( geometry() );

    m_tf            ->setConfig( m_config );
    ui->projectsTree->setConfig( m_config );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Инициализация
 *
 * Если нет файла конфигурации или в конфигурации есть ошибки, вызывается окно для настройки конфигурации.
 */
void AppLetTF::init() {

    m_config.restore();

    if( m_config.m_geometry.isValid() ) {
        setGeometry( m_config.m_geometry );
    }

    if( !m_config.isIncomplete() ) {
        changeSettings();
    }

    if( !m_config.m_azure.diffCmd.isEmpty() ) {
        if( !qputenv("TF_DIFF_COMMAND", m_config.m_azure.diffCmd.toUtf8().data()) ) {
            QMessageBox::warning( this, tr("Ошибка"), tr("Не удалось установить переменнуж окружения TF_DIFF_COMMAND"), QMessageBox::Close );
        }
    }

    m_tf            ->setConfig( m_config );
    ui->projectsTree->setConfig( m_config );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка действий
 */
void AppLetTF::setupActions() {

    m_settingsAction = new QAction( tr("Настройки") );

    connect( m_settingsAction, &QAction::triggered, this, &AppLetTF::changeSettings );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Разбор аргументов командной строки
 *
 *  При инициализации считывается аргмент командной строки, в котором указан путь к файлу конфигурации:
 * -c=/home/user/applettf.cfg
 * --config=/home/user/applettf.cfg
 *
 * Если такого аргемента нет, используется путь к файлу конфигурации по-умолчанию: "applettf.cfg"
 */
void AppLetTF::setupArgs() {

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
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка инструмента для работы с программой TF
 */
void AppLetTF::setupTF() {

    m_tf = new TFRequest( this );
    m_tf->setAsync( true );

    connect( m_tf, &TFRequest::executed, this, &AppLetTF::reactOnCmdExecuted );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка инструментов для работы в трее
 */
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

/*!
 * \brief Настройка интерфейса окна
 */
void AppLetTF::setupUI() {

    ui->setupUi( this );
    setWindowTitle( tr("AppLet TF") );
    setWindowIcon( QIcon(":/branch_vsc.png") );
    moveToCenterScreen();

    ui->toolBar->setMovable( false );
    ui->toolBar->setIconSize( QSize(20, 20) );
    ui->toolBar->addAction( m_settingsAction );

    connect( ui->projectsTree, &ProjectsTree::commandExecuted, this, &AppLetTF::appendOutput );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка события закрытия окна
 * \param event Событие
 *
 * Если установлен признак сворачивания в трей, событие игнорируется и приложение сворачивается в трей.
 */
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

/*!
 * \brief Вывод инфомарции
 * \param isErr    Признак ошибки
 * \param code     Код ошибки
 * \param err      Текст ошибки
 * \param response Список значений из ответа, если нет ошибки
 */
void AppLetTF::appendOutput( bool isErr, int code, const QString& err, const QStringList& response ) {

    ui->logEdit->append("");
    ui->logEdit->append( QTime::currentTime().toString("hh:mm:ss.zzz") );

    if( isErr) {
        QColor textColorSave = ui->logEdit->textColor();

        ui->logEdit->setTextColor( Qt::red );

        ui->logEdit->append( QString("Ошибка %1").arg(code));
        ui->logEdit->append( err );

        ui->logEdit->setTextColor( textColorSave );
        return;
    }

    if( !response.isEmpty() ) {
        ui->logEdit->append( response.join('\n') );
    } else {
        ui->logEdit->append( "Success" );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перемещение окна в центр экрана
 */
void AppLetTF::moveToCenterScreen() {

    QRect rect  = QGuiApplication::primaryScreen()->geometry();
    QPoint center = rect.center();
    center.setX( center.x() - (width ()/2) );
    center.setY( center.y() - (height()/2) );
    move( center );
}
//----------------------------------------------------------------------------------------------------------
