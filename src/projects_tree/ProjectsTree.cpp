//----------------------------------------
#include <QTime>
#include <QDebug>
#include <QToolBar>
#include <QFileDialog>
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
#include "HelpTreeDialog.h"
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
    connect( ui->projectsTree, &QTreeWidget::currentItemChanged        , this,                                     &ProjectsTree::updateActions   );
    connect( ui->projectsTree, &QTreeWidget::customContextMenuRequested, this,                                     &ProjectsTree::showCtxMenu     );
    connect( ui->historyTab  , &QTabWidget ::tabCloseRequested         , this,                                     &ProjectsTree::closeHistoryTab );

    updateActions();
}
//----------------------------------------------------------------------------------------------------------

ProjectsTree::~ProjectsTree() {
    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение списка действий
 * \return
 */
QList<QAction*> ProjectsTree::actions() const {

    return {
             m_cloneLastedAction ,
             m_historyAction     ,
             m_appendFileAction  ,
             m_deleteAction      ,
             m_reloadAction      ,
             m_helpAction        ,
           };
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

    QString path = item->data(0, AzurePathRole).toString() ;

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

    QString path = item->data(0, AzurePathRole).toString() ;

    TFRequest tf;
    tf.setConfig( m_config );
    tf.getDir( path );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
        return;
    }

    setClonedItem( item );

    if( tf.m_response.count() == 1 && tf.m_response.at(0).contains("All files up to date", Qt::CaseInsensitive) ) {
        QMessageBox::information( this, tr("Загрузка изменений"), tr("Новых изменений нет"), QMessageBox::Close );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Клонирование последней версии с принудительной перезаписью всех файлов
 */
void ProjectsTree::cloneRewrite() {

    QTreeWidgetItem* item = ui->projectsTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    QString path = item->data(0, AzurePathRole).toString() ;

    TFRequest tf;
    tf.setConfig( m_config );
    tf.getDir( path, QString(), true );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
        return;
    }

    setClonedItem( item );
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

    QString path = item->data(0, AzurePathRole).toString();

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
        return;
    }

    setClonedItem( item );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Выбор и добавление нового файла в проект
 */
void ProjectsTree::addNewFile() {

    QString path;

    QTreeWidgetItem* currentItem = ui->projectsTree->currentItem();
    if( currentItem != nullptr ) {
        path = currentItem->data(0, AzurePathRole).toString();
    }

    for( auto it = m_config.m_azure.workfoldes.keyValueBegin(); it != m_config.m_azure.workfoldes.keyValueEnd(); it++ ) {
        const QString& azurePath = it->first;
        const QString& localPath = it->second;

        if( path.contains(azurePath) ) {
            path = path.replace( azurePath, localPath );
            break;
        }
    }

    QString filePath = QFileDialog::getOpenFileName( this, tr("Добавление нового файла"), path );
    if( filePath.isEmpty() ) {
        return;
    }

    TFRequest tf;
    tf.setConfig( m_config );
    tf.add( {filePath} );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Удаление элемента (файл или каталог)
 */
void ProjectsTree::deleteItem() {

    QTreeWidgetItem* currentItem = ui->projectsTree->currentItem();
    if( currentItem == nullptr ) {
        return;
    }

    QString path = currentItem->data(0, AzurePathRole).toString();
    for( auto it = m_config.m_azure.workfoldes.keyValueBegin(); it != m_config.m_azure.workfoldes.keyValueEnd(); it++ ) {
        const QString& azurePath = it->first;
        const QString& localPath = it->second;

        if( path.contains(azurePath) ) {
            path = path.replace( azurePath, localPath );
            break;
        }
    }

    if( !QFile(path).exists() ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Удаляемый элемент должен быть склонирован на диск"), QMessageBox::Close );
        return;
    }

    int answer = QMessageBox::question( this, tr("Удаление"), tr("Удалить элемент: %1 ?").arg(path) );
    if( answer != QMessageBox::Yes ) {
        return;
    }

    TFRequest tf;
    tf.setConfig( m_config );
    tf.remove( {path} );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение справки по командам
 */
void ProjectsTree::help() {

    HelpTreeDialog dialog( qobject_cast<QWidget*>(parent()) );
    dialog.exec();
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
            currentPath = currentItem->data(0, AzurePathRole).toString();
        }

        QTreeWidgetItemIterator itemIt( ui->projectsTree, QTreeWidgetItemIterator::All );
        while( *itemIt ) {
          QTreeWidgetItem* item = *itemIt;
          if( item->isExpanded() ) {
              expandPathes.append( item->data(0, AzurePathRole).toString() );
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

          QString path = item->data(0, AzurePathRole).toString();
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

    updateActions();

    m_reloadAction->setText   ( tr("Обновить [%1]").arg(QTime::currentTime().toString("hh:mm"   )) );
    m_reloadAction->setToolTip( tr("Обновлено: %1").arg(QTime::currentTime().toString("hh:mm:ss")) );
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

    if( item->data(0, TypeRole).toInt() != TypeFolder ) {
        return;
    }

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
 * \brief Срздание элемента дерева
 * \param parent Указатель на родительский узел
 * \param entries Записи, которые нужно создать
 */
void ProjectsTree::createTreeItems( QTreeWidgetItem* parent, const QList<AzureItem>& entries ) {

    auto toLocalPath = [&]( const QString& azureItemPath ) {
        QString path;
        for( auto it = m_config.m_azure.workfoldes.keyValueBegin(); it != m_config.m_azure.workfoldes.keyValueEnd(); it++ ) {
            const QString& azurePath = it->first;
            const QString& localPath = it->second;

            if( azureItemPath.contains(azurePath) ) {
                path = QString(azureItemPath).replace( azurePath, localPath );
                break;
            }
        }
        return path;
    };

    QList<QTreeWidgetItem*> items;

    for( const AzureItem& entry : entries ) {

        QString azurePath = entry.folder + "/" + entry.name;
        QTreeWidgetItem* newItem = new QTreeWidgetItem;
        newItem->setText( 0, entry.name                   );
        newItem->setIcon( 0, icon(entry.name, entry.type) );
        newItem->setData( 0, TypeRole       , entry.type  );
        newItem->setData( 0, AzurePathRole  , azurePath   );

        QString localPath = toLocalPath( azurePath );
        QFileInfo fileInfo( localPath );
        if( !fileInfo.exists() ) {
            newItem->setForeground( 0, Qt::darkGray );
        }

        if( entry.type == TypeFolder ) {
            newItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
        }

        items.append( newItem );
        m_treeItems[azurePath] = newItem;
    }

    if( parent == nullptr ) {
        ui->projectsTree->addTopLevelItems( items );
    } else {
        parent->addChildren( items );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Установка признака склонированного элемента
 * \param item Указатель на элемент
 */
void ProjectsTree::setClonedItem( QTreeWidgetItem* item ) {

    item->setForeground( 0, QBrush() );
    for( int idx = 0; idx < item->childCount(); idx++ ) {
        QTreeWidgetItem* childItem = item->child( idx );
        setClonedItem( childItem );
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
    historyWidget->setCache ( m_cache  );
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
 * \brief Обновление состояния действий
 */
void ProjectsTree::updateActions() {

    m_cloneLastedAction ->setEnabled( false );
    m_cloneRewriteAction->setEnabled( false );
    m_cloneСertainAction->setEnabled( false );
    m_historyAction     ->setEnabled( false );
    m_appendFileAction  ->setEnabled( false );
    m_deleteAction      ->setEnabled( false );

    QTreeWidgetItem* item = ui->projectsTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    if( item->data(0, TypeRole).toInt() == TypeFolder ) {
        m_appendFileAction->setEnabled( true );
    }

    m_deleteAction      ->setEnabled( true );
    m_cloneLastedAction ->setEnabled( true );
    m_cloneRewriteAction->setEnabled( true );
    m_cloneСertainAction->setEnabled( true );
    m_historyAction     ->setEnabled( true );
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
 * \brief Инициализация объекта кэширования
 * \param cache Объект кэширования
 */
void ProjectsTree::setCache( ChangesetCache* cache ) {
    m_cache = cache;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка действий
 */
void ProjectsTree::setupActions() {

    m_cloneLastedAction  = new QAction( QIcon(":/save.png"    ), tr("Получить изменения") );
    m_cloneRewriteAction = new QAction( QIcon(":/rewrite.png" ), tr("Получить элемент"  ) );
    m_cloneСertainAction = new QAction( QIcon(":/save.png"    ), tr("Получить версию...") );
    m_historyAction      = new QAction( QIcon(":/list.png"    ), tr("Журнал"            ) );
    m_reloadAction       = new QAction( QIcon(":/reload.png"  ), tr("Обновить"          ) );
    m_appendFileAction   = new QAction( QIcon(":/new_file.png"), tr("Добавить файл"     ) );
    m_deleteAction       = new QAction( QIcon(":/delete.png"  ), tr("Удалить"           ) );
    m_helpAction         = new QAction( QIcon(":/info.png"    ), tr("Справка"           ) );

    m_cloneRewriteAction->setToolTip( tr("Как и обычное клонирование, но заново клонируются все файлы") );

    m_reloadAction->setShortcut( QKeySequence(Qt::Key_F5) );
    m_reloadAction->setToolTip( m_reloadAction->toolTip() + " " + m_reloadAction->shortcut().toString() );
    m_reloadAction->setShortcutVisibleInContextMenu( true );

    m_helpAction->setShortcut( QKeySequence(Qt::Key_F1) );
    m_helpAction->setToolTip( m_helpAction->toolTip() + " " + m_helpAction->shortcut().toString() );
    m_helpAction->setShortcutVisibleInContextMenu( true );

    connect( m_reloadAction      , &QAction::triggered, this, &ProjectsTree::reload       );
    connect( m_historyAction     , &QAction::triggered, this, &ProjectsTree::history      );
    connect( m_cloneLastedAction , &QAction::triggered, this, &ProjectsTree::cloneLasted  );
    connect( m_cloneRewriteAction, &QAction::triggered, this, &ProjectsTree::cloneRewrite );
    connect( m_cloneСertainAction, &QAction::triggered, this, &ProjectsTree::cloneCertain );
    connect( m_appendFileAction  , &QAction::triggered, this, &ProjectsTree::addNewFile   );
    connect( m_deleteAction      , &QAction::triggered, this, &ProjectsTree::deleteItem   );
    connect( m_helpAction        , &QAction::triggered, this, &ProjectsTree::help         );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка контекстного меню
 */
void ProjectsTree::setupCtxMenu() {

    m_ctxMenu = new QMenu( this );
    m_ctxMenu->addAction( m_cloneLastedAction  );
    m_ctxMenu->addAction( m_cloneRewriteAction );
    m_ctxMenu->addAction( m_cloneСertainAction );
    m_ctxMenu->addSeparator();
    m_ctxMenu->addAction( m_appendFileAction );
    m_ctxMenu->addAction( m_deleteAction     );
    m_ctxMenu->addSeparator();
    m_ctxMenu->addAction( m_historyAction );
    m_ctxMenu->addSeparator();
    m_ctxMenu->addAction( m_reloadAction );
}
//----------------------------------------------------------------------------------------------------------
