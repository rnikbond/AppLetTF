//----------------------------------------
#include <QFileDialog>
#include <QPushButton>
//----------------------------------------
#include "AzureTreeDialog.h"
//----------------------------------------
#include "NewWorkspaceDialog.h"
#include "ui_NewWorkspaceDialog.h"
//----------------------------------------

NewWorkspaceDialog::NewWorkspaceDialog( QWidget* parent ) : QDialog(parent), ui(new Ui::NewWorkspaceDialog)
{
    setupUI();
    updateStateOK();
}
//----------------------------------------------------------------------------------------------------------

NewWorkspaceDialog::~NewWorkspaceDialog() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение имени рабочей области
 * \return Имя рабочей области
 */
QString NewWorkspaceDialog::name() const {
    return ui->nameEdit->text();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение пути к каталогу Azure для сопоставления
 * \return Путь к каталогу Azure
 */
QString NewWorkspaceDialog::azureFolderPath() const {
    return ui->azureFolderEdit->text();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение локального пути к каталогу для сопоставления
 * \return Локальный путь к каталогу
 */
QString NewWorkspaceDialog::localFolderPath() const {
    return ui->localFolderEdit->text();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение примечания
 * \return Примечание
 */
QString NewWorkspaceDialog::comment() const {
    return ui->commentEdit->text();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Выбора каталога Azure
 */
void NewWorkspaceDialog::selectAzureFolder() {

    AzureTreeDialog dialog;
    connect( &dialog, &AzureTreeDialog::commandExecuted, this, &NewWorkspaceDialog::commandExecuted );

    dialog.setConfig( m_config );
    if( dialog.exec() != QDialog::Accepted ) {
        return;
    }

    QString path = dialog.selectedPath();
    ui->azureFolderEdit->setText( path );

    updateStateOK();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Выбора локального каталога
 */
void NewWorkspaceDialog::selectLocalFolder() {

    QString path = QFileDialog::getExistingDirectory( this, tr("Выбор каталога для сопоставления"), QDir::homePath() );
    ui->localFolderEdit->setText( path );

    updateStateOK();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обновление состояния кнопки OK
 */
void NewWorkspaceDialog::updateStateOK() {

    bool isOk  = !ui->nameEdit->text().isEmpty();
         isOk &= !ui->azureFolderEdit->text().isEmpty();
         isOk &= !ui->localFolderEdit->text().isEmpty();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( isOk );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void NewWorkspaceDialog::setConfig( const Config& cfg ) {

    m_config = cfg;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка интерфейса
 */
void NewWorkspaceDialog::setupUI() {

    ui->setupUi( this );
    setWindowTitle( tr("Создание рабочего пространства") );

    ui->azureFolderEdit->setReadOnly( true );
    ui->localFolderEdit->setReadOnly( true );

    connect( ui->nameEdit         , &QLineEdit::textChanged, this, &NewWorkspaceDialog::updateStateOK     );
    connect( ui->azureFolderButton, &QToolButton::clicked  , this, &NewWorkspaceDialog::selectAzureFolder );
    connect( ui->localFolderButton, &QToolButton::clicked  , this, &NewWorkspaceDialog::selectLocalFolder );
}
//----------------------------------------------------------------------------------------------------------
