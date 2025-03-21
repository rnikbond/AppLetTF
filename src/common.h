//----------------------------------------
#ifndef COMMON_H
#define COMMON_H
//----------------------------------------
#include <QList>
#include <QString>
//----------------------------------------

/// Типы файлоы
enum FileTypes {
    TypeNone, ///< Неизвестный тип
    TypeFile    , ///< Файл
    TypeFolder  , ///< Директория
};
//----------------------------------------

/// Статусы
enum Statuses {
    StatusNone   = 0x0, ///< Неизвестный статус
    StatusNew    = 0x1, ///< Новый
    StatusEdit   = 0x2, ///< Измененный
    StatusDelete = 0x4, ///< Удаленный
    StatusRename = 0x8, ///< Переименованный
};
//----------------------------------------

/// Пользовательские роли для модели
enum CustomRoles {
    AzurePathRole = Qt::UserRole + 1, ///< Путь к элементу в Azure DevOps Server
    LocalPathRole                   , ///< Локальный путь к элементу
    StatusRole                      , ///< Статус (Новый, измененный, удаленный)
    TypeRole                        , ///< Тип элемента (Каталог, файл)
    LoadedRole                      , ///< Признак загруженных данных
    CustomRole                      , ///< Пользовательская роль
};
//----------------------------------------

/*!
 * \brief Структура элемента сопоставления каталогов
 */
struct WorkfoldItem {
    QString pathServer; /// Путь к каталогу на сервер
    QString pathLocal ; /// Путь к локальному каталогу
};
//----------------------------------------

/*!
 * \brief Структура "Рабочее пространства" (Workspace)
 */
struct Workspace {
    QString             name     ; /// Имя рабочего пространства
    QString             owner    ; /// Владелец
    QString             computer ; /// Имя ПК
    QString             comment  ; /// Комментарий
    QList<WorkfoldItem> workfolds; /// Список сопоставления
};
//----------------------------------------

/*!
 * \brief Структура элемента дерева Azure DevOps Server
 */
struct AzureItem {
    QString   name  ; // Путь к элемента
    QString   folder; // Путь к элемента
    FileTypes type  ; // Тип элемента
};
//----------------------------------------

/*!
 * \brief Структура элемента из журнала изменений
 */
struct HistoryItem {
    QString version ; /// Версия (набор изменений)
    QString author  ; /// Автор
    QString datetime; /// Дата и время
    QString comment ; /// Комментарий
};
//----------------------------------------

/*!
 * \brief Структура с информацией о файле из журнала изменений
 */
struct HistoryFile {
    QString status;
    QString file;
};
//----------------------------------------

/*!
 * \brief Структура подробной информации из журнала изменений
 */
struct HistoryDetailItem {
    QString version ; /// Версия (набор изменений)
    QString author  ; /// Автор
    QString datetime; /// Дата и время
    QString comment ; /// Комментарий
    QList<HistoryFile> files;
};
//----------------------------------------

#endif // COMMON_H
