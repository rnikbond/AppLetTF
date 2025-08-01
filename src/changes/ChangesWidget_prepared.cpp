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

    QString comment = ui->commentEdit->toPlainText();
    if( comment.trimmed().isEmpty() || comment.trimmed().length() < 10 ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Комментарий не может быть пустым или коротким (< 10 символов)") );
        return;
    }

    QStringList changeFiles;

    bool isOnlyChecked = ui->partSelectCheck->isChecked();

    QTreeWidgetItemIterator itemIt( ui->preparedTree, QTreeWidgetItemIterator::All );
    while( *itemIt ) {
        QTreeWidgetItem* item = *itemIt;
        if( item->childCount() != 0 ) {
            itemIt++;
            continue;
        }

        if( isOnlyChecked && item->checkState(0) != Qt::Checked ) {
            itemIt++;
            continue;
        }

        QString path = item->data(0, LocalPathRole).toString();
        changeFiles.append( path );
        itemIt++;
    }

    if( changeFiles.isEmpty() ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Нет файлов для записи изменений") );
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

    ui->commentEdit->blockSignals( true );
    ui->commentEdit->clear();
    ui->commentEdit->blockSignals( false );

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
 * \brief Обработка установки признака частичного коммита
 */
void ChangesWidget::reactOnPartSelectCheck() {

    bool isPart = ui->partSelectCheck->isChecked();

    QTreeWidgetItemIterator itemIt( ui->preparedTree, QTreeWidgetItemIterator::All );
    while( *itemIt ) {
        QTreeWidgetItem* item = *itemIt;
        if( item->childCount() == 0 ) {
            QVariant state = isPart ? Qt::Unchecked : QVariant();
            item->setData( 0, Qt::CheckStateRole, state );
        }
        itemIt++;
    }
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

    QMap<QString, TFRequest*> responses;
    foreach( const QString& dirLocal, m_config.m_azure.workfoldes ) {

        TFRequest* tf = new TFRequest( this );
        tf->setConfig( m_config );
        tf->status( dirLocal );
        emit commandExecuted( tf->m_isErr, tf->m_errCode, tf->m_errText, tf->m_response );

        if( tf->m_isErr ) {
            delete tf;
            continue;
        }

        responses[dirLocal] = tf;
    }

    reloadPrepared( responses );

    foreach( TFRequest* tf, responses ) {
        delete tf;
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка дерева подготовленных (ожидающих) изменений
 * \param responses Список ответов за запросы по сопоставленным каталогам
 */
void ChangesWidget::reloadPrepared( const QMap<QString, TFRequest*>& responses ) {

    QStringList pathFiles;
    QList<StatusItem> items;

    // Получение списка изменений по сопоставленным каталогам
    for( QMap<QString, TFRequest*>::const_iterator it = responses.constBegin(); it != responses.constEnd(); ++it ) {

        const QString& dirLocal = it.key();
        TFRequest* tf = it.value();

        QList<StatusItem> itemsWorkfold = parseStatusPrepared(tf->m_response, dirLocal);

        for( int idx = 0; idx < itemsWorkfold.count(); idx++ ) {
            const StatusItem& item = itemsWorkfold.at( idx );
            if( m_excluded.contains(item.path) ) {
                itemsWorkfold.removeAt(idx);
                idx--;
                continue;
            }

            pathFiles.append( item.path );
        }

        if( !itemsWorkfold.isEmpty() ) {
            items.append( itemsWorkfold );
        }
    }

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

        QTreeWidgetItem* dirItem = m_preparedDirItems[dirPath];
        QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
        fileItem->setIcon( 0, joinIconsFile(fileName, iconPath)  );
        fileItem->setData( 0, Qt::DisplayRole   , fileName       );
        fileItem->setData( 0, LocalPathRole     , item.path      );
        fileItem->setData( 0, StatusRole        , item.status    );
        fileItem->setData( 0, TypeRole          , TypeFile       );
        fileItem->setData( 0, Qt::CheckStateRole, Qt::Unchecked  );
    }

    ui->preparedTree->expandAll();
    reactOnPartSelectCheck();
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
    ui->partSelectCheck   ->setToolTip( tr("Позволяет коммитить только выбранный файлы") );

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
    connect( ui->partSelectCheck   , &QCheckBox  ::clicked                   , this, &ChangesWidget::reactOnPartSelectCheck       );
    connect( ui->preparedTree      , &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnPreparedMenuRequested );
    connect( ui->preparedTree      , &QTreeWidget::currentItemChanged        , this, &ChangesWidget::updatePreparedActions        );
    connect( ui->commitButton      , &QPushButton::clicked                   , this, &ChangesWidget::reactOnCommit                );

    QAbstractTextDocumentLayout* docLayout = ui->commentEdit->document()->documentLayout();
    connect( docLayout, &QAbstractTextDocumentLayout::documentSizeChanged, this, &ChangesWidget::updateCommitSize );
}
//----------------------------------------------------------------------------------------------------------
