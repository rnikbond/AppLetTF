//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QMessageBox>
#include <QAbstractTextDocumentLayout>
//----------------------------------------
#include "methods.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
//----------------------------------------

/*!
 * \brief Перезагрузка дерева исключенных изменений
 */
void ChangesWidget::reactOnInclude() {

    QTreeWidgetItem* currentItem = ui->excludedTree->currentItem();
    if( currentItem == nullptr ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбран элемент") );
        return;
    }

    QStringList excludePathes;
    collectPathFiles( currentItem, LocalPathRole, excludePathes );
    if( excludePathes.isEmpty() ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбраны элементы включения") );
        return;
    }

    foreach( const QString& path, excludePathes ) {
        m_excluded.removeAll( path );
    }

    reloadExcluded();
    reloadPrepared();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка дерева исключенных изменений
 */
void ChangesWidget::reloadExcluded() {

    ui->excludedTree->blockSignals( true );

    ui->excludedTree->clear();
    m_excludedDirItems.clear();

    { // Создание структуры каталогов
        QStringList pathDirs = m_excluded;

        for( int idx = 0; idx < pathDirs.count(); idx++ ) {
            QString path = pathDirs[idx];
            path = path.mid( 0, path.lastIndexOf('/') );
            pathDirs[idx] = path;
        }

        pathDirs.sort();
        pathDirs.removeDuplicates();

        for( int idx = 0; idx < pathDirs.count(); idx++ ) {

            QString path = pathDirs[idx];

            QTreeWidgetItem* rootItem = new QTreeWidgetItem( ui->excludedTree );
            rootItem->setText( 0, path );
            rootItem->setData( 0, LocalPathRole, path );

            pathDirs.removeAt( idx );
            createTreeSubDirs( rootItem, LocalPathRole, pathDirs, idx );
            idx--;
        }
    }

    // Формирование списка: <путь> = QTreeWidgetItem
    QTreeWidgetItemIterator itemIt( ui->excludedTree, QTreeWidgetItemIterator::All );
    while( *itemIt ) {
        QTreeWidgetItem* item = *itemIt;
        item->setIcon( 0, QPixmap(":/folder.png") );
        item->setData( 0, TypeRole, TypeFolder );

        QString pathDir = item->data(0, LocalPathRole).toString();
        m_excludedDirItems[pathDir] = item;
        itemIt++;
    }

     foreach( const QString& path, m_excluded ) {

        QString dirPath ;
        QString fileName;
        splitPath( path, dirPath, fileName );

        QTreeWidgetItem* dirItem = m_excludedDirItems[dirPath];
        QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
        fileItem->setIcon( 0, icon(fileName, TypeFile)  );
        fileItem->setData( 0, Qt::DisplayRole, fileName );
        fileItem->setData( 0, LocalPathRole  , path     );
        fileItem->setData( 0, TypeRole       , TypeFile );
    }

    ui->excludedTree->blockSignals( false );
    ui->excludedTree->expandAll();

    updateExcludeActions();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Добавление файла в дерево исключенных
 * \param path Путь к файлу
 */
void ChangesWidget::addExcludedFileItem( const QString& path ) {

    m_excluded.append( path );
    reloadExcluded();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение контекстного меню в дереве исключенных изменений
 * \param pos Позиция для отображения
 */
void ChangesWidget::reactOnExcludeMenuRequested( const QPoint& pos ) {

    m_excludeCtxMenu->popup( ui->excludedTree->viewport()->mapToGlobal(pos) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обновление действий в дереве исключений
 */
void ChangesWidget::updateExcludeActions() {

    m_includeAction->setEnabled( false );

    QTreeWidgetItem* currentItem = ui->excludedTree->currentItem();
    if( currentItem == nullptr ) {
        return;
    }

    m_includeAction->setEnabled( true );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка части исключенных изменений
 */
void ChangesWidget::setupExcluded() {

    m_excludeCtxMenu = new QMenu( this );
    m_includeAction  = new QAction( QIcon(":/apply.png"), tr("Включить" ) );

    m_excludeCtxMenu->addAction( m_includeAction );

    ui->excludedTree->header()->hide();
    ui->excludedTree->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_includeAction , &QAction    ::triggered                 , this, &ChangesWidget::reactOnInclude              );
    connect( ui->excludedTree, &QTreeWidget::currentItemChanged        , this, &ChangesWidget::updateExcludeActions        );
    connect( ui->excludedTree, &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnExcludeMenuRequested );
}
 //----------------------------------------------------------------------------------------------------------
