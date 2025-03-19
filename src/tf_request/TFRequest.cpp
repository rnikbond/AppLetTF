//----------------------------------------
#include <QDir>
#include <QDebug>
#include <QTextCodec>
#include <QApplication>
//----------------------------------------
#include "methods.h"
//----------------------------------------
#include "TFRequest.h"
//----------------------------------------

TFRequest::TFRequest( QObject* parent ) : QObject(parent) {

#ifdef WIN32
    m_codec = QTextCodec::codecForName("cp1251");
#else
    m_codec = QTextCodec::codecForName("utf8");
#endif

    m_isTouchCursor = true;

    m_tf = new QProcess( this );

    clear();
    setAsync( false );

    connect( m_tf, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished)              , this, &TFRequest::reactOnFinished      );
    connect( m_tf,                                         &QProcess::stateChanged           , this, &TFRequest::reactOnStateChanged  );
    connect( m_tf,                                         &QProcess::errorOccurred          , this, &TFRequest::reactOnErrorOccurred );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Проверка подключения к Azure DevOps Server
 */
void TFRequest::checkConnection() {

    QStringList args = {
        "workfold",
        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
        QString("-collection:%1").arg(m_config.m_azure.url),
        QString("-workspace:%1" ).arg(m_config.m_azure.workspace),
        "-noprompt"
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Загрузка рабочих пространств
 */
void TFRequest::workspaces() {

    m_cmd = CommandWorkspaces;

    QStringList args = {
        "workspaces",
         QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
         QString("-collection:%1").arg(m_config.m_azure.url),
        "-noprompt"
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Создание новой рабочей области
 * \param name Имя рабочей области
 */
void TFRequest::createWorkspace(const QString& name , const QString &comment ) {

    QStringList args = {
        "workspace",
        "-new",
        name,
        QString("-comment:%1").arg(comment),
        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
        QString("-collection:%1").arg(m_config.m_azure.url),
        "-noprompt"
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Удаление новой рабочей области
 * \param name Имя рабочей области
 */
void TFRequest::removeWorkspace( const QString& name ) {

    QStringList args = {
        "workspace",
        "-delete",
        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
        QString("-collection:%1").arg(m_config.m_azure.url),
        name,
        "-noprompt"
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение списка сопоставленных каталогов
 * \param Рабочее пространство
 *
 * Если \a workspace пустое, используется рабочее пространство из конфигурации
 */
void TFRequest::workfolds( const QString& workspace ) {

    QStringList args = {
        "workfold",
        QString("-workspace:%1" ).arg(workspace.isEmpty() ? m_config.m_azure.workspace : workspace),
        QString("-collection:%1").arg(m_config.m_azure.url),
        QString("-login:%1,%2"  ).arg(m_config.m_azure.login, m_config.m_azure.password),
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сопоставление каталога
 * \param azurePath Путь к каталогу на сервере
 * \param localPath Путь к локальному каталогу
 * \param workspace Имя рабочей области
 *
 * Если \a workspace пустое, используется рабочее пространство из конфигурации
 */
void TFRequest::mapWorkfold( const QString& azurePath, const QString& localPath, const QString& workspace ) {

    QStringList args = {
        "workfold",
        QString("-collection:%1").arg(m_config.m_azure.url),
        QString("-workspace:%1" ).arg(workspace.isEmpty() ? m_config.m_azure.workspace : workspace),
        azurePath,
        localPath
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Загрузка каталога
 * \param dir Путь к каталогу
 * \param version Набор изменений (версия)
 *
 * Если \a version не указан, загружается последняя версия.
 */
void TFRequest::getDir( const QString& dir, const QString& version ) {

    QStringList args = { "get",
        dir,
        "-recursive",
        "-overwrite",
        "-force",
        "-noprompt",
        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
    };

    if( !version.isEmpty() ) {
        args.append( QString("-varsion:").arg(version) );
    }

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение состояния изменений элемента
 * \param path Путь к элементу
 */
void TFRequest::status( const QString& path ) {

    QStringList args = {
        "status",
        "-recursive",
        path
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Добавление новых элементов
 * \param files Список элементов для добавления
 */
void TFRequest::add( const QStringList& files ) {

    QStringList args = {
        "add",
    };

    args.append( files );

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Удаление элементов
 * \param files Список элементов для удаление
 */
void TFRequest::remove( const QStringList& files ) {

    QStringList args = {
        "delete",
    };

    args.append( files );

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отмена изменений
 * \param pathes Элементы для отмены изменений
 */
void TFRequest::cancelChanges( const QStringList& pathes ) {

    QStringList args = {
        "undo",
    };

    args.append( pathes );

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Фиксация изменений
 * \param comment Комментарий
 * \param files Список элементов для фиксации
 */
void TFRequest::commit( const QString& comment, const QStringList& files ) {

    QStringList args = {
        "checkin",
        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
        QString("-comment:%1").arg(comment)
    };

    if( !files.isEmpty() ) {
        args.append( files );
    }

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение содержимого каталога
 * \param dir Каталог, у которого нужно получить содержимое
 * \param isFiles Признак загрузки файлов
 */
void TFRequest::entriesDir( const QString& dir , bool isFiles ) {

    QStringList args = { "dir",
                        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
                        QString("-collection:%1").arg(m_config.m_azure.url),
                        dir };

    if( isFiles ) {
        args.append( "-folders" );
    }

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение журнала изменений
 * \param path Путь к элементу, у которого нужно получить журнал изменений
 */
void TFRequest::history( const QString& path ) {

    // tf hist[ory]                         -
    // itemspec                             -
    // -version:versionspec                 -
    // -stopafter:number]                   -
    // -recursive                           -
    // -user:username                       -
    // -format:(brief|detailed)             -
    // -slotmode                            -
    // -itemmode                            -
    // -noprompt                            -
    // -login:username,password             -
    // -sort:ascending|descending           -
    // -collection:TeamProjectCollectionUrl -

    QStringList args = {
        "history",
        "-recursive",
        QString("-collection:%1").arg(m_config.m_azure.url),
        "-noprompt",
        path
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение детальной информации о наборе изменений
 * \param path Путь к элементу, у которого нужно получить журнал изменений
 * \param version Набор изменений (версия)
 */
void TFRequest::historyCertain( const QString& path, const QString& version ) {

    // tf hist[ory]                         -
    // itemspec                             -
    // -version:versionspec                 -
    // -stopafter:number]                   -
    // -recursive                           -
    // -user:username                       -
    // -format:(brief|detailed)             -
    // -slotmode                            -
    // -itemmode                            -
    // -noprompt                            -
    // -login:username,password             -
    // -sort:ascending|descending           -
    // -collection:TeamProjectCollectionUrl -

    QStringList args = {
        "history",
        path,
        QString("-collection:%1").arg(m_config.m_azure.url),
        QString("-version:C%1").arg(version),
        "-format:detailed",
        "-recursive",
        "-stopafter:1",
        "-slotmode",
        "-noprompt",
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение предыдущей версии относительно выбранной версии
 * \param path Путь к элементу, который нужно сравнить
 * \param version Наборе изменений (версия), относительно которого нужно получить предыдущий набор
 */
void TFRequest::historyDiffPrev( const QString& path, const QString& version ) {

    // tf hist[ory]                         -
    // itemspec                             -
    // -version:versionspec                 -
    // -stopafter:number]                   -
    // -recursive                           -
    // -user:username                       -
    // -format:(brief|detailed)             -
    // -slotmode                            -
    // -itemmode                            -
    // -noprompt                            -
    // -login:username,password             -
    // -sort:ascending|descending           -
    // -collection:TeamProjectCollectionUrl -

    QStringList args = {
        "history",
        path,
        QString("-collection:%1").arg(m_config.m_azure.url),
        QString("-version:C%1").arg(version),
        "-format:brief",
        "-recursive",
        "-stopafter:2",
        "-noprompt",
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сравнение файла на сервере с локальной версией
 * \param file Файл, который нужно сравнить
 */
void TFRequest::difference( const QString& file ) {

    QStringList args = {
        "diff",
        file
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сравнение разный версий файла
 * \param file Файл, который нужно сравнить
 * \param version Версия
 * \param versionOther Другая версия
 *
 * Пример аргументов:
 * file        : $/Project/file.txt
 * version     : 123
 * versionOther: 122
 */
void TFRequest::difference( const QString& file, const QString& version, const QString& versionOther ) {

    QStringList args = {
        "diff",
        QString("%1;C%2").arg(file, versionOther),
        QString("%1;C%2").arg(file, version),
    };

    if( m_isAsync ) {
        m_tf->start( m_config.m_azure.tfPath, args );
    } else {
        execute( args );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка изменения состояния процесса программы TF
 * \param state Состояние
 */
void TFRequest::reactOnStateChanged( QProcess::ProcessState state ) {

    switch( state ) {
        case QProcess::Starting: {
            clear();
            if( m_isTouchCursor ) {
                QApplication::setOverrideCursor( Qt::WaitCursor );
            }
            break;
        }
        default: break;
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка данных после завершения работы программы TF
 * \param exitCode   Код завершения работы программы
 * \param exitStatus Статус завершения работы программы
 */
void TFRequest::reactOnFinished( int exitCode, QProcess::ExitStatus exitStatus ) {

    Q_UNUSED( exitCode   );
    Q_UNUSED( exitStatus );

    parseResponse();
    emit executed();

    if( m_isTouchCursor ) {
        QApplication::restoreOverrideCursor();
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Обработка ошибки запуска процесса
 * \param error Информация об ошибке
 *
 * Возникает, если не удается запустить программу TF.
 * При вызове этого слота, слот \a reactOnFinished() не вызывается.
 */
void TFRequest::reactOnErrorOccurred( QProcess::ProcessError error ) {

    Q_UNUSED( error );

    parseResponse();
    emit executed();

    if( m_isTouchCursor ) {
        QApplication::restoreOverrideCursor();
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Выполнение TF запроса с ожиданием завершения
 * \param args Аргументы запроса
 */
void TFRequest::execute( const QStringList& args ) {

    clear();

    if( m_isTouchCursor ) {
        QApplication::setOverrideCursor( Qt::WaitCursor );
    }

    m_tf->start( m_config.m_azure.tfPath, args );
    m_tf->waitForFinished();
    parseResponse();

    if( m_isTouchCursor ) {
        QApplication::restoreOverrideCursor();
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Разбор ответа TF
 *
 * Если процесс TF завершился ошибкой, то параметры ошибки записываются в:
 * \a m_isErr   - устаналивается в true
 * \a m_errCode - код ошибки
 * \a m_errText - описание ошибки
 *
 * Если же процесс завершился успешно, ответ помещается в \a m_response.
 */
void TFRequest::parseResponse() {

    m_errCode = m_tf->error();
    switch( m_errCode ) {
        case QProcess::UnknownError : break;
        case QProcess::FailedToStart: m_isErr = true; m_errText = tr("Не удалось запустить программу TF"); return;
        default                     : m_isErr = true; m_errText = m_tf->errorString();                     return;
    }

    m_errCode = m_tf->exitCode();
    if( m_errCode != 0 ) {
        m_isErr   = true;
        m_errText = m_codec->toUnicode(m_tf->readAllStandardError());
        return;
    }

    m_response = m_codec->toUnicode(m_tf->readAllStandardOutput()).split("\n", Qt::SkipEmptyParts);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение синхронного/асинхронного режима
 * \param isAsync Признак асинхронности
 *
 * Если isAsync == TRUE, после выполнения запроса испускается сигнал \a executed().
 */
void TFRequest::setAsync( bool isAsync ) {

    m_isAsync = isAsync;
    m_tf->blockSignals( !m_isAsync );
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void TFRequest::setConfig( const Config& cfg ) {

    m_config = cfg;

    if( m_config.m_azure.workfoldes.count() > 0 ) {
        QString path = m_config.m_azure.workfoldes.begin().value();
        m_tf->setWorkingDirectory( path );
    }
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Очистка
 */
void TFRequest::clear() {

    m_isErr   = false;
    m_errCode = 0;
    m_errText .clear();
    m_response.clear();
}
//----------------------------------------------------------------------------------------------------------

