//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QAbstractTextDocumentLayout>
//----------------------------------------
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
//----------------------------------------

/*!
 * \brief Перезагрузка дерева исключенных изменений
 */
void ChangesWidget::reactOnInclude() {

    int countIncluded = 0;

    for( int rootIdx = 0; rootIdx < ui->excludedTree->topLevelItemCount(); rootIdx++ ) {

        QTreeWidgetItem* folderItem = ui->excludedTree->topLevelItem( rootIdx );

        for( int idx = 0; idx < folderItem->childCount(); idx++ ) {
            QTreeWidgetItem* fileItem = folderItem->child( idx );
            if( fileItem->checkState(0) != Qt::Checked ) {
                continue;
            }

            QString path = fileItem->data(0, PathRole).toString();
            m_excluded.removeAll( path );
            countIncluded++;
        }
    }

    if( countIncluded == 0 ) {
        return;
    }

    reloadExcluded();
    reloadPrepared();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка изменения состояния элемента в дереве исключенных изменений
 * \param item Измененный элемент
 */
void ChangesWidget::reactOnExcludedItemChanged( QTreeWidgetItem* item, int ) {

    ui->excludedTree->blockSignals( true );

    QTreeWidgetItem* parentItem = item->parent();
    if( parentItem == nullptr ) {
        Qt::CheckState checkState = item->checkState(0);
        for( int idx = 0; idx < item->childCount(); idx++ ) {
            QTreeWidgetItem* childItem = item->child( idx );
            childItem->setCheckState( 0, checkState );
        }
    } else {
        Qt::CheckState checkState = item->checkState(0);
        for( int idx = 0; idx < parentItem->childCount(); idx++ ) {
            QTreeWidgetItem* childItem = parentItem->child( idx );
            if( checkState != childItem->checkState(0) ) {
                checkState = Qt::PartiallyChecked;
                break;
            }
        }
        parentItem->setCheckState( 0, checkState );
    }

    ui->excludedTree->blockSignals( false );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка дерева исключенных изменений
 */
void ChangesWidget::reloadExcluded() {

    ui->excludedTree->blockSignals( true );

    ui->excludedTree->clear();
    m_excludedDirItems.clear();

     foreach( const QString& path, m_excluded ) {

        QString folder;
        QString file;
        splitPath( path, folder, file );

        createExcludedFileItem( file, folder );
    }

    ui->excludedTree->blockSignals( false );
    ui->excludedTree->expandAll();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Добавление файла в дерево исключенных
 * \param path Путь к файлу
 */
void ChangesWidget::addExcludedFileItem( const QString& path ) {

    m_excluded.append( path );

    QString folder;
    QString file;
    splitPath( path, folder, file );

    createExcludedFileItem( file, folder );

    m_excludedDirItems[folder]->setExpanded( true );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание элемента в дереве исключенных изменений
 * \param file Имя файла
 * \param path Полный путь к элемента
 */
void ChangesWidget::createExcludedFileItem( const QString& file, const QString& path ) {

    QString dir = QString(path).remove(file);
    dir = QDir::toNativeSeparators(dir);
    if( dir.endsWith("/") || dir.endsWith("\\") ) {
        dir = dir.remove( dir.length() - 1, 1 );
    }

    QTreeWidgetItem* dirItem = m_excludedDirItems[dir];
    if( dirItem == nullptr ) {

        dirItem = new QTreeWidgetItem;
        dirItem->setIcon( 0, QPixmap(":/folder.png") );
        dirItem->setData( 0, Qt::DisplayRole   , dir           );
        dirItem->setData( 0, Qt::CheckStateRole, Qt::Unchecked );
        dirItem->setFirstColumnSpanned( true );

        ui->excludedTree->addTopLevelItem( dirItem );
        m_excludedDirItems[dir] = dirItem;
    }

    QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
    fileItem->setIcon( 0, icon(file, File)                  );
    fileItem->setData( 0, Qt::DisplayRole   , file          );
    fileItem->setData( 0, Qt::CheckStateRole, Qt::Unchecked );
    fileItem->setData( 0, PathRole          , QString("%1/%2").arg(path, file));
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
 * \brief Настройка части исключенных изменений
 */
void ChangesWidget::setupExcluded() {

    m_excludeCtxMenu = new QMenu( this );
    m_includeAction  = new QAction( QIcon(":/apply.png"), tr("Включить" ) );

    m_excludeCtxMenu->addAction( m_includeAction );

    ui->excludedTree->header()->hide();
    ui->excludedTree->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( m_includeAction , &QAction    ::triggered                 , this, &ChangesWidget::reactOnInclude              );
    connect( ui->excludedTree, &QTreeWidget::itemChanged               , this, &ChangesWidget::reactOnExcludedItemChanged  );
    connect( ui->excludedTree, &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnExcludeMenuRequested );
}
 //----------------------------------------------------------------------------------------------------------
