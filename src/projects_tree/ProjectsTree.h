//----------------------------------------
#ifndef PROJECTSTREE_H
#define PROJECTSTREE_H
//----------------------------------------
#include <QMenu>
#include <QWidget>
//----------------------------------------
#include "Config.h"
//----------------------------------------
namespace Ui { class ProjectsTree; }
//----------------------------------------
class QTableWidget;
class QTreeWidgetItem;
//----------------------------------------
class HistoryWidget;
//----------------------------------------
struct AzureItem;
//----------------------------------------

class ProjectsTree : public QWidget {

    Q_OBJECT

    QMenu  * m_ctxMenu           ;
    QAction* m_reloadAction      ;
    QAction* m_historyAction     ;
    QAction* m_cloneLastedAction ;
    QAction* m_cloneRewriteAction;
    QAction* m_cloneСertainAction;
    QAction* m_helpAction        ;

    Config m_config;
    QMap<QString, QTreeWidgetItem*> m_treeItems;

public:

    explicit ProjectsTree( QWidget* parent = nullptr );
    ~ProjectsTree();

    void setConfig( const Config& cfg );

    void reload();
    void clear();

    void cloneLasted ();
    void cloneRewrite();
    void cloneCertain();
    void history();
    void help();

    QList<QAction*> actions() const;

private:

    void expand( QTreeWidgetItem* item );
    void expand( QTreeWidgetItem* item, int );
    void createTreeItems( QTreeWidgetItem* parent, const QList<AzureItem>& entries );
    void setCurrentProject( const QString& path );

    HistoryWidget* historyTab( const QString& path );
    void closeHistoryTab( int index );

    void updateActions();
    void showCtxMenu( const QPoint& pos );

private:

    Ui::ProjectsTree* ui;

    void setupActions();
    void setupCtxMenu();

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& response );
};
//----------------------------------------------------------------------------------------------------------

#endif // PROJECTSTREE_H
