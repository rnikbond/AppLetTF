//----------------------------------------
#include <QMessageBox>
//----------------------------------------
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "HistoryWidget.h"
#include "ui_HistoryWidget.h"
//----------------------------------------

HistoryWidget::HistoryWidget( QWidget* parent ) : QWidget(parent), ui(new Ui::HistoryWidget)
{
    ui->setupUi( this );

    setupActions();
    setupCtxMenu();

    ui->splitter->setSizes( {500, 200} );

    ui->detail->hide();

    ui->historyTable->setColumnCount      ( ColumnCount );
    ui->historyTable->setSelectionBehavior( QAbstractItemView::SelectRows );
    ui->historyTable->setSelectionMode    ( QAbstractItemView::SingleSelection );
    ui->historyTable->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->historyTable->setEditTriggers     ( QAbstractItemView::NoEditTriggers );
    ui->historyTable->setHorizontalHeaderLabels( {
                                                     tr("Набор изменений"),
                                                     tr("Автор"),
                                                     tr("Дата"),
                                                     tr("Комментарий"),
                                                 } );

    QHeaderView* vHeader = ui->historyTable->verticalHeader  ();
    QHeaderView* hHeader = ui->historyTable->horizontalHeader();

    vHeader->hide();
    vHeader->setDefaultSectionSize( vHeader->minimumSectionSize() );
    hHeader->setStretchLastSection( true );

    connect( ui->historyTable, &QTableWidget       ::customContextMenuRequested, this, &HistoryWidget::showCtxMenu     );
    connect( ui->historyTable, &QTableWidget       ::itemDoubleClicked         , this, &HistoryWidget::showDetail      );
    connect( ui->pathLabel   , &QLabel             ::linkActivated             , this, &HistoryWidget::projectSelected );
    connect( ui->detail      , &HistoryDetailWidget::commandExecuted           , this, &HistoryWidget::commandExecuted );
    connect( ui->detail      , &HistoryDetailWidget::projectSelected           , this, &HistoryWidget::projectSelected );
}
//----------------------------------------------------------------------------------------------------------

HistoryWidget::~HistoryWidget() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение детальной информации о наборе изменений
 */
void HistoryWidget::showDetail() {

    int row = ui->historyTable->currentRow();
    QTableWidgetItem* item = ui->historyTable->item( row, ColumnVersion );
    if( item == nullptr ) {
        ui->detail->hide();
        return;
    }

    QString version = item->data(Qt::DisplayRole).toString();
    ui->detail->reload( m_path, version );

    ui->detail->show();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка журнала изменений
 * \param path Путь к файлу или каталогу, журнал изменений которого нужно отобразить
 *
 * Пример \a path:
 * $/Proj/subproj
 * $/Proj/subproj/file.txt
 */
void HistoryWidget::reload( const QString& path ) {

    m_path = path;

    ui->pathLabel->setText( tr("Исходное расположение: <a href=\"%1\">%1</u>").arg(path) );
    ui->historyTable->setRowCount( 0 );

    TFRequest tf;
    tf.setConfig( m_config );
    tf.history( path);
    emit commandExecuted( tf.m_isErr, tf.m_errCode, tf.m_errText, tf.m_response );

    if( tf.m_isErr ) {
        QMessageBox::critical( this, tr("Ошибка"), tf.m_errText );
        return;
    }

    QList<HistoryItem> historyItems = parseHistory( tf.m_response );

    int row = 0;
    foreach( const HistoryItem& historyItem, historyItems) {

        ui->historyTable->insertRow( row );

        QTableWidgetItem* itemSet     = new QTableWidgetItem( historyItem.version  );
        QTableWidgetItem* itemAuthor  = new QTableWidgetItem( historyItem.author   );
        QTableWidgetItem* itemDate    = new QTableWidgetItem( historyItem.datetime );
        QTableWidgetItem* itemComment = new QTableWidgetItem( historyItem.comment  );

        ui->historyTable->setItem( row, ColumnVersion , itemSet     );
        ui->historyTable->setItem( row, ColumnAuthor  , itemAuthor  );
        ui->historyTable->setItem( row, ColumnDateTime, itemDate    );
        ui->historyTable->setItem( row, ColumnComment , itemComment );

        row++;
    }

    QHeaderView* hHeader = ui->historyTable->horizontalHeader();
    hHeader->resizeSections( QHeaderView::ResizeToContents );
    hHeader->resizeSection( hHeader->count() - 1, QHeaderView::Stretch );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение пути, для которого отображается журнал изменений
 * \return Путь
 */
QString HistoryWidget::path() const {
    return m_path;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение контекстного меню
 * \param pos Позиция для отображения меню
 */
void HistoryWidget::showCtxMenu( const QPoint& pos ) {

    m_ctxMenu->popup( ui->historyTable->viewport()->mapToGlobal(pos) );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка действий
 */
void HistoryWidget::setupActions() {

    m_detailAction = new QAction( QIcon(""), tr("Посмотреть детали") );
    connect( m_detailAction, &QAction::triggered, this, &HistoryWidget::showDetail );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Настройка контекстного меню
 */
void HistoryWidget::setupCtxMenu() {

    m_ctxMenu = new QMenu( this );
    m_ctxMenu->addAction( m_detailAction );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void HistoryWidget::setConfig( const Config& cfg ) {

    m_config = cfg;
    ui->detail->setConfig( cfg );
}
//----------------------------------------------------------------------------------------------------------
