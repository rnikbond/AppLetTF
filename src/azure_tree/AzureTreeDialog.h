//----------------------------------------
#ifndef AZURETREEDIALOG_H
#define AZURETREEDIALOG_H
//----------------------------------------
#include <QDialog>
//----------------------------------------
#include "Config.h"
//----------------------------------------
namespace Ui { class AzureTreeDialog; }
//----------------------------------------
class QTreeWidgetItem;
//----------------------------------------
struct AzureItem;
//----------------------------------------

class AzureTreeDialog : public QDialog {

    Q_OBJECT

    Config m_config;
    QMap<QString, QTreeWidgetItem*> m_treeItems;

public:

    explicit AzureTreeDialog( QWidget* parent = nullptr );
    ~AzureTreeDialog();

    void setConfig( const Config& cfg );
    QString selectedPath() const;

private:

    void reload();
    void updateStateOK();

    void expand( QTreeWidgetItem* item );

    void createTreeItems( QTreeWidgetItem* parent, const QList<AzureItem>& entries );

private:

    Ui::AzureTreeDialog* ui;

    void setupUI();

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& response );
};

#endif // AZURETREEDIALOG_H
