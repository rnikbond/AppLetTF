//----------------------------------------
#ifndef METHODS_H
#define METHODS_H
//----------------------------------------
#include <QPixmap>
#include <QStringList>
#include <QTreeWidgetItem>
//----------------------------------------
#include "common.h"
//----------------------------------------

QList<Workspace>    parseWorkspaces    ( const QStringList& items );
QList<AzureItem>    parseEntries       ( const QStringList& entries );
QList<HistoryItem>  parseHistory       ( const QStringList& items );
HistoryDetailItem   parseDetailHistory ( const QStringList& items );
QList<WorkfoldItem> parseWorkfolds     ( const QStringList& items );
QList<StatusItem>   parseStatusPrepared( const QStringList& items, const QString& dirPath );
QList<StatusItem>   parseStatusDetected( const QStringList& items, const QString& dirPath );
QList<StatusItem>   parseStatus        ( const QStringList& items, const QList<QPair<int, int>>& positions );

QList<QPair<int, int>> parsePositions( const QString& title );

QString projectName( const QString& path );
void splitPath( const QString& path, QString& folder, QString& file );

QPixmap icon( const QString& name, int fileType );
QPixmap joinIconsFile( const QString& fileName, const QString& iconPath );

void createWorkDir();
QString workDirPath();
QString workDirName();

void createTreeSubDirs( QTreeWidgetItem* parent, int pathRole, QStringList& pathDirs, int idxFrom );
void collectPathFiles( const QTreeWidgetItem* item, int role, QStringList& collection );

#endif // METHODS_H
//----------------------------------------
