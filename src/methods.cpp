//----------------------------------------
#include "methods.h"
//----------------------------------------

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
 * \brief Получение позиций столбцов
 * \param title Заголовок (отчерк)
 * \return Список позиций в формате: <позиция,длина>
 *
 * Пример \a title: "--------- --------------------------- -------- --------------------------------"
 *                   ^         ^                           ^        ^
 *                   0         10
 * Пример возвращаемого значения:
 * {
 *  <0 ,9 >,
 *  <10,27>,
 *  <,>,
 * }
 */
QList<QPair<int, int>> parsePositions( const QString& title ) {

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

