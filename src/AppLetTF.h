//----------------------------------------
#ifndef APPLETTF_H
#define APPLETTF_H
//----------------------------------------
#include <QMainWindow>
//----------------------------------------
namespace Ui { class AppLetTF; }
//----------------------------------------

class AppLetTF : public QMainWindow {

    Q_OBJECT

public:

    AppLetTF( QWidget* parent = nullptr );
    ~AppLetTF();

private: // UI

    Ui::AppLetTF* ui;

    void setupUI();
    void moveToCenterScreen();
};
//----------------------------------------------------------------------------------------------------------

#endif // APPLETTF_H
