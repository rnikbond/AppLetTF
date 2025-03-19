//----------------------------------------
#ifndef WORKSPACESDIALOG_H
#define WORKSPACESDIALOG_H
//----------------------------------------
#include <QDialog>
//----------------------------------------
#include "Config.h"
#include "common.h"
//----------------------------------------
namespace Ui { class WorkspacesDialog; }
//----------------------------------------
struct Workspace;

class WorkspacesDialog : public QDialog {

    Q_OBJECT

    enum Columns {
        ColumnName   , ///< Столбец "Имя"
        ColumnPC     , ///< Столбец "Компьютер"
        ColumnOwner  , ///< Столбец "Владелец"
        ColumnComment, ///< Столбец "Примечание"
        ColumnMap    , ///< Столбец "Сопоставление"
        ColumnCount    ///< Количество столбцов
    };

    Config m_config;

public:

    explicit WorkspacesDialog( QWidget* parent = nullptr );
    ~WorkspacesDialog();

    void setConfig( const Config& cfg );
    Workspace selectedWorkspace() const;

private:

    void createWorkspace();
    void removeWorkspace();

    void updateStateOK();

    void reload();
    QList<WorkfoldItem> loadWorkfolds( const QString& workspace );

private:

    Ui::WorkspacesDialog* ui;

    void setupUI();

signals:

     void commandExecuted( bool isErr, int code, const QString& err, const QStringList& response );
};
//----------------------------------------------------------------------------------------------------------

#endif // WORKSPACESDIALOG_H
