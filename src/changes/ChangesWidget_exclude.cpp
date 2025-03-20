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
    fileItem->setData( 0, PathRole          , path          );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка части исключенных изменений
 */
void ChangesWidget::setupExcluded() {

    ui->excludedTree->header()->hide();
    ui->excludedTree->setContextMenuPolicy( Qt::CustomContextMenu );
}
 //----------------------------------------------------------------------------------------------------------
