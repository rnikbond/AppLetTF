//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QDebug>
#include <QAction>
#include <QToolButton>
#include <QMessageBox>
#include <QAbstractTextDocumentLayout>
//----------------------------------------
#include "methods.h"
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

    QTreeWidgetItemIterator itemIt( ui->preparedTree, QTreeWidgetItemIterator::All );
    while( *itemIt ) {
        QTreeWidgetItem* item = *itemIt;
        if( item->childCount() == 0 ) {
            QString path = item->data(0, LocalPathRole).toString();
            changeFiles.append( path );
        }
        itemIt++;
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
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбран файл для сравнения") );
        return;
    }

    if( item->data(0, TypeRole).toInt() != TypeFile ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбран файл для сравнения") );
        return;
    }

    QString path = item->data(0, LocalPathRole).toString();

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

    QTreeWidgetItem* currentItem = ui->preparedTree->currentItem();
    if( currentItem == nullptr ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбран элемент") );
        return;
    }

    QStringList undoPathes;
    collectPathFiles( currentItem, LocalPathRole, undoPathes );
    if( undoPathes.isEmpty() ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбраны элементы для отмены изменений") );
        return;
    }

    QStringList undoNames;
    foreach( const QString& path,  undoPathes) {
        int idx = path.lastIndexOf('/');
        undoNames.append( path.mid(idx + 1, path.length() - idx) );
    }

    bool isOk = question( tr("Отмена изменений"), tr("Отменить изменения в этих файлах?"), undoNames.join('\n') );
    if( !isOk ) {
        return;
    }

    TFRequest tf;
    tf.setConfig( m_config );
    tf.cancelChanges( {undoPathes} );
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

    QTreeWidgetItem* currentItem = ui->preparedTree->currentItem();
    if( currentItem == nullptr ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбран элемент") );
        return;
    }

    QStringList excludePathes;
    collectPathFiles( currentItem, LocalPathRole, excludePathes );
    if( excludePathes.isEmpty() ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбраны элементы для отмены изменений") );
        return;
    }

    foreach( const QString& path, excludePathes ) {
        addExcludedFileItem( path );
    }

    reloadPrepared();
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

    QStringList pathFiles;
    QStringList preparedChanges;
    QMap<QString, int> statuses;

#ifdef WIN32
    QString detectedCaption = tr("Обнаруженные изменения:");
#else
    QString detectedCaption = tr("Detected Changes:");
#endif

    // Получение списка изменений по сопоставленным каталогам
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

    // Из полученных изменений отбираем только ожидающие.
    // В список файлов записываем только полный путь к элементу
    foreach( const QString& item, preparedChanges ) {

        int status = StatusNone;
        QStringList parts;

        foreach( const QString& caption, m_statusesTfsMap.keys()) {
            if( !item.contains(caption, Qt::CaseInsensitive) ) {
                continue;
            }
            status = m_statusesTfsMap[caption];
            parts = item.split(caption);
            break;
        }

        if( status == StatusNone ) {
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

        statuses[parts[1]] = status;
        pathFiles.append( parts[1] );
    }

    ui->preparedTree->blockSignals( true );

    ui->preparedTree->clear();
    m_preparedDirItems.clear();

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

            QTreeWidgetItem* rootItem = new QTreeWidgetItem( ui->preparedTree );
            rootItem->setText( 0, path );
            rootItem->setData( 0, LocalPathRole, path );

            pathDirs.removeAt( idx );
            createTreeSubDirs( rootItem, LocalPathRole, pathDirs, idx );
            idx--;
        }
    }

    // Формирование списка: <путь> = QTreeWidgetItem
    QTreeWidgetItemIterator itemIt( ui->preparedTree, QTreeWidgetItemIterator::All );
    while( *itemIt ) {
        QTreeWidgetItem* item = *itemIt;
        item->setIcon( 0, QPixmap(":/folder.png") );
        item->setData( 0, TypeRole, TypeFolder );

        QString pathDir = item->data(0, LocalPathRole).toString();
        m_preparedDirItems[pathDir] = item;
        itemIt++;
    }

    // Создание файлов в структуре каталогов
    foreach( const QString& filePath, pathFiles ) {

        QString dirPath ;
        QString fileName;
        splitPath( filePath, dirPath, fileName );

        int status = statuses[filePath];
        QString iconPath;
        switch( status ) {
            case StatusNew   : iconPath = ":/plus.png" ; break;
            case StatusEdit  : iconPath = ":/edit.png" ; break;
            case StatusDelete: iconPath = ":/minus.png"; break;
            default          : break;
        }

        QTreeWidgetItem* dirItem = m_preparedDirItems[dirPath];
        QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
        fileItem->setIcon( 0, joinIconsFile(fileName, iconPath) );
        fileItem->setData( 0, Qt::DisplayRole   , fileName      );
        fileItem->setData( 0, LocalPathRole     , filePath      );
        fileItem->setData( 0, StatusRole        , status        );
        fileItem->setData( 0, TypeRole          , TypeFile      );
    }

    ui->preparedTree->blockSignals( false );
    ui->preparedTree->expandAll();

    updatePreparedActions();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обновление действий в дереве ожидающий изменений
 */
void ChangesWidget::updatePreparedActions() {

    m_preparedDiffAction   ->setEnabled( false );
    m_preparedCancelAction ->setEnabled( false );
    m_excludeAction        ->setEnabled( false );

    QTreeWidgetItem* currentItem = ui->preparedTree->currentItem();
    if( currentItem == nullptr ) {
        return;
    }

    m_preparedDiffAction   ->setEnabled( currentItem->data(0, TypeRole).toInt() == TypeFile );
    m_preparedCancelAction ->setEnabled( true );
    m_excludeAction        ->setEnabled( true );

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

    m_preparedCtxMenu      = new QMenu( this );
    m_preparedDiffAction   = new QAction( QIcon(":/compare.png"), tr("Просмотр изменений" ) );
    m_preparedCancelAction = new QAction( QIcon(":/undo.png"   ), tr("Отменить изменения" ) );
    m_excludeAction        = new QAction( QIcon(":/exclude.png"), tr("Исключить") );

    m_preparedDiffAction  ->setToolTip( tr("Сравнить выбранный файл с последней версией") );
    m_preparedCancelAction->setToolTip( tr("Отменить изменения") );
    m_excludeAction       ->setToolTip( tr("Исключить") );

    ui->preparedTree->header()->hide();
    ui->preparedTree->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->preparedTree->setIconSize( QSize(40, 20) );

    m_preparedCtxMenu->addAction( m_preparedDiffAction );
    m_preparedCtxMenu->addSeparator();
    m_preparedCtxMenu->addAction( m_excludeAction        );
    m_preparedCtxMenu->addAction( m_preparedCancelAction );

    connect( m_preparedDiffAction  , &QAction    ::triggered                 , this, &ChangesWidget::reactOnPreparedDiff          );
    connect( m_preparedCancelAction, &QAction    ::triggered                 , this, &ChangesWidget::reactOnPreparedCancel        );
    connect( m_excludeAction       , &QAction    ::triggered                 , this, &ChangesWidget::reactOnPreparedExclude       );
    connect( ui->preparedTree      , &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnPreparedMenuRequested );
    connect( ui->preparedTree      , &QTreeWidget::currentItemChanged        , this, &ChangesWidget::updatePreparedActions        );
    connect( ui->commitButton      , &QPushButton::clicked                   , this, &ChangesWidget::reactOnCommit                );

    QAbstractTextDocumentLayout* docLayout = ui->commentEdit->document()->documentLayout();
    connect( docLayout, &QAbstractTextDocumentLayout::documentSizeChanged, this, &ChangesWidget::updateCommitSize );
}
//----------------------------------------------------------------------------------------------------------
