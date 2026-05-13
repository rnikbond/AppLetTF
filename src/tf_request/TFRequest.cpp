//----------------------------------------
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QProgressBar>
#include <QTextCodec>
#include <QTimer>
#include <QVBoxLayout>
//----------------------------------------
#include "TFRequest.h"
#include "qboxlayout.h"
#include "qnamespace.h"
//----------------------------------------

class Loader {

public:

    QWidget* overlay;
    QLabel* label;
    QProgressBar* progressBar;

public:

    Loader() {
        QWidget* mainWindow = nullptr;
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (widget->isWindow() && widget->isVisible()) {
                mainWindow = widget;
                break;
            }
        }

        overlay     = new QWidget(mainWindow);
        label       = new QLabel(overlay);
        progressBar = new QProgressBar(overlay);

        QVBoxLayout* mainVLayout = new QVBoxLayout(overlay);
        mainVLayout->addWidget(label);
        mainVLayout->addWidget(progressBar);

        overlay->setFixedSize(350, 60);

        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("QLabel { color: #222222; background: transparent; }");

        progressBar->setRange(0, 0);                // Значения 0, 0 включают бесконечный режим анимации
        progressBar->setAlignment(Qt::AlignCenter); // Текст ровно по центру бара
        progressBar->setTextVisible(true);
        progressBar->setFormat("Прошло: 0 сек"); // Стартовый

        progressBar->setStyleSheet("QProgressBar {"
                                   "   border: 1px solid rgba(255, 255, 255, 0.12);"
                                   "   border-radius: 3px;"                       // Небольшое закругление корпуса
                                   "   background-color: rgba(30, 30, 30, 0.85);" // Глубокий темный фон
                                   "   text-align: center;"
                                   "   color: #FFFFFF;" // Белый текст поверх
                                   "   font-family: 'Segoe UI', Arial, sans-serif;"
                                   "   font-size: 12px;"
                                   "   font-weight: 600;"
                                   "}"
                                   "QProgressBar::chunk { "
                                   "   background-color: qlineargradient(" // Плавный зеленый градиент
                                   "       x1: 0, y1: 0, x2: 1, y2: 0,"
                                   "       stop: 0 #22c55e, stop: 1 #4ade80" // От насыщенного к светло-зеленому
                                   "   );"
                                   "   border-radius: 3px;" // Закругление самой полосы
                                   "}");

        if (mainWindow) {
            overlay->move(mainWindow->rect().center() - overlay->rect().center());
        }
    }

    ~Loader() {
        delete overlay;
    }
};
//----------------------------------------------------------------------------------------------------------

TFRequest::TFRequest( QObject* parent ) : QObject(parent) {

#ifdef WIN32
    m_codec = QTextCodec::codecForName("cp1251");
#else
    m_codec = QTextCodec::codecForName("utf8");
#endif

    m_cmd           = CmdNone;
    m_isTouchCursor = true;

    m_tf = new QProcess( this );

    clear();
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

    execute(args);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Загрузка рабочих пространств
 */
void TFRequest::workspaces() {

    m_cmd = CmdWorkspaces;

    QStringList args = {
        "workspaces",
         QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
         QString("-collection:%1").arg(m_config.m_azure.url),
        "-noprompt"
    };

    execute(args);
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

    execute(args);
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

    execute(args);
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

    execute(args);
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

    execute(args);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Загрузка каталога
 * \param dir     Путь к каталогу
 * \param version Набор изменений (версия)
 * \param isForce Признак принудительной перезаписи
 *
 * Если \a version не указан, загружается последняя версия.
 */
void TFRequest::getDir( const QString& dir, const QString& version , bool isForce ) {

    m_cmd = version.isEmpty() ? CmdGetLastest : CmdGetVersion;

    QStringList args = { "get",
        dir,
        "-recursive",
        "-overwrite",
        "-noprompt",
        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
    };

    if( isForce ) {
        args.append( "-force" );
    }

    if( !version.isEmpty() ) {
        args.append( QString("-version:%1").arg(version) );
    }

    execute(args);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение состояния изменений элемента
 * \param path Путь к элементу
 */
void TFRequest::status( const QString& path ) {

    m_cmd = CmdStatus;

    QStringList args = {
        "status",
        "-recursive",
        path
    };

    execute(args);
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

    execute(args);
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

    execute(args);
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

    execute(args);
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

    execute(args);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение содержимого каталога
 * \param dir Каталог, у которого нужно получить содержимое
 * \param isFiles Признак загрузки файлов
 */
void TFRequest::entriesDir( const QString& dir, bool isFiles ) {

    QStringList args = { "dir",
                        QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
                        QString("-collection:%1").arg(m_config.m_azure.url),
                        dir };

    if( !isFiles ) {
        args.append( "-folders" );
    }

    execute(args);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Просмотр файла на сервере
 * \param file Путь к файлу
 */
void TFRequest::view( const QString& file, const QString& version ) {

    QStringList args = { "view",
                        // QString("-login:%1,%2").arg(m_config.m_azure.login, m_config.m_azure.password),
                        // QString("-collection:%1").arg(m_config.m_azure.url),
                        file };

    if( !version.isEmpty() ) {
        args.append( QString("-version:C%1").arg(version) );
    }

    execute(args);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение журнала изменений
 * \param path      Путь к элементу, у которого нужно получить журнал изменений
 * \param from      С какого набора изменений начать загрузку. По-умолчанию "T" - последний набор изменений.
 * \param stopAfter Сколько наборов изменений загржать. По-умолчанию 50.
 */
void TFRequest::history( const QString& path , const QString& from, int stopAfter ) {

    m_cmd = CmdHistory;
    qDebug() << "history: " << path;

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
        //QString("-collection:%1").arg(m_config.m_azure.url),
        //"-noprompt",
        QString("-version:%1"  ).arg(from),
        QString("-stopafter:%1").arg(stopAfter),
        path
    };

    execute(args);
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

    execute(args);
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

    execute(args);
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

    execute(args);
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

    execute(args);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Выполнение TF запроса с ожиданием завершения
 * \param args        Аргументы запроса
 * \param timeout_sec Таймаут выполнения команды в секундах
 */
void TFRequest::execute(const QStringList& args) {

    clear();

    if (m_isTouchCursor) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    QEventLoop eventLoop;

    //: Связываем завершение процесса (или ошибку) с выходом из цикла событий
    connect(m_tf, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), &eventLoop, &QEventLoop::quit);
    connect(m_tf, &QProcess::errorOccurred, &eventLoop, &QEventLoop::quit);

    Loader loader;

    //: Индикация выполнения процесса
    QTimer timer_loader;
    int elapsedSeconds = 0;
    QObject::connect(&timer_loader, &QTimer::timeout, [&loader, &elapsedSeconds]() {
        elapsedSeconds++;

        //: На 5-й секуне отображаем инликацию
        if (elapsedSeconds == 5) {
            loader.overlay->show();
            loader.overlay->raise();
        }

        QString fmt = (elapsedSeconds < 60) ? "s сек" : "m мин ss сек";
        QTime time  = QTime::fromMSecsSinceStartOfDay(elapsedSeconds * 1000);
        loader.label->setText(tr("Выполняется: %1").arg(time.toString(fmt)));
    });
    timer_loader.start(1000);

    m_tf->start(m_config.m_azure.tfPath, args);

    //: Ждем выполнения процесса
    eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    parseResponse();

    if (m_isTouchCursor) {
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

