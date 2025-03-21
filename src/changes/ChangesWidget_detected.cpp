//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QMessageBox>
//----------------------------------------
#include "common.h"
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

    QStringList detectedChanges;

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

        for( int idx = idx_detected + 1; idx < tf.m_response.count(); idx++ ) {
            const QString& item = tf.m_response.at(idx);
            if( item.contains(dirLocal) ) {
                detectedChanges.append( tf.m_response[idx] );
            }
        }
    }

    ui->detectedTree->clear();
    m_detectedDirItems.clear();

    foreach( const QString& item, detectedChanges ) {

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
        createDetectedFileItem( parts[0], parts[1], status );
    }

    ui->detectedTree->expandAll();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание эдемента в дереве обнаруженных изменений
 * \param file Имя файла
 * \param path Полный путь к элемента
 * \param status Состояние элемента
 */
void ChangesWidget::createDetectedFileItem( const QString& file, const QString& path, int status ) {

    QString dir = QString(path).remove(file);
    dir = QDir::toNativeSeparators(dir);
    if( dir.endsWith("/") || dir.endsWith("\\") ) {
        dir = dir.remove( dir.length() - 1, 1 );
    }

    QTreeWidgetItem* dirItem = m_detectedDirItems[dir];
    if( dirItem == nullptr ) {

        dirItem = new QTreeWidgetItem;
        dirItem->setIcon( 0, QPixmap(":/folder.png") );
        dirItem->setData( 0, Qt::DisplayRole, dir );
        dirItem->setFirstColumnSpanned( true );

        ui->detectedTree->addTopLevelItem( dirItem );
        m_detectedDirItems[dir] = dirItem;
    }

    QString iconPath;

    switch( status ) {
        case StatusNew   : iconPath = ":/plus.png" ; break;
        case StatusEdit  : iconPath = ":/edit.png" ; break;
        case StatusDelete: iconPath = ":/minus.png"; break;
        default          : break;
    }

    QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
    fileItem->setIcon( 0, QIcon(iconPath)         );
    fileItem->setData( 0, Qt::DisplayRole, file   );
    fileItem->setData( 0, LocalPathRole  , path   );
    fileItem->setData( 0, StatusRole     , status );
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

    m_detectedCtxMenu->addAction(m_detectedApplyAction);

    connect( m_detectedApplyAction, &QAction    ::triggered                 , this, &ChangesWidget::reactOnDetectedApply         );
    connect( ui->detectedTree     , &QTreeWidget::currentItemChanged        , this, &ChangesWidget::updateDetectedActions        );
    connect( ui->detectedTree     , &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnDetectedMenuRequested );
}
//----------------------------------------------------------------------------------------------------------
