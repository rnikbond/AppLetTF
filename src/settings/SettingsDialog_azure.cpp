//----------------------------------------
#include <QUuid>
#include <QTime>
#include <QDebug>
#include <QFileDialog>
//----------------------------------------
#include "methods.h"
#include "TFRequest.h"
#include "WorkspacesDialog.h"
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

/*!
 * \brief Изменение пути к программе TF
 */
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
 * \brief Изменение рабочей области
 */
void SettingsDialog::changeWorkspace() {

    WorkspacesDialog workspacesDialog;
    connect( &workspacesDialog, &WorkspacesDialog::commandExecuted, this, &SettingsDialog::commandExecuted );
    workspacesDialog.setConfig( m_config );
    if( workspacesDialog.exec() != QDialog::Accepted ) {
        return;
    }

    Workspace workspace = workspacesDialog.selectedWorkspace();
    m_config.m_azure.workspace = workspace.name;

    foreach( const WorkfoldItem& workfold, workspace.workfolds ) {
        m_config.m_azure.workfoldes[workfold.pathServer] = workfold.pathLocal;
    }

    ui->workspaceEdit->setText( m_config.m_azure.workspace );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Переподключение к Azure DevOps Server
 */
void SettingsDialog::reconnectAzure() {

    ui->infoEdit->clear();

    saveConfigPageAzure();

    if( m_config.m_azure.tfPath.isEmpty() ) {
        logAzureError( tr("Не указан путь к программе TF") );
        ui->tfPathEdit->setFocus();
        return;
    }

    if( m_config.m_azure.diffCmd.isEmpty() ) {
        logAzureWarning( tr("Не указана программа для сравнения TF_DIFF_COMMAND") );
    }

    if( !QFile(m_config.m_azure.tfPath).exists() ) {
        logAzureError( tr("Не найдена программа TF") );
        ui->tfPathEdit->setFocus();
        return;
    }

    if( m_config.m_azure.url.isEmpty() ) {
        logAzureError( tr("Не указан URL") );
        ui->asureUrlEdit->setFocus();
        return;
    }

    if( m_config.m_azure.login.isEmpty() ) {
        logAzureError( tr("Не указан Логин") );
        ui->loginEdit->setFocus();
        return;
    }

    if( m_config.m_azure.password.isEmpty() ) {
        logAzureError( tr("Не указан Пароль") );
        ui->passwordEdit->setFocus();
        return;
    }

    if( m_config.m_azure.workspace.isEmpty() ) {
        logAzureError( tr("Не настроена рабочая область") );
        ui->workspaceButton->setFocus();
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
 * \brief Сохранение конфигурации страницы "Azure DevOps Server"
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
 * \brief Восстановление конфигурации страницы "Azure DevOps Server"
 */
void SettingsDialog::restoreConfigPageAzure() {

    ui->tfPathEdit  ->setText( m_config.m_azure.tfPath   );
    ui->diffToolEdit->setText( m_config.m_azure.diffCmd  );
    ui->asureUrlEdit->setText( m_config.m_azure.url      );
    ui->loginEdit   ->setText( m_config.m_azure.login    );
    ui->passwordEdit->setText( m_config.m_azure.password );

    ui->workspaceEdit->setText( m_config.m_azure.workspace );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Инициализация страницы "Azure DevOps Server"
 */
void SettingsDialog::initPageAzure() {

    ui->tfPathEdit  ->setPlaceholderText( EXAMPLE_TF_BIN );
    ui->diffToolEdit->setPlaceholderText( "/usr/bin/bcompare %1 %2" );
    ui->infoEdit    ->setPlaceholderText( tr("Здесь будет отображаться информация о подключении...") );

    ui->azureChechButton->setIcon( QIcon(":/settings_update.png") );

    ui->workspaceEdit->setReadOnly( true );

    connect( ui->tfPathButton    , &QToolButton::clicked, this, &SettingsDialog::changeTfPath    );
    connect( ui->workspaceButton , &QToolButton::clicked, this, &SettingsDialog::changeWorkspace );
    connect( ui->azureChechButton, &QPushButton::clicked, this, &SettingsDialog::reconnectAzure  );
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

