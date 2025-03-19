//----------------------------------------
#ifndef TFREQUEST_H
#define TFREQUEST_H
//----------------------------------------
#include <QObject>
#include <QProcess>
//----------------------------------------
#include "Config.h"
//----------------------------------------

class TFRequest : public QObject {

    Q_OBJECT

    QProcess  * m_tf     ;
    QTextCodec* m_codec  ;
    Config      m_config ;
    bool        m_isAsync; /// признак асинхронного выполнения запроса

public:

    enum Commands {
        CommandNone      , ///< Отсутствие команды
        CommandWorkspaces, ///< Команда "workspaces"
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
    void clear();

public: // Requests

    void checkConnection();

    void workspaces();
    void createWorkspace( const QString& name );

    void entriesDir( const QString& dir );

    void history( const QString& path );
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

signals:

    void executed();
};
//----------------------------------------------------------------------------------------------------------

#endif // TFREQUEST_H
