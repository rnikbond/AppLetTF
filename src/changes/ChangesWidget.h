//----------------------------------------
#ifndef CHANGESWIDGET_H
#define CHANGESWIDGET_H
//----------------------------------------
#include <QWidget>
//----------------------------------------
#include "Config.h"
//----------------------------------------
namespace Ui { class ChangesWidget; }
//----------------------------------------
class QMenu;
class QSettings;
class QToolButton;
class QTreeWidget;
class QTreeWidgetItem;
//----------------------------------------
class TFRequest;
//----------------------------------------

class ChangesWidget : public QWidget {

    Q_OBJECT

    bool m_reloadState;
    Config m_config;
    QAction* m_reloadAction;
    QMap<QString, int> m_statusesTfsMap;

public:

    explicit ChangesWidget(QWidget *parent = nullptr);
    ~ChangesWidget();

    void setConfig( const Config& cfg );
    void reload();

    void saveData   ();
    void restoreData();

    bool isReloaded() const;

    QList<QAction*> actions() const;

    bool question( const QString& caption, const QString& text, const QString& detail = "" ) const;

private: // Prepared Tree

    QMenu      * m_preparedCtxMenu     ;
    QAction    * m_preparedDiffAction  ;
    QAction    * m_preparedCancelAction;
    QAction    * m_excludeAction       ;
    QMap<QString, QTreeWidgetItem*> m_preparedDirItems;

    void setupPrepared ();
    void reloadPrepared();
    void reloadPrepared( const QMap<QString, TFRequest*>& responses );

    void reactOnCommit();
    void reactOnPreparedMenuRequested( const QPoint& pos );
    void reactOnPreparedDiff();
    void reactOnPreparedCancel();
    void reactOnPreparedExclude();
    void reactOnPartSelectCheck();

    void updateCommitSize();
    void updatePreparedActions();

private: // Detected Tree

    QMenu      * m_detectedCtxMenu;
    QAction    * m_detectedApplyAction;
    QMap<QString, QTreeWidgetItem*> m_detectedDirItems;

    void setupDetected ();
    void reloadDetected();
    void reloadDetected( const QMap<QString, TFRequest*>& responses );

    void reactOnDetectedMenuRequested( const QPoint& pos );
    void reactOnDetectedApply();

    void updateDetectedActions();

private: // Exclude Tree

    QMenu      * m_excludeCtxMenu;
    QAction    * m_includeAction;
    QStringList m_excluded;
    QMap<QString, QTreeWidgetItem*> m_excludedDirItems;

    void setupExcluded();
    void reloadExcluded();
    void reactOnInclude();
    void reactOnExcludeMenuRequested( const QPoint& pos );

    void addExcludedFileItem( const QString& path );

    void updateExcludeActions();

private:

    Ui::ChangesWidget* ui;

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& result );
};
//----------------------------------------------------------------------------------------------------------

#endif // CHANGESWIDGET_H
