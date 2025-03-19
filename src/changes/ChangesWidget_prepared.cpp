//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QMessageBox>
#include <QAbstractTextDocumentLayout>
//----------------------------------------
#include "TFRequest.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
//----------------------------------------

/*!
 * \brief Фиксация изменений
 */
void ChangesWidget::reactOnCommit() {

    QString     comment = ui->commentEdit->toPlainText();
    QStringList changeFiles;

    for( int dir_idx = 0; dir_idx < ui->preparedTree->topLevelItemCount(); dir_idx++ ) {

        QTreeWidgetItem* dirItem = ui->preparedTree->topLevelItem( dir_idx );
        for( int file_idx = 0; file_idx < dirItem->childCount(); file_idx++ ) {

            QTreeWidgetItem* fileItem = dirItem->child( file_idx );
            if( fileItem->checkState(0) == Qt::Unchecked ) {
                continue;
            }

            QString path = fileItem->data(0, PathRole).toString();
            changeFiles.append( path );
        }
    }

    if( changeFiles.isEmpty() ) {
        return;
    }

    TFRequest tf;
    tf.setConfig( m_config );
    tf.commit( comment, changeFiles );

    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );
    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText );
        return;
    }

    reload();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сравнение подготовленных (ожидающих) изменений
 */
void ChangesWidget::reactOnPreparedDiff() {

    QTreeWidgetItem* item = ui->preparedTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    QString path = item->data(0, PathRole).toString();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.difference( path );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText );
        return;
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отмена подготовленных (ожидающих) изменений
 */
void ChangesWidget::reactOnPreparedCancel() {

    QStringList undoPathes;

    QList<QTreeWidgetItem*> items = ui->preparedTree->selectedItems();
    foreach( QTreeWidgetItem* item, items ) {

        if( item->parent() == nullptr ) {
            for( int idx = 0; idx < item->childCount(); idx++ ) {
                QTreeWidgetItem* childItem = item->child( idx );
                QString path = childItem->data(0, PathRole).toString();
                undoPathes.append( path );
            }
        } else {
            QString path = item->data(0, PathRole).toString();
            undoPathes.append( path );
        }
    }

    if( undoPathes.isEmpty() ) {
        return;
    }


    TFRequest tf;
    tf.setConfig( m_config );
    tf.cancelChanges( undoPathes );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText );
        return;
    }

   reload();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Исключение подготовленных (ожидающих) изменений
 */
void ChangesWidget::reactOnPreparedExclude() {

    QStringList             pathes;
    QList<QTreeWidgetItem*> removeItems;

    QList<QTreeWidgetItem*> items = ui->preparedTree->selectedItems();
    foreach( QTreeWidgetItem* item, items ) {

        if( item->parent() == nullptr ) {
            for( int idx = 0; idx < item->childCount(); idx++ ) {
                QTreeWidgetItem* childItem = item->child( idx );
                QString path = childItem->data(0, PathRole).toString();
                pathes.append( path );
            }
        } else {
            QString path = item->data(0, PathRole).toString();
            pathes.append( path );
        }

        removeItems.append( item );
    }

    foreach (const QString& path, pathes) {
        addExcludedFileItem( path );
    }

    foreach( QTreeWidgetItem* item, removeItems ) {
        delete item;
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка изменения состояния элемента в дереве подготовленных изменений
 * \param item Измененный элемент
 */
void ChangesWidget::reactOnPreparedItemChanged( QTreeWidgetItem* item, int ) {

    ui->preparedTree->blockSignals( true );

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

    ui->preparedTree->blockSignals( false );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение контекстного меню в дереве подготовленных (ожидающих) изменений
 * \param pos Позиция для отображения
 */
void ChangesWidget::reactOnPreparedMenuRequested( const QPoint& pos ) {

    m_preparedCtxMenu->popup( ui->preparedTree->viewport()->mapToGlobal(pos) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка дерева подготовленных (ожидающих) изменений
 */
void ChangesWidget::reloadPrepared() {

    ui->commentEdit->clear();

    QStringList preparedChanges;

#ifdef WIN32
    QString detectedCaption = tr("Обнаруженные изменения:");
#else
    QString detectedCaption = tr("Detected Changes:");
#endif

    foreach( const QString& dirLocal, m_config.m_azure.workfoldes ) {

        TFRequest tf;
        tf.setConfig( m_config );
        tf.status( dirLocal );
        emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

        if( tf.m_isErr ) {
            continue;
        }

        int idx_detected = tf.m_response.count();
        for( int idx = 0; idx < tf.m_response.count(); idx++ ) {
            const QString& item = tf.m_response.at(idx);
            if (item.contains(detectedCaption)) {
                idx_detected = idx;
                break;
            }
        }

        for( int idx = 0; idx < idx_detected; idx++ ) {
            const QString& item = tf.m_response.at(idx);
            if( item.contains(dirLocal) ) {
                preparedChanges.append( tf.m_response[idx] );
            }
        }
    }

    ui->preparedTree->blockSignals( true );

    ui->preparedTree->clear();
    m_preparedDirItems.clear();

    foreach( const QString& item, preparedChanges ) {

        int status = UnknownStatus;
        QStringList parts;

        foreach( const QString& caption, m_statusesTfsMap.keys()) {
            if( !item.contains(caption, Qt::CaseInsensitive) ) {
                continue;
            }

            status = m_statusesTfsMap[caption];
            parts = item.split(caption);
            break;
        }

        if( status == UnknownStatus ) {
            continue;
        }

        if( parts.count() != 2 ) {
            continue;
        }

        parts[0] = parts[0].trimmed();
        parts[1] = parts[1].trimmed();
        parts[0] = QDir::toNativeSeparators(parts[0]);

        if( m_excluded.contains(parts[1]) ) {
            continue;
        }

        createPreparedFileItem( parts[0], parts[1], status );
    }

    ui->preparedTree->blockSignals( false );
    ui->preparedTree->expandAll();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание элемента в дереве подготовленных (ожидающих) изменений
 * \param file Имя файла
 * \param path Полный путь к элемента
 * \param status Состояние элемента
 */
void ChangesWidget::createPreparedFileItem( const QString& file, const QString& path, int status ) {

    QString dir = QString(path).remove(file);
    dir = QDir::toNativeSeparators(dir);
    if( dir.endsWith("/") || dir.endsWith("\\") ) {
        dir = dir.remove( dir.length() - 1, 1 );
    }

    QTreeWidgetItem* dirItem = m_preparedDirItems[dir];
    if( dirItem == nullptr ) {

        dirItem = new QTreeWidgetItem;
        dirItem->setIcon( 0, QPixmap(":/folder.png") );
        dirItem->setData( 0, Qt::DisplayRole   , dir           );
        dirItem->setData( 0, Qt::CheckStateRole, Qt::Unchecked );
        dirItem->setFirstColumnSpanned( true );

        ui->preparedTree->addTopLevelItem( dirItem );
        m_preparedDirItems[dir] = dirItem;
    }

    QString iconPath;

    switch( status ) {
        case CreateStatus: iconPath = ":/plus.png" ; break;
        case ChangeStatus: iconPath = ":/edit.png" ; break;
        case DeleteStatus: iconPath = ":/minus.png"; break;
        default          : break;
    }

    QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
    fileItem->setIcon( 0, QIcon(iconPath) );
    fileItem->setData( 0, Qt::DisplayRole   , file          );
    fileItem->setData( 0, Qt::CheckStateRole, Qt::Unchecked );
    fileItem->setData( 0, PathRole          , path          );
    fileItem->setData( 0, StatusRole        , status        );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обновление размера поля для ввода комментария
 */
void ChangesWidget::updateCommitSize() {

   auto textHeight = ui->commentEdit->document()->documentLayout()->documentSize().height();
   ui->commentEdit->setFixedHeight(textHeight + ui->commentEdit->height() - ui->commentEdit->viewport()->height());
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка части подготовленных (ожидающих) изменений
 */
void ChangesWidget::setupPrepared() {

    m_reloadPreparedButton = new QToolButton( this );
    m_preparedCtxMenu      = new QMenu( this );
    m_preparedDiffAction   = new QAction( QIcon(":/compare_files.png"), tr("Сравнить с последней версией") );
    m_preparedCancelAction = new QAction( QIcon(":/undo.png"         ), tr("Отменить изменения") );
    m_excludeAction        = new QAction( tr("Исключить") );

    m_reloadPreparedButton->setToolTip( tr("Обновить") );
    m_reloadPreparedButton->setIcon( QIcon(":/refresh.png") );

    ui->preparedTab->setCornerWidget( m_reloadPreparedButton, Qt::TopLeftCorner );

    ui->preparedTree->header()->hide();
    ui->preparedTree->setContextMenuPolicy( Qt::CustomContextMenu );

    m_preparedCtxMenu->addAction(m_preparedDiffAction);
    m_preparedCtxMenu->addSeparator();
    m_preparedCtxMenu->addAction(m_preparedCancelAction);
    m_preparedCtxMenu->addSeparator();
    m_preparedCtxMenu->addAction( m_excludeAction );

    connect( m_preparedDiffAction  , &QAction    ::triggered                 , this, &ChangesWidget::reactOnPreparedDiff          );
    connect( m_preparedCancelAction, &QAction    ::triggered                 , this, &ChangesWidget::reactOnPreparedCancel        );
    connect( m_excludeAction       , &QAction    ::triggered                 , this, &ChangesWidget::reactOnPreparedExclude       );
    connect( ui->preparedTree      , &QTreeWidget::itemChanged               , this, &ChangesWidget::reactOnPreparedItemChanged   );
    connect( ui->preparedTree      , &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnPreparedMenuRequested );
    connect( ui->commitButton      , &QPushButton::clicked                   , this, &ChangesWidget::reactOnCommit                );
    connect( m_reloadPreparedButton, &QToolButton::clicked                   , this, &ChangesWidget::reloadPrepared               );

    QAbstractTextDocumentLayout* docLayout = ui->commentEdit->document()->documentLayout();
    connect( docLayout, &QAbstractTextDocumentLayout::documentSizeChanged, this, &ChangesWidget::updateCommitSize );
}
//----------------------------------------------------------------------------------------------------------
