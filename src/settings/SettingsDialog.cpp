//----------------------------------------
#include <QDebug>
//----------------------------------------
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
//----------------------------------------

enum Pages {
    PageCommon, ///< Страница: Общее
    PageAzure , ///< Страница: Azure DevOps Server
};
//----------------------------------------
enum {
    PageRole = Qt::UserRole + 1,
};
//----------------------------------------

SettingsDialog::SettingsDialog( QWidget* parent ) : QDialog(parent), ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->splitter->setSizes( {150, 500} );
    setWindowTitle( tr("Настройка") );

    initPages();

    connect( ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::save );
}
//----------------------------------------------------------------------------------------------------------

SettingsDialog::~SettingsDialog() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сохранение настроек
 */
void SettingsDialog::save() {

    saveConfigPageCommon();
    saveConfigPageAzure ();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Восстановление настроек
 */
void SettingsDialog::restore() {

    restoreConfigPageCommon();
    restoreConfigPageAzure ();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void SettingsDialog::setConfig( const Config& cfg ) {

    m_config = cfg;
    restore();

    ui->pagesList->setCurrentItem( findPage(PageAzure) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение конфигурации
 * \return Настроенная конфигурация
 */
const Config& SettingsDialog::config() const {
    return m_config;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Поиск страницы
 * \param page Номер страницы
 * \return Указатель на элемент списка, соответствующий странице
 */
QListWidgetItem* SettingsDialog::findPage( int page ) const {

    for( int row = 0; row < ui->pagesList->count(); row++ ) {
        QListWidgetItem* itemPage = ui->pagesList->item( row );
        if( itemPage->data(PageRole).toInt() == page ) {
            return itemPage;
        }
    }

    return nullptr;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Выбор страницы
 * \param itemPage Указатель на элемент списка, соответствующий странице
 */
void SettingsDialog::selectPage( QListWidgetItem* itemPage, QListWidgetItem* ) {

    if( itemPage == nullptr ) {
        return;
    }

    int page = itemPage->data(PageRole).toInt();
    switch( page) {
        case PageCommon: {
        ui->pagesStack->setCurrentWidget( ui->commonPage );
            break;
        }
        case PageAzure: {
            ui->pagesStack->setCurrentWidget( ui->azurePage );
            break;
        }
        default: {
            qDebug() << "SettingsDialog::selectPage()" << QString("unknown page role: %1").arg(page);
            break;
        }
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Инициализация страниц
 */
void SettingsDialog::initPages() {

    QListWidgetItem* itemCommon = new QListWidgetItem;
    QListWidgetItem* itemAzure = new QListWidgetItem;

    itemCommon->setText( tr("Общее")               );
    itemAzure ->setText( tr("Azure DevOps Server") );

    itemCommon->setData( PageRole, PageCommon );
    itemAzure ->setData( PageRole, PageAzure  );

    ui->pagesList->addItem( itemCommon );
    ui->pagesList->addItem( itemAzure  );

    initPageCommon();
    initPageAzure();

    connect( ui->pagesList, &QListWidget::currentItemChanged, this, &SettingsDialog::selectPage );
}
//----------------------------------------------------------------------------------------------------------


