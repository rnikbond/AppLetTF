//----------------------------------------
#ifndef COMMON_H
#define COMMON_H
//----------------------------------------
#include <QString>
//----------------------------------------

/*!
 * \brief Структура "Рабочее пространства" (Workspace)
 */
struct Workspace {
    QString name    ; /// Имя рабочего пространства
    QString owner   ; /// Владелец
    QString computer; /// Имя ПК
    QString comment ; /// Комментарий
};
//----------------------------------------

#endif // COMMON_H
