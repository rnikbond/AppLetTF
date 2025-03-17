//----------------------------------------
#include <QApplication>
//----------------------------------------
#include "AppLetTF.h"
//----------------------------------------

int main( int argc, char* argv[] ) {

    QApplication app( argc, argv );

    AppLetTF applet;
    applet.show();

    return app.exec();
}
//----------------------------------------------------------------------------------------------------------
