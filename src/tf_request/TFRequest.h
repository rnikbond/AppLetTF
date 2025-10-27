//----------------------------------------
#ifndef TFREQUEST_H
#define TFREQUEST_H
//----------------------------------------
#include <QObject>
#include <QProcess>
//----------------------------------------
#include "Config.h"
#include "ChangesetCache.h"
//----------------------------------------

class TFRequest : public QObject {

    Q_OBJECT

    QProcess  *     m_tf     ;
    QTextCodec*     m_codec  ;
    Config          m_config ;
    ChangesetCache* m_cache  ;
    bool            m_isAsync; /// признак асинхронного выполнения запроса

public:

    enum Commands {
        CmdNone        , ///< Отсутствие команды
        CmdWorkspaces  , ///< Команда "workspaces"
        CmdStatus      , ///< Команда "status"
        CmdHistory     , ///< Команда "history"
        CmdGetLastest  , ///< Команда "get"
        CmdGetVersion  , ///< Команда "get -version"
    };

public:

    Commands m_cmd          ; /// Последняя выполненная команда
    bool     m_isTouchCursor; /// Признак изменения курсора во время выполнения команды

    bool        m_isErr   ; /// Признак ошибки выполнения запроса
    int         m_errCode ; /// Код ошибки
    QString     m_errText ; /// Текст ошибки
    QStringList m_response; /// Ответ TF

public:

    TFRequest( QObject* parent = nullptr );

    void setAsync( bool isAsync );
    void setConfig( const Config& cfg );
    void setCache ( ChangesetCache* cache );
    void clear();

public: // Requests

    void checkConnection();

    void workspaces();
    void createWorkspace( const QString& name, const QString& comment );
    void removeWorkspace( const QString& name );

    void workfolds( const QString& workspace = "" );
    void mapWorkfold( const QString& azurePath, const QString& localPath, const QString& workspace = "" );

    void getDir( const QString& dir, const QString& version = "", bool isForce = false );
    void entriesDir( const QString& dir, bool isFiles = true );

    void view( const QString& file, const QString& version = "" );

    void status( const QString& path );
    void add( const QStringList& files );
    void remove( const QStringList& files );
    void cancelChanges( const QStringList& pathes );
    void commit( const QString& comment, const QStringList& files );

    void history( const QString& path, const QString& from = "T", int stopAfter = 50 );
    void historyCertain( const QString& path, const QString& version );
    void historyDiffPrev( const QString& path, const QString& version );

    void difference( const QString& file );
    void difference( const QString& file, const QString& version, const QString& versionOther );

private:

    void reactOnStateChanged( QProcess::ProcessState state );
    void reactOnFinished( int exitCode, QProcess::ExitStatus exitStatus );
    void reactOnErrorOccurred( QProcess::ProcessError error );

    void execute( const QStringList& args );
    void parseResponse();

    void updateCache();

signals:

    void executed();
};
//----------------------------------------------------------------------------------------------------------

#endif // TFREQUEST_H
