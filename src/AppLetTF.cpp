//----------------------------------------
#include <QScreen>
//----------------------------------------
#include "AppLetTF.h"
#include "ui_AppLetTF.h"
//----------------------------------------

AppLetTF::AppLetTF( QWidget* parent ) : QMainWindow(parent), ui(new Ui::AppLetTF)
{
    ui->setupUi(this);
    setupUI();
}
//----------------------------------------------------------------------------------------------------------

AppLetTF::~AppLetTF() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::setupUI() {

    setWindowTitle( tr("AppLet TF") );
    moveToCenterScreen();
}
//----------------------------------------------------------------------------------------------------------

void AppLetTF::moveToCenterScreen() {

    QRect rect  = QGuiApplication::primaryScreen()->geometry();
    QPoint center = rect.center();
    center.setX( center.x() - (width ()/2) );
    center.setY( center.y() - (height()/2) );
    move( center );
}
//----------------------------------------------------------------------------------------------------------
