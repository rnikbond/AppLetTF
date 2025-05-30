//----------------------------------------
#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H
//----------------------------------------
#include <QMenu>
#include <QWidget>
//----------------------------------------
#include "Config.h"
//----------------------------------------
namespace Ui { class HistoryWidget; }
//----------------------------------------
class TFRequest;
//----------------------------------------
struct HistoryItem;
//----------------------------------------

class HistoryWidget : public QWidget {

    Q_OBJECT

    enum Columns {
        ColumnVersion  = 0,
        ColumnAuthor   = 1,
        ColumnDateTime = 2,
        ColumnComment  = 3,
        ColumnCount    = 4
    };

    enum ColumnsDetail {
        ColumnCaption  = 0,
        ColumnValue    = 1,
    };

    enum CustomRoles {
        MinChangesetRole = Qt::UserRole + 1,
    };

    QString m_path;
    Config  m_config;

    QMenu  * m_ctxMenu     ;
    QAction* m_detailAction;

public:

    explicit HistoryWidget( QWidget* parent = nullptr );
    ~HistoryWidget();

    void setConfig( const Config& cfg );
    void reload( const QString& path );
    QString path() const;

private:

    void showCtxMenu( const QPoint& pos );
    void showDetail();

    void insertItems( TFRequest& tf );

private:

    Ui::HistoryWidget* ui;

    void setupActions();
    void setupCtxMenu();

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& result );
    void projectSelected( const QString& path );
};
//----------------------------------------------------------------------------------------------------------

#endif // HISTORYWIDGET_H
