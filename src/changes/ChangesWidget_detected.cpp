//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QMessageBox>
//----------------------------------------
#include "TFRequest.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
//----------------------------------------

void ChangesWidget::reactOnDetectedApply() {

    QTreeWidgetItem* item = ui->detectedTree->currentItem();
    if( item == nullptr ) {
        return;
    }

    QString path = item->data(0, PathRole).toString();
    int status   = item->data(0, StatusRole).toInt();

    TFRequest tf;
    tf.setConfig( m_config );
    switch( status ) {
        case CreateStatus: tf.add   ({path}); break;
        case DeleteStatus: tf.remove({path}); break;
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

void ChangesWidget::reactOnDetectedMenuRequested( const QPoint& pos ) {

    m_detectedCtxMenu->popup( ui->detectedTree->viewport()->mapToGlobal(pos) );
}
//----------------------------------------------------------------------------------------------------------

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
        createDetectedFileItem( parts[0], parts[1], status );
    }

    ui->detectedTree->expandAll();
}
//----------------------------------------------------------------------------------------------------------

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
        case CreateStatus: iconPath = ":/plus.png" ; break;
        case ChangeStatus: iconPath = ":/edit.png" ; break;
        case DeleteStatus: iconPath = ":/minus.png"; break;
        default          : break;
    }

    QTreeWidgetItem* fileItem = new QTreeWidgetItem( dirItem );
    fileItem->setIcon( 0, QIcon(iconPath)         );
    fileItem->setData( 0, Qt::DisplayRole, file   );
    fileItem->setData( 0, PathRole       , path   );
    fileItem->setData( 0, StatusRole     , status );
}
//----------------------------------------------------------------------------------------------------------

void ChangesWidget::setupDetected() {

    m_reloadDetectedButton = new QToolButton( this );
    m_detectedCtxMenu      = new QMenu( this );
    m_detectedApplyAction  = new QAction( tr("Применить изменения") );

    m_reloadDetectedButton->setToolTip( tr("Обновить") );
    m_reloadDetectedButton->setIcon( QIcon(":/refresh.png") );

    ui->detectedTab->setCornerWidget( m_reloadDetectedButton, Qt::TopLeftCorner );

    ui->detectedTree->header()->hide();
    ui->detectedTree->setContextMenuPolicy( Qt::CustomContextMenu );

    m_detectedCtxMenu->addAction(m_detectedApplyAction);

    connect( m_detectedApplyAction , &QAction    ::triggered                 , this, &ChangesWidget::reactOnDetectedApply         );
    connect( ui->detectedTree      , &QTreeWidget::customContextMenuRequested, this, &ChangesWidget::reactOnDetectedMenuRequested );
    connect( m_reloadDetectedButton, &QToolButton::clicked                   , this, &ChangesWidget::reloadDetected               );
}
//----------------------------------------------------------------------------------------------------------
