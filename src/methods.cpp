//----------------------------------------
#include <QDir>
#include <QStyle>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTemporaryFile>
#include <QFileIconProvider>
//----------------------------------------
#include "methods.h"
//----------------------------------------

/*!
 * \brief Получение списка рабочих пространств
 * \param items Ответ запроса workspaces
 * \return Список рабочих пространств
 */
QList<Workspace> parseWorkspaces( const QStringList& items ) {

    /* Пример вывода команды
    [0] Collection: http://host:port/a/b/
    [1] Workspace Owner                       Computer Comment
    [2] --------- --------------------------- -------- --------------------------------
    [3] WKPS1    Ололоев Ололой Ололоевич ololo
    [4] WKPS2    Ололоев Ололой Ололоевич OLOLO
    [5] WKPS3213 Ололоев Ололой Ололоевич ololo      Commemt
    */

    if( items.count() < 4 ) {
        return {}; // При наличии созданных рабочих пространств, должно быть минимум 4 строки
    }

    // Строка с индексом 2 - отчерк заголовков
    QList<QPair<int, int>> positions = parsePositions( items.at(2) );
    if( positions.count() != 4 ) {
        return {}; // В ответе команды workspaces должно быть 4 заголовка
    }

    QList<Workspace> workspaces;

    // Начинаем со строки с индексом 3 - с нее начинается информация о рабочих областях
    for( int i = 3; i < items.count(); i++ ) {

        const QString& workspaceInfo = items[i];

        const QPair<int, int>& posName     = positions[0];
        const QPair<int, int>& posOwner    = positions[1];
        const QPair<int, int>& posComputer = positions[2];
        const QPair<int, int>& posComment  = positions[3];

        Workspace workspace;
        workspace.name     = workspaceInfo.mid(posName    .first, posName    .second).trimmed();
        workspace.owner    = workspaceInfo.mid(posOwner   .first, posOwner   .second).trimmed();
        workspace.computer = workspaceInfo.mid(posComputer.first, posComputer.second).trimmed();
        workspace.comment  = workspaceInfo.mid(posComment .first, posComment .second).trimmed();

        workspaces.append( workspace );
    }

    return workspaces;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение содержимого каталога в виде списка
 * \param entries Список, полученный в результате запроса команды "dir"
 * \return Обработанный список
 */
QList<AzureItem> parseEntries( const QStringList& entries ) {

    /* Пример вывода команды
     * [0] "$/KOTMI/mainline/develop:"
     * [1] "$_ver"
     * [2] "$common"
     * [3] "$Examples"
     * [4] "$src"
     * [5] ".tfignore"
     * [6] "Project.pro"
    */

    if( entries.isEmpty() ) {
        return {};
    }

    QString folder = QString(entries[0]).remove(":");

    QList<AzureItem> items;

    for( int i = 1; i < entries.count() - 1; i++ ) {

        QString name = entries.at(i);

        AzureItem item;
        item.type   = name.contains("$") ? TypeFolder : TypeFile;
        item.name   = name.remove("$");
        item.folder = folder;

        items.append( item );
    }

    return items;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение информации об изменениях
 * \param items Список, полученный в результате запроса команды "history -format:brief"
 * \return Информацию об изменениях
 */
QList<HistoryItem> parseHistory( const QStringList& items ) {

    /* Пример вывода команды
     *
     */

    QList<HistoryItem> info;

    if( items.count() < 2 ) {
        return info;
    }

    QList<QPair<int, int>> positions = parsePositions(items[1]);
    if( positions.count() != 4 ) {
        return info; // В ответе команды "history -format:brief" должно быть 4 столбца
    }

    for( int i = 2; i < items.count(); i++ ) {

        const QString& historyStr = items.at(i);

        QPair<int, int> posVersion  = positions[0];
        QPair<int, int> posAuthor   = positions[1];
        QPair<int, int> posDateTime = positions[2];
        QPair<int, int> posComment  = positions[3];

        HistoryItem history;
        history.version  = historyStr.mid(posVersion .first, posVersion .second).trimmed();
        history.author   = historyStr.mid(posAuthor  .first, posAuthor  .second).trimmed();
        history.datetime = historyStr.mid(posDateTime.first, posDateTime.second).trimmed();
        history.comment  = historyStr.mid(posComment .first, posComment .second).trimmed();

        info.append( history );
    }

    return info;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение детальной информации об изменении
 * \param items Список, полученный в результате запроса команды "history -format:detail"
 * \return Информацию об изменениях
 */
HistoryDetailItem parseDetailHistory( const QStringList& items ) {

    /* Пример вывода команды
     * [ 0] Changeset: 32441
     * [ 1] User: Ололоев Ололой Ололоевич
     * [ 2] Date: 13 мар. 2025 г., 14:15:09
     * [ 3] Comment:
     * [ 4]  Удален класс someClass. Теперь используется AnotherClass из BaseLib
     * [ 5] Items:
     * [ 6]  delete $/PROJ/main/dev/src/Settings/dialogs
     * [ 7]  edit $/PROJ/main/dev/src/Settings/Settings.h
     * [ 8]  add $/PROJ/main/dev/src/Settings/Settings.pri
     * [ 9] Check-in Notes:
     * [10]  Code Reviewer:
     * [11]  Performance Reviewer:
     * [12]  Security Reviewer:
     */

    HistoryDetailItem detail;

    if( items.count() < 3 ) {
        return detail; // В выводе команды history -format:detail должно быть минимум 3 строки
    }

    detail.version  = items[0].split("Changeset:")[1].trimmed();
    detail.author   = items[1].split("User:"     )[1].trimmed();
    detail.datetime = items[2].split("Date:"     )[1].trimmed();

    int idxItems   = items.indexOf("Items:");
    int idxCheck   = items.indexOf("Check-in Notes:");
    int idxComment = items.indexOf("Comment:");

    for( int idx = idxComment + 1; idx < idxItems; idx++ ) {
        if( !detail.comment.isEmpty() ) {
            detail.comment += "\n";
        }
        detail.comment += items[idx].trimmed();
    }

    if( idxCheck == -1 ) {
        idxCheck = items.count();
    }

    for( int idx = idxItems + 1; idx < idxCheck; idx++ ) {

        QStringList fileInfo = items[idx].trimmed().split(" ", Qt::SkipEmptyParts);

        HistoryFile file;
        file.status = fileInfo[0].trimmed();
        file.file   = fileInfo[1].trimmed();

        detail.files.append( file );
    }

    return detail;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение информации о сопоставлениях
 * \param items Список, полученный в результате запроса команды "workfolds"
 * \return Информация о сопоставлениях
 */
QList<WorkfoldItem> parseWorkfolds( const QStringList& items ) {

    /* Пример вывода команды
     * [0] ===============================================================================
     * [1] Workspace: MyPC
     * [2] Collection: http://<host>:<port>/tfs/dir/
     * [3] $/dir/main: /home/user/tfs/main
     */

    QList<WorkfoldItem> workfolds;

    for( int i = 2; i < items.count(); i++ ) {

        // [0]: $/dir/main
        // [1]: /home/user/tfs/main
        QStringList bundles = items[i].split(": ", Qt::SkipEmptyParts);
        if( bundles.count() != 2 ) {
            continue;
        }

        if( !bundles[0].contains("$") ) {
            continue;
        }

        WorkfoldItem workfold;
        workfold.pathServer = bundles[0];
        workfold.pathLocal  = QDir::toNativeSeparators(bundles[1]);

        workfolds.append( workfold );
    }

    return workfolds;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение информации об ожидающих изменений из команды status
 * \param items Ответ команды
 * \param dirPath Путь к каталогу сопоставленному каталогу, в которого запрашивался статус
 * \return Список изменений
 */
QList<StatusItem> parseStatusPrepared( const QStringList& items, const QString& dirPath ) {

    /* Пример вывода команды
     * [ 0] File name                                                       Change Local path
     * [ 1] --------------------------------------------------------------- ------ --------
     * [ 2] $/Proje/mainfold/wwwwwww/Fld/SubFld/src
     * [ 3] datStr.cpp                                                      edit   /home/usr/work/mainfold/wwwwwww/Fld/SubFld/src/datStr.cpp
     * [ 4]
     * [ 5] $/Proje/mainfold/wwwwwww/Customs/common
     * [ 6] json.h                                                          edit   /home/usr/work/mainfold/wwwwwww/Customs/common/json.h
     * [ 7] $/Proje/mainfold/wwwwwww/Tmp/FldDebug/src
     * [ 8] CFldDebug.cpp                                                   edit   /home/usr/work/mainfold/wwwwwww/Tmp/FldDebug/src/CFldDebug.cpp
     * [ 9] CFldDebug.h                                                     edit   /home/usr/work/mainfold/wwwwwww/Tmp/FldDebug/src/CFldDebug.h
     * [10] -------------------------------------------------------------------------------
     * [11] Detected Changes:
     * [12] -------------------------------------------------------------------------------
     * [13] File name                                                     Change             Local path
     * [14] ------------------------------------------------------------- ------------------ -
     * [15] $/Proje/mainfold/wwwwwww/Tmp/TmpCoreCustom/build/5_15_3-Debug
     * [16] .qmake.stash                                                  add                /home/usr/work/mainfold/wwwwwww/Sch/TmpCoreCustom/build/5_15_3-Debug/.qmake.stash
     * [17] Makefile                                                      add                /home/usr/work/mainfold/wwwwwww/Sch/TmpCoreCustom/build/5_15_3-Debug/Makefile
     * [18] $/Proje/mainfold/wwwwwww/Builstation
     * [19] linux_symlinks.sh                                             add, property (+x) /home/usr/work/mainfold/wwwwwww/Builstation/linux_symlinks.sh
     * [20] 15 change(s), 7 detected change(s)
     */

    if( items.count() < 2 ) {
        return {}; // В ответе на status должно быть минимум 2 строки
    }

    int idxDetected = items.count() ;
    for( int idx = 0; idx < items.count(); idx++ ) {
        const QString& item = items.at(idx);
        if (item.contains("Detected Changes:")) {
            idxDetected = idx;
            break;
        }
    }

    QStringList preparedFilesInfo;
    for( int idx = 0; idx < idxDetected; idx++ ) {
        const QString& item = items.at(idx);
        if( item.contains(dirPath) ) {
            preparedFilesInfo.append( items[idx] );
        }
    }

    QString title = items[1];
    QList<QPair<int, int>> positions = parsePositions( title );

    return parseStatus( preparedFilesInfo, positions );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение информации об обнаруженных изменениях из команды status
 * \param items Ответ команды
 * \param dirPath Путь к каталогу сопоставленному каталогу, в которого запрашивался статус
 * \return Список изменений
 */
QList<StatusItem> parseStatusDetected( const QStringList& items, const QString& dirPath ) {

    /* Пример вывода команды
     * [ 0] File name                                                       Change Local path
     * [ 1] --------------------------------------------------------------- ------ --------
     * [ 2] $/Proje/mainfold/wwwwwww/Fld/SubFld/src
     * [ 3] datStr.cpp                                                      edit   /home/usr/work/mainfold/wwwwwww/Fld/SubFld/src/datStr.cpp
     * [ 4]
     * [ 5] $/Proje/mainfold/wwwwwww/Customs/common
     * [ 6] json.h                                                          edit   /home/usr/work/mainfold/wwwwwww/Customs/common/json.h
     * [ 7] $/Proje/mainfold/wwwwwww/Tmp/FldDebug/src
     * [ 8] CFldDebug.cpp                                                   edit   /home/usr/work/mainfold/wwwwwww/Tmp/FldDebug/src/CFldDebug.cpp
     * [ 9] CFldDebug.h                                                     edit   /home/usr/work/mainfold/wwwwwww/Tmp/FldDebug/src/CFldDebug.h
     * [10] -------------------------------------------------------------------------------
     * [11] Detected Changes:
     * [12] -------------------------------------------------------------------------------
     * [13] File name                                                     Change             Local path
     * [14] ------------------------------------------------------------- ------------------ -
     * [15] $/Proje/mainfold/wwwwwww/Tmp/TmpCoreCustom/build/5_15_3-Debug
     * [16] .qmake.stash                                                  add                /home/usr/work/mainfold/wwwwwww/Sch/TmpCoreCustom/build/5_15_3-Debug/.qmake.stash
     * [17] Makefile                                                      add                /home/usr/work/mainfold/wwwwwww/Sch/TmpCoreCustom/build/5_15_3-Debug/Makefile
     * [18] $/Proje/mainfold/wwwwwww/Builstation
     * [19] linux_symlinks.sh                                             add, property (+x) /home/usr/work/mainfold/wwwwwww/Builstation/linux_symlinks.sh
     * [20] 15 change(s), 7 detected change(s)
     */

    if( items.count() < 3 ) {
        return {}; // В ответе на status должно быть минимум 3 строки
    }

    int idxDetected = items.count() ;
    for( int idx = 0; idx < items.count(); idx++ ) {
        const QString& item = items.at(idx);
        if (item.contains("Detected Changes:")) {
            idxDetected = idx;
            break;
        }
    }

    QStringList detectedFilesInfo;
    for( int idx = idxDetected + 1; idx < items.count(); idx++ ) {
        const QString& item = items.at(idx);
        if( item.contains(dirPath) ) {
            detectedFilesInfo.append( items[idx] );
        }
    }

    QString title = items[idxDetected + 3];
    QList<QPair<int, int>> positions = parsePositions( title );
    return parseStatus( detectedFilesInfo, positions );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение информации об изменениях из предобработанного списка
 * \param items Предобработанный список элементов
 * \param positions Позиции заголовков
 * \return Информация об изменениях в виде структурированног списка
 *
 * Это вспомогательный метод и он не должен вызываться напрямую.
 * Вместо это вызывать нужно:
 * \sa parseStatusPrepared()
 * \sa parseStatusDetected()
 */
QList<StatusItem> parseStatus( const QStringList& items, const QList<QPair<int, int>>& positions ) {

    if( positions.count() != 3 ) {
        return {}; // Должно быть 3 загововка
    }

    QList<StatusItem> statusItems;

    QPair<int, int> posFile   = positions[0];
    QPair<int, int> posStatus = positions[1];
    QPair<int, int> posPath   = positions[2];

    foreach( const QString& fileInfo, items ) {

        int status = StatusNone;
        { // Вычисление статусы
            QString statusInfo = fileInfo.mid(posStatus.first, posStatus.second).trimmed();
            if(statusInfo.contains("add")) {
                status |= StatusNew;
            }
            if(statusInfo.contains("edit")) {
                status |= StatusEdit;
            }
            if(statusInfo.contains("del") || statusInfo.contains("remove")) {
                status |= StatusDelete;
            }
            if(statusInfo.contains("rename")) {
                status |= StatusRename;
            }
        }

        // Для последнего столбца заголовок не доходит до конца строки,
        // поэтому пересчитываем для каждой строки отдельно
        posPath.second = fileInfo.length() - posPath.first;

        StatusItem item;
        item.name   = fileInfo.mid(posFile.first, posFile.second).trimmed();
        item.status = status;
        item.path   = fileInfo.mid(posPath.first, posPath.second).trimmed();

        statusItems.append( item );
    }

    return statusItems;
}
//-----------------------------7-----------------------------------------------------------------------------

/*!
 * \brief Получение позиций столбцов
 * \param title Заголовок (отчерк)
 * \return Список позиций в формате: <позиция,длина>
 */
QList<QPair<int, int>> parsePositions( const QString& title ) {

    /* Пример вывода команды (title)
     * --------- --------------------------- -------- --------------------------------
     */

    /* Как происходит разбор
     * --------- --------------------------- -------- --------------------------------
     * ^         ^                           ^        ^
     * 0         10                          38       47
     */

    /* Пример возвращаемого значения:
     * {
     *  <0 , 9>,
     *  <10,27>,
     *  <38, 8>,
     *  <47,32>,
     * }
     */

    QList<QPair<int, int>> positions;

    QStringList parts = title.split( " ", Qt::SkipEmptyParts );
    int posStart = 0;
    for( int i = 0; i < parts.count(); i++ ) {
        int len = parts[i].length();
        positions.append( QPair<int, int>(posStart, len) );
        posStart += len + 1;
    }

    return positions;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение имени проекта из пути
 * \param path Путь
 * \return Имя проекта
 */
QString projectName( const QString& path ) {

    /* Пример path
     *
     */

    int idx = path.lastIndexOf('/');
    if( idx == -1 ) {
        return path;
    }

    idx++;
    return path.mid( idx, path.length() - idx );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Разделение полного пути на путь к каталогу и имя файла
 * \param[in] path Полный путь
 * \param[out] folder Путь к каталогу
 * \param[out] file Имя файла
 */
void splitPath( const QString& path, QString& folder, QString& file ) {

    int idx_sep = path.lastIndexOf('/');
    folder = path.mid( 0, idx_sep );
    file   = path.mid( idx_sep + 1, path.length() - idx_sep );

    // Для удаленных файлов tf возвращает элементы в виде:
    // file.ext;X111111
    // Убираем часть ';X111111'
    int idx = file.indexOf(";");
    if( idx >= 0 ) {
        file = file.mid( 0, idx );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение иконки по имени файла
 * \param name Имя
 * \return Иконка
 */
QPixmap icon( const QString& name , int fileType ) {

    if( fileType == TypeFolder ) {
        return QPixmap(":/folder.png");
    }

    QString ext;
    int lastPoint = name.lastIndexOf(".");
    if( lastPoint >= 0 && lastPoint < name.length() ) {
        ext = name.right( name.length() - lastPoint );
    }

    QTemporaryFile* tmpFile = new QTemporaryFile();
    tmpFile->setFileTemplate( QString("XXXXXX%1").arg(ext) );
    tmpFile->open();
    tmpFile->close();

    const QSize imageSize( 64, 64 );

    QFileIconProvider* IconProvider = new QFileIconProvider();
    QPixmap image = IconProvider->icon(tmpFile->fileName()).pixmap(imageSize);
    delete IconProvider;

    if( image.isNull() ) {
        image = qApp->style()->standardIcon(QStyle::SP_FileIcon).pixmap(imageSize);
    }

    return image;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение сдвоенной иконки
 * \param fileName Имя файла
 * \param iconPath Путь к иконке
 * \return Сдвоенную иконку
 */
QPixmap joinIconsFile( const QString& fileName, const QString& iconPath ) {

    int spacing = 20;
    QSize iconSize = QSize( 64, 64 );

    QPixmap pixmapLeft  = QPixmap(iconPath).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap pixmapRight = icon(fileName, TypeFile);
    QPixmap pixmapMix  ( iconSize.width() * 2 + spacing, iconSize.height() );

    pixmapMix.fill( Qt::transparent );

    QPainter painter( &pixmapMix );
    painter.drawPixmap( 0, 0, pixmapLeft );
    painter.drawPixmap( pixmapLeft.width() + spacing, 0, pixmapRight );

    return pixmapMix;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Путь к рабочему каталогу
 * \return Путь
 */
QString workDirPath() {

    return QString("%1/%2").arg(QDir::homePath(), workDirName());
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Имя рабочего каталога
 * \return Имя
 */
QString workDirName() {
    return ".applettf";
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание рабочего каталога
 */
void createWorkDir() {

    QDir workDir(workDirPath());
    if( !workDir.exists() ) {
        QDir::home().mkdir( workDirName() );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание структуры каталогов
 * \param parent Указатель на родительский узел
 * \param role Роль, по которой можно получить у родителя полный путь и по которой нужно установить путь для нового узла
 * \param pathDirs Список каталогов
 * \param idxFrom Индекс, с которого нужно начинать просмотривать каталог
 */
void createTreeSubDirs( QTreeWidgetItem* parent, int role, QStringList& pathDirs, int idxFrom ) {

    for( int idx = idxFrom; idx < pathDirs.count(); idx++ ) {

        QString pathDir = pathDirs[idx];

        QString parentPath = parent->data(0, role).toString();
        if( !pathDir.startsWith(parentPath) ) {
            continue;
        }

        QTreeWidgetItem* item = new QTreeWidgetItem( parent );
        item->setData( 0, role, pathDir );
        item->setText( 0, pathDir.remove(parentPath + "/") );

        pathDirs.removeAt( idx );
        createTreeSubDirs( item, role, pathDirs, idx );
        idx--;
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение списка путей к файлу, включая дочерние узлы
 * \param[in] item Элемент дерева
 * \param[in] role Роль, по которой можно получить путь
 * \param[out] collection Коллекция для записи путей
 */
void collectPathFiles( const QTreeWidgetItem* item, int role, QStringList& collection ) {

    if( item->data(0, TypeRole).toInt() == TypeFile ) {
        collection.append( item->data(0, role).toString() );
    }

    for( int idx = 0; idx < item->childCount(); idx++ ) {

        QTreeWidgetItem* childItem = item->child( idx );
        if( childItem->data(0, TypeRole).toInt() == TypeFile ) {
            collection.append( childItem->data(0, role).toString() );
        }

        collectPathFiles( childItem, role, collection );
    }

    collection.removeDuplicates();
}
//----------------------------------------------------------------------------------------------------------
