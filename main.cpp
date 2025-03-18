//----------------------------------------
#include <QTimer>
#include <QApplication>
//----------------------------------------
#include "AppLetTF.h"
//----------------------------------------

int main( int argc, char* argv[] ) {

    QApplication app( argc, argv );

    AppLetTF applet;
    applet.show();

    QTimer::singleShot( 0, &applet, &AppLetTF::init );

    return app.exec();
}
//----------------------------------------------------------------------------------------------------------
