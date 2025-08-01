//----------------------------------------
#include <QDir>
#include <QFile>
#include <QRect>
#include <QSettings>
//----------------------------------------
#include "methods.h"
//----------------------------------------
#include "Config.h"
//----------------------------------------
#define CONF_DEFAULT_PATH "applettf.cfg"

#define CONF_VERSION 1
#define CONF_PROP_VERSION "version"

#define CONF_GROUP_WINDOW "window"
    #define CONF_PROP_LOG     "log"
    #define CONF_PROP_TOOLBAR "toolbar"

#define CONF_GROUP_COMMON "common"
    #define CONF_PROP_TRAY "tray"

#define CONF_GROUP_GEOMETRY "geometry"
    #define CONF_PROP_GEOMETRY_X "x"
    #define CONF_PROP_GEOMETRY_Y "y"
    #define CONF_PROP_GEOMETRY_WIDTH  "width"
    #define CONF_PROP_GEOMETRY_HEIGHT "height"

#define CONF_GROUP_AZURE "tfs"
    #define CONF_PROP_TF_PATH         "tf_path"
    #define CONF_PROP_DIFF_CMD        "diff_command"
    #define CONF_PROP_AZURE_URL       "url"
    #define CONF_PROP_AZURE_WORKSPACE "workspace"
    #define CONF_PROP_AZURE_WORKFOLDS "workfolds"
    #define CONF_PROP_AZURE_LOGIN     "login"
    #define CONF_PROP_AZURE_PASSWORD  "password"
//----------------------------------------

Config::Config() { }
//----------------------------------------------------------------------------------------------------------

Config::Config( const Config& other ) {

    m_path      = other.m_path     ;
    m_tray      = other.m_tray     ;
    m_isLog     = other.m_isLog    ;
    m_isToolBar = other.m_isToolBar;
    m_geometry  = other.m_geometry ;

    m_azure.tfPath     = other.m_azure.tfPath    ;
    m_azure.diffCmd    = other.m_azure.diffCmd   ;
    m_azure.url        = other.m_azure.url       ;
    m_azure.workspace  = other.m_azure.workspace ;
    m_azure.workfoldes = other.m_azure.workfoldes;
    m_azure.login      = other.m_azure.login     ;
    m_azure.password   = other.m_azure.password  ;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Инициализация
 * \param path Путь к файлу конфигурации
 *
 * Если \a path пустой, используется путь по-умолчанию: "applettf.cfg"
 */
void Config::init( const QString& path ) {

    m_path = path.isEmpty() ? QString("%1/%2").arg(workDirPath(), CONF_DEFAULT_PATH) : path ;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сохранение конфигурации
 * \param geometry Геометрия основного окна
 */
void Config::save( QRect geometry , bool isLog, bool isMaximize ) {

    QStringList workfolds;
    foreach( const QString& azurePath, m_azure.workfoldes.keys() ) {
        workfolds.append( QString("%1:%2").arg(azurePath, m_azure.workfoldes[azurePath]) );
    }

    QSettings conf(m_path, QSettings::IniFormat);

    conf.setValue( CONF_PROP_VERSION, CONF_VERSION );

    conf.beginGroup( CONF_GROUP_AZURE );
    conf.setValue( CONF_PROP_TF_PATH        , m_azure.tfPath    );
    conf.setValue( CONF_PROP_DIFF_CMD       , m_azure.diffCmd   );
    conf.setValue( CONF_PROP_AZURE_URL      , m_azure.url       );
    conf.setValue( CONF_PROP_AZURE_WORKSPACE, m_azure.workspace );
    conf.setValue( CONF_PROP_AZURE_WORKFOLDS, workfolds         );
    conf.setValue( CONF_PROP_AZURE_LOGIN    , m_azure.login     );
    conf.setValue( CONF_PROP_AZURE_PASSWORD , m_azure.password  );
    conf.endGroup();

    conf.beginGroup( CONF_GROUP_COMMON );
    conf.setValue( CONF_PROP_TRAY, m_tray );
    conf.endGroup();

    conf.beginGroup( CONF_GROUP_WINDOW );
    conf.setValue( CONF_PROP_LOG    , isLog      );
    conf.setValue( CONF_PROP_TOOLBAR, isMaximize );
    conf.endGroup();

    conf.beginGroup( CONF_GROUP_COMMON );
    conf.setValue( CONF_PROP_TRAY, m_tray );
    conf.endGroup();

    if( geometry.isValid() ) {
        conf.beginGroup( CONF_GROUP_GEOMETRY );
        conf.setValue( CONF_PROP_GEOMETRY_X      , geometry.x()      );
        conf.setValue( CONF_PROP_GEOMETRY_Y      , geometry.y()      );
        conf.setValue( CONF_PROP_GEOMETRY_WIDTH  , geometry.width () );
        conf.setValue( CONF_PROP_GEOMETRY_HEIGHT , geometry.height() );
        conf.endGroup();
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Восстановление конфигурации
 */
void Config::restore() {

    if( !QFile(m_path).exists() ) {
        return;
    }

    QSettings conf(m_path, QSettings::IniFormat);

    int version = conf.value( CONF_PROP_VERSION, 0 ).toInt();
    switch( version ) {
        case CONF_VERSION: {
            break;
        }
        default: {
            return;
        }
    }

    QStringList workfolds;

    conf.beginGroup( CONF_GROUP_COMMON );
    m_tray = conf.value(CONF_PROP_TRAY, false).toBool();
    conf.endGroup();

    conf.beginGroup( CONF_GROUP_WINDOW );
    m_isLog     = conf.value(CONF_PROP_LOG    , true).toBool();
    m_isToolBar = conf.value(CONF_PROP_TOOLBAR, true).toBool();
    conf.endGroup();

    conf.beginGroup( CONF_GROUP_GEOMETRY );
    m_geometry.setX     ( conf.value(CONF_PROP_GEOMETRY_X     , -1).toInt() );
    m_geometry.setY     ( conf.value(CONF_PROP_GEOMETRY_Y     , -1).toInt() );
    m_geometry.setWidth ( conf.value(CONF_PROP_GEOMETRY_WIDTH , -1).toInt() );
    m_geometry.setHeight( conf.value(CONF_PROP_GEOMETRY_HEIGHT, -1).toInt() );
    conf.endGroup();

    conf.beginGroup( CONF_GROUP_AZURE );
    m_azure.tfPath    = conf.value(CONF_PROP_TF_PATH         , "").toString();
    m_azure.diffCmd   = conf.value(CONF_PROP_DIFF_CMD        , "").toString();
    m_azure.url       = conf.value(CONF_PROP_AZURE_URL       , "").toString();
    m_azure.workspace = conf.value(CONF_PROP_AZURE_WORKSPACE , "").toString();
    workfolds         = conf.value(CONF_PROP_AZURE_WORKFOLDS , {}).toStringList();
    m_azure.login     = conf.value(CONF_PROP_AZURE_LOGIN     , "").toString();
    m_azure.password  = conf.value(CONF_PROP_AZURE_PASSWORD  , "").toString();
    conf.endGroup();

    foreach( const QString& workfold, workfolds ) {
        QStringList parts = workfold.split(":");
        if( parts.count() != 2 ) {
            continue;
        }

        if( !QDir(parts[1]).exists() ) {
            continue;
        }

        m_azure.workfoldes[parts[0]] = parts[1];
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение признака настроенной конфиграции
 * \return TRUE, если конфигурация настроена. Иначе FALSE.
 */
bool Config::isIncomplete() const {

    if( m_azure.tfPath.isEmpty() || !QFile(m_azure.tfPath).exists() ) {
        return false;
    }

    return ( !m_azure.url      .isEmpty() &&
             !m_azure.workspace.isEmpty() &&
             !m_azure.login    .isEmpty() &&
             !m_azure.password .isEmpty() );
}
//----------------------------------------------------------------------------------------------------------

void Config::operator = ( const Config& other ) {

    m_path      = other.m_path     ;
    m_tray      = other.m_tray     ;
    m_isLog     = other.m_isLog    ;
    m_isToolBar = other.m_isToolBar;
    m_geometry  = other.m_geometry ;

    m_azure.tfPath     = other.m_azure.tfPath    ;
    m_azure.diffCmd    = other.m_azure.diffCmd   ;
    m_azure.url        = other.m_azure.url       ;
    m_azure.workspace  = other.m_azure.workspace ;
    m_azure.workfoldes = other.m_azure.workfoldes;
    m_azure.login      = other.m_azure.login     ;
    m_azure.password   = other.m_azure.password  ;
}
//----------------------------------------------------------------------------------------------------------
