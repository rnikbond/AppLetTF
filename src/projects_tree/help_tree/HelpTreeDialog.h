//----------------------------------------
#ifndef HELPTREEDIALOG_H
#define HELPTREEDIALOG_H
//----------------------------------------
#include <QDialog>
//----------------------------------------
namespace Ui { class HelpTreeDialog; }
//----------------------------------------

class HelpTreeDialog : public QDialog {

    Q_OBJECT

public:

    explicit HelpTreeDialog( QWidget* parent = nullptr );
    ~HelpTreeDialog();

    void setText();

private:

    Ui::HelpTreeDialog* ui;

    void moveToCenter();

protected:

    void showEvent(QShowEvent *) override;
};
//----------------------------------------------------------------------------------------------------------

#endif // HELPTREEDIALOG_H
