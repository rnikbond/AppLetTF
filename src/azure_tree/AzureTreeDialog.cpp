//----------------------------------------
#include <QPushButton>
#include <QMessageBox>
//----------------------------------------
#include "common.h"
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "AzureTreeDialog.h"
#include "ui_AzureTreeDialog.h"
//----------------------------------------

AzureTreeDialog::AzureTreeDialog( QWidget* parent ) : QDialog(parent), ui(new Ui::AzureTreeDialog)
{
    setupUI();
}
//----------------------------------------------------------------------------------------------------------

AzureTreeDialog::~AzureTreeDialog() {
    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение пути к выбранному каталогу
 * \return Путь к каталогу Azure
 */
QString AzureTreeDialog::selectedPath() const {

    QTreeWidgetItem* item = ui->azureTree->currentItem();
    if( item == nullptr ) {
        return "";
    }

    QString path = item->data(0, AzurePathRole).toString();
    return path;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Раскрытие ветки в дереве проектов
 * \param item Элемент, которые раскрывается
 */
void AzureTreeDialog::expand( QTreeWidgetItem* item ) {

    if( item->data(0, LoadedRole).isValid() ) {
        return;
    }

    QString path = item->data(0, AzurePathRole).toString();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.entriesDir( path );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::critical( this, tr("Ошибка"), tf.m_errText );
        return;
    }

    QList<AzureItem> entries = parseEntries( tf.m_response );
    createTreeItems( item, entries );

    item->setData( 0, LoadedRole, true );
    item->setChildIndicatorPolicy( QTreeWidgetItem::DontShowIndicatorWhenChildless );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание дочерних элементов
 * \param parent Указатель на родительский элемент
 * \param entries Список элементов, которые нужно создать
 */
void AzureTreeDialog::createTreeItems( QTreeWidgetItem* parent, const QList<AzureItem>& entries ) {

    QList<QTreeWidgetItem*> items;

    for( const AzureItem& entry : entries ) {

        QString path = entry.folder + "/" + entry.name;
        QTreeWidgetItem* newItem = new QTreeWidgetItem;
        newItem->setText( 0, entry.name );
        newItem->setIcon( 0, icon(entry.name, entry.type) );
        newItem->setData( 0, AzurePathRole  , path );

        if( entry.type == Folder ) {
            newItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
        }

        items.append( newItem );
        m_treeItems[path] = newItem;
    }

    if( parent == nullptr ) {
        ui->azureTree->addTopLevelItems( items );
    } else {
        parent->addChildren( items );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обновление состояния кнопки OK
 */
void AzureTreeDialog::updateStateOK() {

    bool isOk = ui->azureTree->currentItem() != nullptr;
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( isOk );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void AzureTreeDialog::setConfig( const Config& cfg ) {

    m_config = cfg;

    TFRequest tf;
    tf.setConfig( m_config );
    tf.entriesDir( "$/" );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        return;
    }

    QList<AzureItem> entries = parseEntries( tf.m_response );

    ui->azureTree->blockSignals( true );
    createTreeItems( nullptr, entries );
    ui->azureTree->blockSignals( false );

    updateStateOK();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка интерфейса
 */
void AzureTreeDialog::setupUI() {

    ui->setupUi( this );
    setWindowTitle( tr("Выбор каталога Azure DevOps Server") );

    ui->azureTree->header()->hide();
    ui->azureTree->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->azureTree->setExpandsOnDoubleClick( false );

    connect( ui->azureTree, &QTreeWidget::itemDoubleClicked   , this, &AzureTreeDialog::accept        );
    connect( ui->azureTree, &QTreeWidget::itemExpanded        , this, &AzureTreeDialog::expand        );
    connect( ui->azureTree, &QTreeWidget::itemSelectionChanged, this, &AzureTreeDialog::updateStateOK );
}
//----------------------------------------------------------------------------------------------------------
