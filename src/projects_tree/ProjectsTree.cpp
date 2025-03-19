//----------------------------------------
#include <QToolBar>
#include <QMessageBox>
#include <QTableWidget>
#include <QInputDialog>
#include <QTemporaryFile>
#include <QFileIconProvider>
//----------------------------------------
#include "common.h"
#include "methods.h"
#include "TFRequest.h"
#include "HistoryWidget.h"
//----------------------------------------
#include "ProjectsTree.h"
#include "ui_ProjectsTree.h"
//----------------------------------------

ProjectsTree::ProjectsTree(QWidget *parent) : QWidget(parent) , ui(new Ui::ProjectsTree)
{
    ui->setupUi(this);

    setupActions();
    setupCtxMenu();

    ui->historyTab->hide();
    ui->historyTab->setTabsClosable( true );

    ui->projectsTree->header()->hide();
    ui->projectsTree->setContextMenuPolicy( Qt::CustomContextMenu );

    ui->splitter->setSizes( {150, 500} );

    connect( ui->projectsTree, &QTreeWidget::itemDoubleClicked         , this, QOverload<QTreeWidgetItem*,int>::of(&ProjectsTree::expand)         );
    connect( ui->projectsTree, &QTreeWidget::itemExpanded              , this, QOverload<QTreeWidgetItem*    >::of(&ProjectsTree::expand)         );
    connect( ui->projectsTree, &QTreeWidget::customContextMenuRequested, this,                                     &ProjectsTree::showCtxMenu     );
    connect( ui->historyTab  , &QTabWidget::tabCloseRequested          , this,                                     &ProjectsTree::closeHistoryTab );
}
//----------------------------------------------------------------------------------------------------------

ProjectsTree::~ProjectsTree() {
    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение журнала изменений выбранного элемента
 */
void ProjectsTree::history() {

    QTreeWidgetItem* item = ui->projectsTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    QString path = item->data(0, TFSPathRole).toString() ;

    HistoryWidget* historyWidget = historyTab( path );
    historyWidget->reload( path );

    ui->historyTab->setCurrentWidget( historyWidget );
    ui->historyTab->show();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Клонирование последней версии выбранного элемента дерева
 */
void ProjectsTree::cloneLasted() {

    QTreeWidgetItem* item = ui->projectsTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    QString path = item->data(0, TFSPathRole).toString() ;

    TFRequest tf;
    tf.setConfig( m_config );
    tf.getDir( path );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Клонирование определенной версии выбранного элемента дерева
 */
void ProjectsTree::cloneCertain() {

    QTreeWidgetItem* item = ui->projectsTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    QString path = item->data(0, TFSPathRole).toString();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.history( path );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
        return;
    }

    QList<HistoryItem> historyItems = parseHistory( tf.m_response );
    QStringList changeSets;
    foreach( const HistoryItem& historyItem, historyItems) {
        changeSets.append( historyItem.version );
    }

    bool ok;
    QString version = QInputDialog::getItem( this,
                                             tr("Выберите набор изменений"),
                                             tr("Набор изменений:"), changeSets,
                                             1,
                                             false,
                                             &ok );

    if( !ok ) {
        return;
    }

    tf.getDir( path, version );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка дерева проектов
 *
 * Перед перезагрузкой запоминаются открытие узлы и после перезагрузки восстанавливаются.
 */
void ProjectsTree::reload() {

    QString     currentPath;
    QStringList expandPathes;

    { // Сохранение состояния дерева
        QTreeWidgetItem* currentItem = ui->projectsTree->currentItem();
        if( currentItem != nullptr ) {
            currentPath = currentItem->data(0, TFSPathRole).toString();
        }

        QTreeWidgetItemIterator itemIt( ui->projectsTree, QTreeWidgetItemIterator::All );
        while( *itemIt ) {
          QTreeWidgetItem* item = *itemIt;
          if( item->isExpanded() ) {
              expandPathes.append( item->data(0, TFSPathRole).toString() );
          }
          itemIt++;
        }
    }

    clear();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.entriesDir( "$/" );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::critical( this, tr("Ошибка"), tf.m_errText );
        return;
    }

    QList<AzureItem> entries = parseEntries( tf.m_response );

    ui->projectsTree->blockSignals( true );
    createTreeItems( nullptr, entries );
    ui->projectsTree->blockSignals( false );

    { // Восстановление развернутых узлоов
        QTreeWidgetItemIterator itemIt( ui->projectsTree, QTreeWidgetItemIterator::All );
        while( *itemIt ) {

          QTreeWidgetItem* item = *itemIt;

          QString path = item->data(0, TFSPathRole).toString();
          if( expandPathes.contains(path) ) {
              expandPathes.removeOne( path );
              item->setExpanded( true );
          }

          itemIt++;
        }

        if( !currentPath.isEmpty() ) {
            QTreeWidgetItem* item = m_treeItems[currentPath];
            if( item != nullptr ) {
                ui->projectsTree->setCurrentItem( item );
            }
        }
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Очистка дерева проектов
 */
void ProjectsTree::clear() {

    m_treeItems.clear();

    ui->historyTab->clear();

    ui->projectsTree->blockSignals( true );
    ui->projectsTree->clear();
    ui->projectsTree->blockSignals( false );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Раскрытие ветки в дереве проектов
 * \param item Элемент, которые раскрывается
 */
void ProjectsTree::expand( QTreeWidgetItem* item, int ) {

    expand( item );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Раскрытие ветки в дереве проектов
 * \param item Элемент, которые раскрывается
 */
void ProjectsTree::expand( QTreeWidgetItem* item ) {

    if( item->data(0, ItemTypeRole).toInt() != Folder ) {
        return;
    }

    if( item->data(0, LoadRole).isValid() ) {
        return;
    }

    QString path = item->data(0, TFSPathRole).toString();

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

    item->setData( 0, LoadRole, true );
    item->setChildIndicatorPolicy( QTreeWidgetItem::DontShowIndicatorWhenChildless );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Срздание элемента дерева
 * \param parent Указатель на родительский узел
 * \param entries Записи, которые нужно создать
 */
void ProjectsTree::createTreeItems( QTreeWidgetItem* parent, const QList<AzureItem>& entries ) {

    QList<QTreeWidgetItem*> items;

    for( const AzureItem& entry : entries ) {

        QString path = entry.folder + "/" + entry.name;
        QTreeWidgetItem* newItem = new QTreeWidgetItem;
        newItem->setText( 0, entry.name       );
        newItem->setIcon( 0, icon(entry.name, entry.type) );
        newItem->setData( 0, ItemTypeRole, entry.type );
        newItem->setData( 0, TFSPathRole , path );

        if( entry.type == Folder ) {
            newItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
        }

        items.append( newItem );
        m_treeItems[path] = newItem;
    }

    if( parent == nullptr ) {
        ui->projectsTree->addTopLevelItems( items );
    } else {
        parent->addChildren( items );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Установка фокуса на элемент дерева
 * \param path Путь к каталогу
 */
void ProjectsTree::setCurrentProject( const QString& path ) {

    QTreeWidgetItem* item = m_treeItems[path];
    if( item == nullptr ) {
        QMessageBox::warning( this,
                              tr("Ошибка"),
                              tr("Не удалось найти элемент:\n%1\n\nВозможно, этот узел не загружен в дереве.").arg(path),
                              QMessageBox::Close );
        return;
    }

    ui->projectsTree->setCurrentItem( item );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение вкладки журнала изменений
 * \param path Путь к элементу, информация о которого должна отобразиться
 * \return Указатель на вкладку
 *
 * Если вкладки по путь \a path ранее не было, она будет создана.
 */
HistoryWidget* ProjectsTree::historyTab( const QString& path ) {

    // Поиск уже созданной вкладки журнала
    for( int index = 0; index < ui->historyTab->count(); index++ ) {
        HistoryWidget* historyAt = qobject_cast<HistoryWidget*>(ui->historyTab->widget(index));
        if( historyAt->path() == path ) {
            return historyAt;
        }
    }

    // Создание новой вкладки журнала
    HistoryWidget* historyWidget = new HistoryWidget( ui->historyTab );
    historyWidget->setConfig( m_config );
    historyWidget->reload( path );
    connect( historyWidget, &HistoryWidget::commandExecuted, this, &ProjectsTree::commandExecuted   );
    connect( historyWidget, &HistoryWidget::projectSelected, this, &ProjectsTree::setCurrentProject );

    ui->historyTab->addTab( historyWidget, tr("Журнал: %1").arg(projectName(path)) );

    return historyWidget;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Закрытие вкладки журнала изменений
 * \param index Номер вкладки
 */
void ProjectsTree::closeHistoryTab( int index ) {

    QWidget* widet = ui->historyTab->widget( index );
    ui->historyTab->removeTab( index );
    delete widet;

    if( ui->historyTab->count() == 0 ) {
        ui->historyTab->hide();
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение контекстного меню
 * \param pos Позиция для отображения
 */
void ProjectsTree::showCtxMenu( const QPoint& pos ) {

    m_ctxMenu->popup( ui->projectsTree->viewport()->mapToGlobal(pos) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void ProjectsTree::setConfig( const Config& cfg ) {

    m_config = cfg;
    reload();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка действий
 */
void ProjectsTree::setupActions() {

    m_cloneLastedAction  = new QAction( QIcon(":/save.png"   ), tr("Получить последнюю версию"   ) );
    m_cloneСertainAction = new QAction( QIcon(":/save.png"   ), tr("Получить определенную версию") );
    m_historyAction      = new QAction( QIcon(":/file.png"   ), tr("Посмотреть журнал"           ) );
    m_reloadAction       = new QAction( QIcon(":/refresh.png"), tr("Перезагрузить дерево"        ) );

    connect( m_reloadAction      , &QAction::triggered, this, &ProjectsTree::reload       );
    connect( m_historyAction     , &QAction::triggered, this, &ProjectsTree::history      );
    connect( m_cloneLastedAction , &QAction::triggered, this, &ProjectsTree::cloneLasted  );
    connect( m_cloneСertainAction, &QAction::triggered, this, &ProjectsTree::cloneCertain );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка контекстного меню
 */
void ProjectsTree::setupCtxMenu() {

    m_ctxMenu = new QMenu( this );
    m_ctxMenu->addAction( m_cloneLastedAction  );
    m_ctxMenu->addAction( m_cloneСertainAction );
    m_ctxMenu->addSeparator();
    m_ctxMenu->addAction( m_historyAction );
    m_ctxMenu->addSeparator();
    m_ctxMenu->addAction( m_reloadAction );
}
//----------------------------------------------------------------------------------------------------------
