//----------------------------------------
#ifndef HISTORYDETAILWIDGET_H
#define HISTORYDETAILWIDGET_H
//----------------------------------------
#include <QMenu>
#include <QWidget>
//----------------------------------------
#include "Config.h"
//----------------------------------------
namespace Ui { class HistoryDetailWidget; }
//----------------------------------------

class HistoryDetailWidget : public QWidget {

    Q_OBJECT

    enum Columns {
        ColumnFile   = 0,
        ColumnStatus = 1,
        ColumnFolder = 2,
        ColumnCount  = 3
    };

    QString m_path   ;
    QString m_version;
    Config  m_config ;

    QMenu  * m_ctxMenu   ;
    QAction* m_diffAction;

public:

    explicit HistoryDetailWidget( QWidget* parent = nullptr );
    ~HistoryDetailWidget();

    void setConfig( const Config& cfg );
    void reload( const QString& path , const QString& version );

private:

    void diff();

    void showCtxMenu( const QPoint& pos );
    void showDetail();

private:

    Ui::HistoryDetailWidget* ui;

    void setupActions();
    void setupCtxMenu();

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& result );
    void projectSelected( const QString& path );
};
//----------------------------------------------------------------------------------------------------------

#endif // HISTORYDETAILWIDGET_H
//----------------------------------------------------------------------------------------------------------
