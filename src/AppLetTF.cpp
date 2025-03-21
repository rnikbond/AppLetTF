//----------------------------------------
#include <QDir>
#include <QTime>
#include <QMenu>
#include <QDebug>
#include <QScreen>
#include <QTextBlock>
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
    setupArgs   ();

    QDir::setCurrent( workDirPath() );

    // Очистка рабочей директории от файлов, которые создал TF
    QStringList files = QDir(workDirPath()).entryList( QDir::Files );
    foreach( const QString& file, files ) {
        if( !file.endsWith(".cfg") && !files.endsWith(".dat") ) {
            QFile(file).remove();
        }
    }

    m_logAction     ->setChecked( true );
    m_maximizeAction->setChecked( true );
}
//----------------------------------------------------------------------------------------------------------

AppLetTF::~AppLetTF() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение проектов
 */
void AppLetTF::showProjects() {

    m_projectsAction->blockSignals( true  );
    m_changesAction ->blockSignals( true  );
    m_projectsAction->setChecked  ( true  );
    m_changesAction ->setChecked  ( false );
    m_projectsAction->blockSignals( false );
    m_changesAction ->blockSignals( false );

    ui->stackedWidget->setCurrentWidget( ui->projectsPage );
    reloadActions();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение изменений
 */
void AppLetTF::showChanges() {

    m_projectsAction->blockSignals( true  );
    m_changesAction ->blockSignals( true  );
    m_projectsAction->setChecked  ( false );
    m_changesAction ->setChecked  ( true  );
    m_projectsAction->blockSignals( false );
    m_changesAction ->blockSignals( false );

    ui->stackedWidget->setCurrentWidget( ui->changesPage );
    reloadActions();

    if( !ui->changes->isReloaded() ) {
        ui->changes->reload();
    }
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::reloadActions() {

    auto stretchWidget = [&]() {
        QWidget* widget = new QWidget;
        widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
        return widget;
    };

    QList<QAction*> actions;
    if( ui->stackedWidget->currentWidget() == ui->projectsPage ) {
        actions = ui->projectsTree->actions();
    } else {
        actions = ui->changes->actions();
    }

    ui->toolBar->clear();

    ui->toolBar->addAction ( m_projectsAction );
    ui->toolBar->addAction ( m_changesAction  );
    ui->toolBar->addSeparator();
    ui->toolBar->addActions( actions          );
    ui->toolBar->addWidget ( stretchWidget()  );
    ui->toolBar->addSeparator();
    ui->toolBar->addAction ( m_settingsAction );
    ui->toolBar->addSeparator();
    ui->toolBar->addAction ( m_logAction      );
    ui->toolBar->addAction ( m_maximizeAction );

    QLayout* toolBarLayout = ui->toolBar->layout();
    for(int i = 0; i < toolBarLayout->count(); ++i) {
        QLayoutItem* item = toolBarLayout->itemAt(i);
        item->setAlignment( Qt::AlignLeft );
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
        case QSystemTrayIcon::Trigger: {
            bool newState = !isVisible();
            setVisible( newState );
            if( newState ) {
                raise();
            }
            break;
        }
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

    ui->projectsTree->setConfig( m_config );
    ui->changes     ->setConfig( m_config );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение видимости вывода сообщений
 * \param state Признак видимости
 */
void AppLetTF::changeOutput( bool state ) {

    m_logAction->blockSignals( true );
    m_logAction->setChecked( state );
    m_logAction->blockSignals( false );

    ui->logEdit->setVisible( state );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение видимости текста на панели инструментов
 * \param state Признак видимости
 */
void AppLetTF::changeToolBarText( bool state ) {

    m_maximizeAction->blockSignals( true );
    m_maximizeAction->setChecked( state );
    m_maximizeAction->blockSignals( false );

    if( state ) {
        ui->toolBar->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    } else {
        ui->toolBar->setToolButtonStyle( Qt::ToolButtonIconOnly );
    }
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

    changeOutput     ( m_config.m_isLog     );
    changeToolBarText( m_config.m_isToolBar );

    ui->projectsTree->setConfig( m_config );
    ui->changes     ->setConfig( m_config );

    ui->changes->restoreData();

    showProjects();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка действий
 */
void AppLetTF::setupActions() {

    QActionGroup* tabsActionGroup = new QActionGroup( this );

    m_projectsAction = new QAction( tr("Проекты"       ) );
    m_changesAction  = new QAction( tr("Изменения"     ) );
    m_settingsAction = new QAction( tr("Настройки"     ) );
    m_logAction      = new QAction( tr("Вывод"         ) );
    m_maximizeAction = new QAction( tr("Подробный вид" ) );

    m_projectsAction->setIcon( QIcon(":/project_tree.png") );
    m_changesAction ->setIcon( QIcon(":/wait_edit.png"   ) );
    m_settingsAction->setIcon( QIcon(":/settings.png"    ) );
    m_logAction     ->setIcon( QIcon(":/message.png"     ) );
    m_maximizeAction->setIcon( QIcon(":/maximize.png"    ) );

    m_projectsAction->setCheckable( true );
    m_changesAction ->setCheckable( true );
    m_logAction     ->setCheckable( true );
    m_maximizeAction->setCheckable( true );

    tabsActionGroup->addAction( m_projectsAction );
    tabsActionGroup->addAction( m_changesAction  );

    connect( m_projectsAction, &QAction::triggered, this, &AppLetTF::showProjects     );
    connect( m_changesAction , &QAction::triggered, this, &AppLetTF::showChanges       );
    connect( m_settingsAction, &QAction::triggered, this, &AppLetTF::changeSettings    );
    connect( m_logAction     , &QAction::triggered, this, &AppLetTF::changeOutput      );
    connect( m_maximizeAction, &QAction::triggered, this, &AppLetTF::changeToolBarText );
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
    ui->toolBar->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

    connect( ui->projectsTree, &ProjectsTree::commandExecuted, this, &AppLetTF::appendOutput );

    addToolBar( Qt::LeftToolBarArea, ui->toolBar );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка события закрытия окна
 * \param event Событие
 *
 * Если установлен признак сворачивания в трей, событие игнорируется и приложение сворачивается в трей.
 */
void AppLetTF::closeEvent( QCloseEvent* event ) {

    m_config.save( geometry(), m_logAction->isChecked(), m_maximizeAction->isChecked() );
    ui->changes->saveData();

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

    int lines = 0;
    for( int idx = 0; idx < ui->logEdit->document()->blockCount(); idx++ ) {
        lines += ui->logEdit->document()->findBlock(idx).length();
    }
    if( lines > 3000 ) {
        ui->logEdit->clear();
    }

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
