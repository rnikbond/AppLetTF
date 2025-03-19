//----------------------------------------
#ifndef METHODS_H
#define METHODS_H
//----------------------------------------
#include <QPixmap>
#include <QStringList>
//----------------------------------------
#include "common.h"
//----------------------------------------

QList<Workspace>    parseWorkspaces   ( const QStringList& items );
QList<AzureItem>    parseEntries      ( const QStringList& entries );
QList<HistoryItem>  parseHistory      ( const QStringList& items );
HistoryDetailItem   parseDetailHistory( const QStringList& items );
QList<WorkfoldItem> parseWorkfolds    ( const QStringList& items );

QList<QPair<int, int>> parsePositions( const QString& title );

QString projectName( const QString& path );
void splitPath( const QString& path, QString& folder, QString& file );

QPixmap icon( const QString& name, int fileType );

void createWorkDir();
QString workDirPath();
QString workDirName();

#endif // METHODS_H
//----------------------------------------
