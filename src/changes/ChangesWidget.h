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

class ChangesWidget : public QWidget {

    Q_OBJECT

    enum CustomRoles {
        PathRole   = Qt::UserRole + 1,
        StatusRole = Qt::UserRole + 2,
    };

    enum FileStatuses {
        UnknownStatus,
        CreateStatus,
        ChangeStatus,
        DeleteStatus,
    };

    bool m_reloadState;
    Config m_config;
    QMap<QString, int> m_statusesTfsMap;

public:

    explicit ChangesWidget(QWidget *parent = nullptr);
    ~ChangesWidget();

    void setConfig( const Config& cfg );
    void reload();

    bool isReloaded() const;

private: // Prepared Tree

    QToolButton* m_reloadPreparedButton;
    QMenu      * m_preparedCtxMenu     ;
    QAction    * m_preparedDiffAction  ;
    QAction    * m_preparedCancelAction;
    QAction    * m_excludeAction       ;
    QMap<QString, QTreeWidgetItem*> m_preparedDirItems;

    void setupPrepared ();
    void reloadPrepared();

    void reactOnCommit();
    void reactOnPreparedItemChanged( QTreeWidgetItem* item, int );
    void reactOnPreparedMenuRequested( const QPoint& pos );
    void reactOnPreparedDiff();
    void reactOnPreparedCancel();
    void reactOnPreparedExclude();

    void createPreparedFileItem( const QString& file, const QString& path, int status );
    void updateCommitSize();

private: // Detected Tree

    QToolButton* m_reloadDetectedButton;
    QMenu      * m_detectedCtxMenu;
    QAction    * m_detectedApplyAction;
    QMap<QString, QTreeWidgetItem*> m_detectedDirItems;

    void setupDetected ();
    void reloadDetected();
    void reactOnDetectedMenuRequested( const QPoint& pos );
    void reactOnDetectedApply();

    void createDetectedFileItem( const QString& file, const QString& path, int status );

private: // Exclude Tree

    QToolButton* m_reloadExcludedButton;
    QStringList m_excluded;
    QMap<QString, QTreeWidgetItem*> m_excludedDirItems;

    void setupExcluded();
    void reloadExcluded();

    void addExcludedFileItem( const QString& path );
    void createExcludedFileItem( const QString& file, const QString& path );

private:

    Ui::ChangesWidget* ui;

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& result );
};
//----------------------------------------------------------------------------------------------------------

#endif // CHANGESWIDGET_H
