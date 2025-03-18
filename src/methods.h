//----------------------------------------
#ifndef METHODS_H
#define METHODS_H
//----------------------------------------
#include <QStringList>
//----------------------------------------
#include "common.h"
//----------------------------------------

QList<Workspace> parseWorkspaces( const QStringList& items );
QList<QPair<int, int>> parsePositions( const QString& title );

#endif // METHODS_H
//----------------------------------------
