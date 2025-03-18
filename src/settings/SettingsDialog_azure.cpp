//----------------------------------------
#include <QTime>
#include <QDebug>
#include <QFileDialog>
//----------------------------------------
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
//----------------------------------------
#ifdef WIN32
 #define BIN_FILTER     "*.exe"
 #define EXAMPLE_TF_BIN "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/TeamFoundation/Team Explorer/TF.exe"
#else
 #define BIN_FILTER ""
 #define EXAMPLE_TF_BIN "/home/user/TEE-CLC-xx.xxx.x/tf"
#endif
//----------------------------------------

void SettingsDialog::changeTfPath() {

    QString dir = m_config.m_azure.tfPath.isEmpty() ? QDir::homePath() : m_config.m_azure.tfPath;
    QString path = QFileDialog::getOpenFileName( this, tr("Программа TF"), dir, BIN_FILTER );
    if( path.isEmpty() ) {
        return;
    }

    ui->tfPathEdit->setText( QDir::fromNativeSeparators(path) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Переподключение к Azure DevOps Server
 */
void SettingsDialog::reconnectAzure() {

    ui->infoEdit->clear();

    saveConfigPageAzure();

    if( m_config.m_azure.tfPath.isEmpty() ) {
        logAzureError( "Не указан путь к программе TF" );
        ui->tfPathEdit->setFocus();
        return;
    }

    if( m_config.m_azure.diffCmd.isEmpty() ) {
        logAzureWarning( "Не указана программа для сравнения TF_DIFF_COMMAND" );
    }

    if( !QFile(m_config.m_azure.tfPath).exists() ) {
        logAzureError( "Не найдена программа TF" );
        ui->tfPathEdit->setFocus();
        return;
    }

    if( m_config.m_azure.url.isEmpty() ) {
        logAzureError( "Не указан URL" );
        ui->asureUrlEdit->setFocus();
        return;
    }

    if( m_config.m_azure.login.isEmpty() ) {
        logAzureError( "Не указан Логин" );
        ui->loginEdit->setFocus();
        return;
    }

    if( m_config.m_azure.password.isEmpty() ) {
        logAzureError( "Не указан Пароль" );
        ui->passwordEdit->setFocus();
        return;
    }

    QList<Workspace> workspacesAzure;
    if( !azureWorkspaces(workspacesAzure) ) {
        return;
    }

    if( !m_config.m_azure.workspace.isEmpty() ) {

        if( !isExistsAzureWorkspace(workspacesAzure, m_config.m_azure.workspace) ) {
            logAzureWarning( tr("Рабочее пространство %1 не найдено").arg(m_config.m_azure.workspace) );
            m_config.m_azure.workspace.clear();
        }
    }

    if( m_config.m_azure.workspace.isEmpty() ) {
        logAzureInfo( tr("Подключение к рабочему пространству...") );
        if( !initAzureWorkspace(workspacesAzure) ) {
            return;
        }
    }

    if( m_config.m_azure.workspace.isEmpty() ) {
        logAzureError( tr("Не удалось подключиться к рабочему пространству") );
        return;
    }

    TFRequest tf;
    tf.setConfig( m_config );
    tf.checkConnection();

    if( tf.m_isErr) {
        logAzureError( tf.m_errText );
        return;
    }

    logAzureSuccess( "Подключение успешно установлено" );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Проверка существования рабочего пространства
 * \param workspacesAzure Список рабочих пространств
 * \param name Название рабочего пространства, которое нужно проверить
 * \return TRUE, если существует. Иначе FALSE.
 */
bool SettingsDialog::isExistsAzureWorkspace( const QList<Workspace>& workspacesAzure, const QString& name ) {

    foreach( const Workspace& workspace, workspacesAzure ) {
        if( QString::compare(workspace.name, name, Qt::CaseInsensitive) == 0 ) {
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получени списка абочих пространств с Azure DevOps Server
 * \param[out] workspacesAzure Список рабочих пространств
 * \return TRUE, если удалось загрузить информацию. Иначе FALSE.
 */
bool SettingsDialog::azureWorkspaces( QList<Workspace>& workspacesAzure ) {

    workspacesAzure.clear();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.workspaces();

    if( tf.m_isErr ) {
        logAzureError( tf.m_errText );
        return false;
    }

    workspacesAzure = parseWorkspaces( tf.m_response );
    return true;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Инициализация рабочего пространства
 * \param workspacesAzure Список рабочих пространств
 * \return TRUE, если удалось подключиться к рабочему пространстру или создать его. Иначе False.
 *
 * В качестве имени рабочего пространства используется имя ПК.
 * Если такое имя существует в \a workspacesAzure, то используется это рабочее пространство.
 * Если такого рабочего пространства нет в \a workspacesAzure, то выполняется запрос на его создание.
 */
bool SettingsDialog::initAzureWorkspace( const QList<Workspace>& workspacesAzure ) {

    QString machineHostName = QSysInfo::machineHostName();

    if( !isExistsAzureWorkspace(workspacesAzure, machineHostName) ) {

        logAzureInfo( tr("Создается рабочее пространство: %1").arg(machineHostName) );

        TFRequest tf;
        tf.setConfig( m_config );
        tf.createWorkspace( machineHostName );
        if( tf.m_isErr ) {
            logAzureError( tf.m_errText );
            return false;
        }

    } else {
        m_config.m_azure.workspace = QSysInfo::machineHostName();
    }

    logAzureSuccess( tr("Подключено к рабочему пространсву: %1").arg(m_config.m_azure.workspace) );

    return true;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сохранение конфигурации
 */
void SettingsDialog::saveConfigPageAzure() {

    m_config.m_azure.tfPath   = ui->tfPathEdit  ->text();
    m_config.m_azure.diffCmd  = ui->diffToolEdit->text();
    m_config.m_azure.url      = ui->asureUrlEdit->text();
    m_config.m_azure.login    = ui->loginEdit   ->text();
    m_config.m_azure.password = ui->passwordEdit->text();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Восстановление конфигурации
 */
void SettingsDialog::restoreConfigPageAzure() {

    ui->tfPathEdit  ->setText( m_config.m_azure.tfPath   );
    ui->diffToolEdit->setText( m_config.m_azure.diffCmd );
    ui->asureUrlEdit->setText( m_config.m_azure.url      );
    ui->loginEdit   ->setText( m_config.m_azure.login    );
    ui->passwordEdit->setText( m_config.m_azure.password );
}
//----------------------------------------------------------------------------------------------------------

void SettingsDialog::initPageAzure() {

    ui->tfPathEdit  ->setPlaceholderText( EXAMPLE_TF_BIN );
    ui->diffToolEdit->setPlaceholderText( "/usr/bin/bcompare %1 %2" );
    ui->infoEdit    ->setPlaceholderText( tr("Здесь будет отображаться информация о подключении...") );

    ui->azureChechButton->setIcon( QIcon(":/settings_update.png") );

    connect( ui->tfPathButton    , &QToolButton::clicked, this, &SettingsDialog::changeTfPath   );
    connect( ui->azureChechButton, &QPushButton::clicked, this, &SettingsDialog::reconnectAzure );
}
//----------------------------------------------------------------------------------------------------------

void SettingsDialog::logAzureInfo( const QString& text ) {
    logAzureText( text, Qt::darkGray );
}
//----------------------------------------------------------------------------------------------------------

void SettingsDialog::logAzureError( const QString& text ) {
    logAzureText( text, Qt::darkRed );
}
//----------------------------------------------------------------------------------------------------------

void SettingsDialog::logAzureWarning( const QString& text ) {
    logAzureText( text, Qt::darkYellow );
}
//----------------------------------------------------------------------------------------------------------

void SettingsDialog::logAzureSuccess( const QString& text ) {
    logAzureText( text, Qt::darkGreen );
}
//----------------------------------------------------------------------------------------------------------

void SettingsDialog::logAzureText( const QString& text, const QColor& color ) {

    QColor textColorSave = ui->infoEdit->textColor();
    ui->infoEdit->setTextColor( color );
    ui->infoEdit->append( QString("%1: %2").arg(QTime::currentTime().toString("hh:mm:ss.zzz"), text) );
    ui->infoEdit->setTextColor( textColorSave );
}
//----------------------------------------------------------------------------------------------------------

