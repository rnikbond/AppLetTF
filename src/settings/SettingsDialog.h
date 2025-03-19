//----------------------------------------
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
//----------------------------------------
#include <QDialog>
//----------------------------------------
#include "common.h"
#include "Config.h"
//----------------------------------------
namespace Ui { class SettingsDialog; }
//----------------------------------------
class QListWidgetItem;
//----------------------------------------

class SettingsDialog : public QDialog {

    Q_OBJECT

    Ui::SettingsDialog* ui;

public:

    explicit SettingsDialog( QWidget* parent = nullptr );
    ~SettingsDialog();

    void setConfig( const Config& cfg );
    const Config& config() const;

private: // Config

    Config m_config;

    void save   ();
    void restore();

private: // Pages

    void initPages();

    void selectPage( QListWidgetItem* itemPage, QListWidgetItem* );
    QListWidgetItem* findPage( int page ) const;

private: // Page Common

    void initPageCommon         ();
    void saveConfigPageCommon   ();
    void restoreConfigPageCommon();

private: // Page Azure

    void initPageAzure         ();
    void saveConfigPageAzure   ();
    void restoreConfigPageAzure();

    void changeTfPath();
    void changeWorkspace();

    void reconnectAzure();

    void logAzureInfo   ( const QString& text );
    void logAzureError  ( const QString& text );
    void logAzureWarning( const QString& text );
    void logAzureSuccess( const QString& text );
    void logAzureText   ( const QString& text, const QColor& color );

signals:

    void commandExecuted( bool isErr, int code, const QString& err, const QStringList& result );
};
//----------------------------------------------------------------------------------------------------------

#endif // SETTINGSDIALOG_H
