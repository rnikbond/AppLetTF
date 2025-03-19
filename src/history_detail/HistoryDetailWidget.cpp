//----------------------------------------
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "HistoryDetailWidget.h"
#include "ui_HistoryDetailWidget.h"
//----------------------------------------

HistoryDetailWidget::HistoryDetailWidget( QWidget* parent ) : QWidget(parent), ui(new Ui::HistoryDetailWidget)
{
    ui->setupUi( this );

    setupActions();
    setupCtxMenu();

    ui->infoLabel->clear();
    ui->infoLabel->setWordWrap( true );

    ui->filesTable->setColumnCount      ( ColumnCount );
    ui->filesTable->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->filesTable->setSelectionMode    ( QAbstractItemView::SingleSelection );
    ui->filesTable->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->filesTable->setEditTriggers     ( QAbstractItemView::NoEditTriggers );
    ui->filesTable->setShowGrid( false );
    ui->filesTable->setColumnCount( ColumnCount );
    ui->filesTable->setHorizontalHeaderLabels( {tr("Файл"), tr("Статус"), tr("Папка")} );

    QHeaderView* hHeader = ui->filesTable->horizontalHeader();
    QHeaderView* vHeader = ui->filesTable->verticalHeader  ();

    vHeader->hide();
    vHeader->setDefaultSectionSize( vHeader->minimumSectionSize() );
    hHeader->setStretchLastSection( true );

    connect( ui->filesTable, &QTableWidget::customContextMenuRequested, this, &HistoryDetailWidget::showCtxMenu     );
    connect( ui->infoLabel , &QLabel      ::linkActivated             , this, &HistoryDetailWidget::projectSelected );
}
//----------------------------------------------------------------------------------------------------------

HistoryDetailWidget::~HistoryDetailWidget() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка детальной информации о наборе изменений
 * \param path Путь к файлу или каталогу, изменения которого нужно перезагрузить
 * \param version Набор изменений (версия), информацию о которого нужно отобразить
 */
void HistoryDetailWidget::reload( const QString& path, const QString& version ) {

    m_path    = path;
    m_version = version;

    ui->filesTable->setRowCount( 0 );
    ui->infoLabel->clear();

    TFRequest tf;
    tf.setConfig( m_config );
    tf.historyCertain( m_path, version );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        return;
    }

    HistoryDetailItem detail = parseDetailHistory( tf.m_response);

    {
        QString html = "<table width=100% height=100% border=0 cellspacing=-1 cellpadding=3 style=\"vertical-align:middle\">";

        html += "<tr>";
        html += tr("<td>Набор изменений</td>");
        html += QString("<td>%1</td>").arg(detail.version);
        html += "</tr>";

        html += "<tr>";
        html += tr("<td>Автор</td>");
        html += QString("<td>%1</td>").arg(detail.author);
        html += "</tr>";

        html += "<tr>";
        html += tr("<td>Дата и время</td>");
        html += QString("<td>%1</td>").arg(detail.datetime);
        html += "</tr>";

        html += "<tr></tr>";

        html += "<tr>";
        html += tr("<td colspan=2><b>Комментарий:</b></td>");
        html += "</tr>";
        html += "<tr>";
        html += QString("<td colspan=2>%1</td>").arg(detail.comment.replace("\n", "<br/>"));
        html += "</tr>";

        html += "<tr></tr>";

        html += "<tr>";
        html += tr("<td width=25%>Исходное расположение</td>");
        html += QString("<td><a href=\"%1\">%1</a></td>").arg(m_path);
        html += "</tr>";

        html += "</table>";

        ui->infoLabel->setText( html );
    }

    int row = 0;
    foreach( const HistoryFile& file, detail.files ) {

        QString folder  ;
        QString fileName;
        splitPath( file.file, folder, fileName );

        QTableWidgetItem* fileItem   = new QTableWidgetItem;
        QTableWidgetItem* statusItem = new QTableWidgetItem;
        QTableWidgetItem* folderItem = new QTableWidgetItem;

        fileItem  ->setIcon( icon(fileName, File) );
        fileItem  ->setText( fileName    );
        statusItem->setText( file.status );
        folderItem->setIcon( icon(fileName, Folder) );
        folderItem->setText( folder      );

        ui->filesTable->insertRow( row );
        ui->filesTable->setItem( row, ColumnFile  , fileItem   );
        ui->filesTable->setItem( row, ColumnStatus, statusItem );
        ui->filesTable->setItem( row, ColumnFolder, folderItem );

        row++;
    }

    QHeaderView* hHeader = ui->filesTable->horizontalHeader();
    hHeader->resizeSections( QHeaderView::ResizeToContents );
    hHeader->resizeSection( hHeader->count() - 1, QHeaderView::Stretch );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сравнение выбранного файла с предыдущей версией
 */
void HistoryDetailWidget::diff() {

    int row = ui->filesTable->currentRow();

    QTableWidgetItem* fileItem = ui->filesTable->item( row, ColumnFile   );
    QTableWidgetItem* foldItem = ui->filesTable->item( row, ColumnFolder );

    if( fileItem == nullptr || foldItem == nullptr ) {
        return;
    }

    QString file = fileItem->text();
    QString fold = foldItem->text();
    QString path = QString("%1/%2").arg(fold, file);

    TFRequest tf;
    tf.setConfig( m_config );
    tf.historyDiffPrev( path, m_version );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        return;
    }

    QList<HistoryItem> historyItems = parseHistory( tf.m_response );
    if( historyItems.count() != 2 ) {
        return;
    }

    QString versionPrev = historyItems[1].version;
    tf.difference( path, m_version, versionPrev );
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение контекстного меню
 * \param pos Позиция для отображения
 */
void HistoryDetailWidget::showCtxMenu( const QPoint& pos ) {

    m_ctxMenu->popup( ui->filesTable->viewport()->mapToGlobal(pos) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка действий
 */
void HistoryDetailWidget::setupActions() {

    m_diffAction = new QAction( QIcon(":/compare_files.png"), tr("Сравнить с предыдущей версией") );
    connect( m_diffAction, &QAction::triggered, this, &HistoryDetailWidget::diff );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка контекстного меню
 */
void HistoryDetailWidget::setupCtxMenu() {

    m_ctxMenu = new QMenu( this );
    m_ctxMenu->addAction( m_diffAction );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void HistoryDetailWidget::setConfig( const Config& cfg ) {

    m_config = cfg;
}
//----------------------------------------------------------------------------------------------------------
