//----------------------------------------
#ifndef NEWWORKSPACEDIALOG_H
#define NEWWORKSPACEDIALOG_H
//----------------------------------------
#include <QDialog>
//----------------------------------------
#include "Config.h"
//----------------------------------------
namespace Ui { class NewWorkspaceDialog; }
//----------------------------------------

class NewWorkspaceDialog : public QDialog {

    Q_OBJECT

    Config m_config;

public:

    explicit NewWorkspaceDialog( QWidget* parent = nullptr );
    ~NewWorkspaceDialog();

    void setConfig( const Config& cfg );

    QString name() const;
    QString azureFolderPath() const;
    QString localFolderPath() const;

private:

    void selectAzureFolder();
    void selectLocalFolder();

    void updateStateOK();

private:

    Ui::NewWorkspaceDialog* ui;
    void setupUI();

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& response );
};

#endif // NEWWORKSPACEDIALOG_H
