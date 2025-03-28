//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QDebug>
#include <QAction>
#include <QToolButton>
#include <QMessageBox>
//----------------------------------------
#include "common.h"
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
//----------------------------------------

/*!
 * \brief Обработка применения обнаруженных изменений
 */
void ChangesWidget::reactOnDetectedApply() {

    QTreeWidgetItem* item = ui->detectedTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    QString path = item->data(0, LocalPathRole).toString();
    int status   = item->data(0, StatusRole   ).toInt();

    TFRequest tf;
    tf.setConfig( m_config );
    switch( status ) {
        case StatusNew   : tf.add   ({path}); break;
        case StatusDelete: tf.remove({path}); break;
        default          : return;
    }

    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );
    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText );
        return;
    }

   reload();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение контекстного меню в дереве обнаруженных изменений
 * \param pos Позиция для отображения
 */
void ChangesWidget::reactOnDetectedMenuRequested( const QPoint& pos ) {

    m_detectedCtxMenu->popup( ui->detectedTree->viewport()->mapToGlobal(pos) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка дерева обнаруженных изменений
 */
void ChangesWidget::reloadDetected() {

    QStringList pathFiles;
    QList<StatusItem> items;

    foreach( const QString& dirLocal, m_config.m_azure.workfoldes ) {

        TFRequest tf;
        tf.setConfig( m_config );
        tf.status( dirLocal );
        emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

        if( tf.m_isErr ) {
            continue;
        }

        QList<StatusItem> itemsWorkfold = parseStatusDetected( tf.m_response, dirLocal );
        for( int idx = 0; idx < itemsWorkfold.count(); idx++ ) {
            const StatusItem& item = itemsWorkfold.at( idx );
            pathFiles.append( item.path );
        }

        items.append( itemsWorkfold );
    }

    ui->detectedTree->clear();
    m_detectedDirItems.clear();

    { // Создание структуры каталогов
        QStringList pathDirs = pathFiles;

        for( int idx = 0; idx < pathDirs.count(); idx++ ) {
            QString path = pathDirs[idx];
            path = path.mid( 0, path.lastIndexOf('/') );
            pathDirs[idx] = path;
        }

        pathDirs.sort();
        pathDirs.removeDuplicates();

        for( int idx = 0; idx < pathDirs.count(); idx++ ) {

            QString path = pathDirs[idx];

            QTreeWidgetItem* rootItem = new QTreeWidgetItem( ui->detectedTree );
            rootItem->setText( 0, path );
            rootItem->setData( 0, LocalPathRole, path );

            pathDirs.removeAt( idx );
            createTreeSubDirs( rootItem, LocalPathRole, pathDirs, idx );
            idx--;
        }
    }

    // Формирование списка: <путь> = QTreeWidgetItem
    QTreeWidgetItemIterator itemIt( ui->detectedTree, QTreeWidgetItemIterator::All );
    while( *itemIt ) {
        QTreeWidgetItem* item = *itemIt;
        item->setIcon( 0, QPixmap(":/folder.png") );
        item->setData( 0, TypeRole, TypeFolder );

        QString pathDir = item->data(0, LocalPathRole).toString();
        m_detectedDirItems[pathDir] = item;
        itemIt++;
    }

    // Создание файлов в структуре каталогов
    foreach( const StatusItem& item, items ) {

        QString dirPath ;
        QString fileName;
        splitPath( item.path, dirPath, fileName );

        QString iconPath;
        switch( item.status ) {
        case StatusNew   : iconPath = ":/plus.png" ; break;
        case StatusEdit  : iconPath = ":/edit.png" ; break;
        case StatusDelete: iconPath = ":/minus.png"; break;
        default          : break;
        }

        QTreeWidgetItem* dirItem = m_detectedDirItems[dirPath];
        QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
        fileItem->setIcon( 0, joinIconsFile(fileName, iconPath)  );
        fileItem->setData( 0, Qt::DisplayRole   , fileName       );
        fileItem->setData( 0, LocalPathRole     , item.path      );
        fileItem->setData( 0, StatusRole        , item.status    );
        fileItem->setData( 0, TypeRole          , TypeFile       );
    }

    ui->detectedTree->expandAll();
    updateDetectedActions();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обновление действий в дереве обнаруженных изменений
 */
void ChangesWidget::updateDetectedActions() {

    m_detectedApplyAction->setEnabled( false );

    QTreeWidgetItem* currentItem = ui->detectedTree->currentItem();
    if( currentItem == nullptr ) {
        return;
    }

    m_detectedApplyAction->setEnabled( true );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка части обнаруженных изменений
 */
void ChangesWidget::setupDetected() {

    m_detectedCtxMenu      = new QMenu( this );
    m_detectedApplyAction  = new QAction( QIcon(":/apply.png"), tr("Применить") );

    m_detectedApplyAction->setToolTip( tr("Применить изменения") );

    ui->detectedTree->header()->hide();
    ui->detectedTree->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->detectedTree->setIconSize( QSize(40, 20) );

    m_detectedCtxMenu->addAction(m_detectedApplyAction);

    connect( m_detectedApplyAction, &QAction    ::triggered                 , this, &ChangesWidget::reactOnDetectedApply         );
    connect( ui->detectedTree     , &QTreeWidget::currentItemChanged        , this, &ChangesWidget::updateDetectedActions        );
    connect( ui->detectedTree     , &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnDetectedMenuRequested );
}
//----------------------------------------------------------------------------------------------------------
