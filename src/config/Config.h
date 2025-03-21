//----------------------------------------
#ifndef CONFIG_H
#define CONFIG_H
//----------------------------------------
#include <QMap>
#include <QRect>
#include <QString>
//----------------------------------------

/*!
 * \brief Структура свойств для работы с Azure DevOps Server
 */
struct AzureDevOps {

    QString               tfPath    ; /// Путь к программе TF
    QString               diffCmd   ; /// Настройка сравнения (TF_DIFF_COMMAND)
    QString               url       ; /// URL сервера (-collection)
    QString               workspace ; /// Текущее рабочее пространство
    QString               login     ; /// Логин
    QString               password  ; /// Пароль
    QMap<QString,QString> workfoldes; /// Сопоставления каталогов
};
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Класс свойств для работы с конфигурацией
 */
class Config {

public:

    QString m_path;

    QRect       m_geometry;
    bool        m_isLog;
    bool        m_isToolBar;
    bool        m_tray;
    AzureDevOps m_azure;

public:

    Config();
    Config( const Config& other );
    void init( const QString& path );

    bool isIncomplete() const;

    void save   ( QRect geometry = QRect(), bool isLog = true, bool isMaximize = true );
    void restore();

public:

    void operator = ( const Config& other );
};
//----------------------------------------------------------------------------------------------------------
#endif // CONFIG_H
