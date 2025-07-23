//----------------------------------------
#include <QDir>
#include <QDebug>
#include <QMessageBox>
//----------------------------------------
#include "common.h"
#include "methods.h"
#include "TFRequest.h"
#include "NewWorkspaceDialog.h"
//----------------------------------------
#include "WorkspacesDialog.h"
#include "ui_WorkspacesDialog.h"
//----------------------------------------

WorkspacesDialog::WorkspacesDialog( QWidget* parent ) : QDialog( parent ), ui( new Ui::WorkspacesDialog )
{
    setupUI();
}
//----------------------------------------------------------------------------------------------------------

WorkspacesDialog::~WorkspacesDialog()
{
    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение выбранной рабочей области
 * \return Выбранная рабочая область
 */
Workspace WorkspacesDialog::selectedWorkspace() const {

    Workspace workspace;

    int row = ui->workspacesTable->currentRow();
    if( row == -1 ) {
        return workspace;
    }

    QTableWidgetItem* nameItem    = ui->workspacesTable->item( row, ColumnName    );
    QTableWidgetItem* pcItem      = ui->workspacesTable->item( row, ColumnPC      );
    QTableWidgetItem* ownerItem   = ui->workspacesTable->item( row, ColumnOwner   );
    QTableWidgetItem* commentItem = ui->workspacesTable->item( row, ColumnComment );
    QTableWidgetItem* mapItem     = ui->workspacesTable->item( row, ColumnMap     );

    workspace.name     = nameItem   ->text();
    workspace.owner    = pcItem     ->text();
    workspace.computer = ownerItem  ->text();
    workspace.comment  = commentItem->text();

    QStringList pathes = mapItem->text().split("\n", Qt::SkipEmptyParts);
    foreach( const QString& path, pathes ) {

        QStringList parts = path.split( " -> " );
        WorkfoldItem workfold;
        workfold.pathServer = parts[0].trimmed();
        workfold.pathLocal  = parts[1].trimmed();

        workspace.workfolds.append( workfold );
    }

    return workspace;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обновление состояния кнопки OK
 */
void WorkspacesDialog::updateStateOK() {

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( false );

    int row = ui->workspacesTable->currentRow();
    if( row == -1 ) {
        return;
    }

    QTableWidgetItem* mapItem = ui->workspacesTable->item( row, ColumnMap );

    QStringList localPathes = mapItem->data(Qt::UserRole).toStringList();
    if( localPathes.isEmpty() ) {
        return;
    }

    foreach( const QString& path, localPathes ) {
        if( !QDir(path).exists() ) {
            return;
        }
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( true );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание рабочего пространства
 */
void WorkspacesDialog::createWorkspace() {

    NewWorkspaceDialog dialog;
    connect( &dialog, &NewWorkspaceDialog::commandExecuted, this, &WorkspacesDialog::commandExecuted );

    dialog.setConfig( m_config );
    if( dialog.exec() != QDialog::Accepted ) {
        return;
    }

    QString name      = dialog.name();
    QString azurePath = dialog.azureFolderPath();
    QString localPath = dialog.localFolderPath();
    QString comment   = dialog.comment();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.createWorkspace( name, comment );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
        return;
    }

    tf.mapWorkfold( azurePath, localPath, name );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        tf.removeWorkspace( name );
        emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
        return;
    }

    reload();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Удаление рабочего пространства
 */
void WorkspacesDialog::removeWorkspace() {

    int row = ui->workspacesTable->currentRow();
    if( row == -1 ) {
        QMessageBox::warning( this, tr("Ошибка"), tr("Не выбрана рабочая область"), QMessageBox::Close );
        return;
    }

    QTableWidgetItem* nameItem = ui->workspacesTable->item( row, ColumnName );
    QString name = nameItem->text();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.removeWorkspace( name );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::warning( this, tr("Ошибка"), tf.m_errText, QMessageBox::Close );
        return;
    }

    ui->workspacesTable->removeRow( row );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка
 */
void WorkspacesDialog::reload() {

    ui->workspacesTable->setRowCount( 0 );

    TFRequest tf;
    tf.setConfig( m_config );
    tf.workspaces();
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        return;
    }

    int row = 0;
    QList<Workspace> workspacesAzure = parseWorkspaces( tf.m_response );
    foreach( const Workspace& workspace, workspacesAzure ) {

        QString maps;
        QStringList localPathes;
        QList<WorkfoldItem> workfolds = loadWorkfolds( workspace.name );
        foreach(const WorkfoldItem& item, workfolds) {
            maps += maps.isEmpty() ? "" : "\n";
            maps += QString("%1 -> %2").arg(item.pathServer, item.pathLocal);
            localPathes.append( item.pathLocal );
        }

        QTableWidgetItem* nameItem    = new QTableWidgetItem;
        QTableWidgetItem* pcItem      = new QTableWidgetItem;
        QTableWidgetItem* ownerItem   = new QTableWidgetItem;
        QTableWidgetItem* commentItem = new QTableWidgetItem;
        QTableWidgetItem* mapItem     = new QTableWidgetItem;

        nameItem   ->setText( workspace.name     );
        pcItem     ->setText( workspace.computer );
        ownerItem  ->setText( workspace.owner    );
        commentItem->setText( workspace.comment  );
        mapItem    ->setText( maps               );
        mapItem    ->setData( Qt::UserRole,  localPathes );

        if( workspace.name != m_config.m_azure.workspace ) {
            nameItem->setForeground( QBrush(Qt::darkGray) );
        } else {
            nameItem->setToolTip( tr("Текущее рабочее пространство") );
        }

        ui->workspacesTable->insertRow( row );
        ui->workspacesTable->setItem( row, ColumnName   , nameItem    );
        ui->workspacesTable->setItem( row, ColumnPC     , pcItem      );
        ui->workspacesTable->setItem( row, ColumnOwner  , ownerItem   );
        ui->workspacesTable->setItem( row, ColumnComment, commentItem );
        ui->workspacesTable->setItem( row, ColumnMap    , mapItem     );

        row++;
    }

    ui->workspacesTable->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );
    ui->workspacesTable->horizontalHeader()->setStretchLastSection( true );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Загрузка сопоставлений в рабочей области
 * \param workspace Рабочая область
 * \return Список сопоставлений
 */
QList<WorkfoldItem> WorkspacesDialog::loadWorkfolds( const QString& workspace ) {

    TFRequest tf;
    tf.setConfig( m_config );
    tf.workfolds( workspace );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        return {};
    }

    return parseWorkfolds( tf.m_response );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void WorkspacesDialog::setConfig( const Config& cfg ) {

    m_config = cfg;
    reload();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка интерфейса
 */
void WorkspacesDialog::setupUI() {

    ui->setupUi( this );

    setWindowTitle( tr("Выбор рабочей области") );

    ui->workspacesTable->setSelectionMode    ( QAbstractItemView::SingleSelection );
    ui->workspacesTable->setSelectionBehavior( QAbstractItemView::SelectRows      );
    ui->workspacesTable->setColumnCount( ColumnCount );
    ui->workspacesTable->setHorizontalHeaderLabels( {
                                                       tr("Имя"          ), // ColumnName
                                                       tr("Компьютер"    ), // ColumnPC
                                                       tr("Владелец"     ), // ColumnOwner
                                                       tr("Примечание"   ), // ColumnComment
                                                       tr("Сопоставление"), // ColumnMap
                                                    } );

    QHeaderView* vHeader = ui->workspacesTable->verticalHeader();
    QHeaderView* hHeader = ui->workspacesTable->horizontalHeader();

    vHeader->setDefaultSectionSize( vHeader->minimumHeight() );
    vHeader->hide();

    hHeader->setStretchLastSection( true );

    connect( ui->createButton   , &QPushButton ::clicked           , this, &WorkspacesDialog::createWorkspace );
    connect( ui->removeButton   , &QPushButton ::clicked           , this, &WorkspacesDialog::removeWorkspace );
    connect( ui->workspacesTable, &QTableWidget::currentItemChanged, this, &WorkspacesDialog::updateStateOK   );

    updateStateOK();
}
//----------------------------------------------------------------------------------------------------------
