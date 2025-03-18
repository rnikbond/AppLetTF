//----------------------------------------
#ifndef APPLETTF_H
#define APPLETTF_H
//----------------------------------------
#include <QMainWindow>
//----------------------------------------
#include "Config.h"
#include "TFRequest.h"
//----------------------------------------
namespace Ui { class AppLetTF; }
//----------------------------------------

class AppLetTF : public QMainWindow {

    Q_OBJECT

public:

    AppLetTF( QWidget* parent = nullptr );
    ~AppLetTF();

    void init();

private: // Config

    Config m_config;

private: // TF

    TFRequest* m_tf;
    void reactOnCmdExecuted();

private: // UI

    Ui::AppLetTF* ui;

    void setupUI();
    void moveToCenterScreen();
};
//----------------------------------------------------------------------------------------------------------

#endif // APPLETTF_H
