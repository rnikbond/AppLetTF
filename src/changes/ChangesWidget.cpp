//----------------------------------------
#include <QDir>
#include <QMenu>
#include <QDebug>
#include <QAction>
#include <QSettings>
#include <QTextStream>
#include <QMessageBox>
#include <QAbstractTextDocumentLayout>
//----------------------------------------
#include "methods.h"
#include "TFRequest.h"
//----------------------------------------
#include "ChangesWidget.h"
#include "ui_ChangesWidget.h"
//----------------------------------------
#define CHANGES_FILE ".changes.dat"
//----------------------------------------

ChangesWidget::ChangesWidget( QWidget* parent ) : QWidget(parent), ui(new Ui::ChangesWidget)
{
    ui->setupUi(this);

    setupPrepared();
    setupExcluded();
    setupDetected();

    m_reloadAction = new QAction( QIcon(":/reload.png"), tr("Обновить"), this );
    m_reloadAction->setShortcut( QKeySequence(Qt::Key_F5) );
    m_reloadAction->setToolTip( m_reloadAction->toolTip() + " " + m_reloadAction->shortcut().toString() );

    m_reloadState = false;

#ifdef WIN32
    m_statusesTfsMap = {
        { tr("добавление"), StatusNew    },
        { tr("изменить"  ), StatusEdit   },
        { tr("удалить"   ), StatusDelete },
    };
#else
    m_statusesTfsMap = {
        { tr("add"   ), StatusNew    },
        { tr("edit"  ), StatusEdit   },
        { tr("delete"), StatusDelete },
    };
#endif

    connect( m_reloadAction, &QAction::triggered, this, &ChangesWidget::reload );
}
//----------------------------------------------------------------------------------------------------------

ChangesWidget::~ChangesWidget() {

    delete ui;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Сохранение данных
 */
void ChangesWidget::saveData() {

    QString path = QString("%1/%2").arg(workDirPath(), CHANGES_FILE);
    QFile file(path);
    if( !file.open(QIODevice::WriteOnly) ) {
        return;
    }

    QTextStream out( &file );
    out << m_excluded.join(";");
    file.close();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Восстановление данных
 */
void ChangesWidget::restoreData() {

    QString path = QString("%1/%2").arg(workDirPath(), CHANGES_FILE);
    QFile file(path);
    if( !file.open(QIODevice::ReadOnly) ) {
        return;
    }

    QString data = file.readAll();
    file.close();

    m_excluded = data.split(";");
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение списка действий
 * \return
 */
QList<QAction*> ChangesWidget::actions() const {

    return {
             m_preparedDiffAction  ,
             m_preparedCancelAction,
             m_excludeAction       ,
             m_reloadAction        ,
            };
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Отображение MessageBox с вопросом
 * \param caption Заголовок
 * \param text    Текст
 * \param detail  Детали
 * \return TRUE, если пользователь нажал "Да". Иначе FALSE.
 */
bool ChangesWidget::question( const QString& caption, const QString& text, const QString& detail ) const {

    QMessageBox messageBox( QMessageBox::Question, caption, text, QMessageBox::Yes | QMessageBox::No );
    messageBox.setDefaultButton( QMessageBox::No );

    if( !detail.isEmpty() ) {
        messageBox.setDetailedText(detail);
        // Раскрытие детальной инфомарции
        foreach( QAbstractButton *button, messageBox.buttons() ) {
            if (messageBox.buttonRole(button) == QMessageBox::ActionRole) {
                button->click();
                break;
            }
        }
    }

    return (messageBox.exec() == QMessageBox::Yes);
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Перезагрузка изменений
 */
void ChangesWidget::reload() {

    m_reloadState = true;

    reloadPrepared();
    reloadExcluded();
    reloadDetected();
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Получение признака, были ли перезагружены изменения
 * \return TRUE, если изменения были перезагружены. Иначе FALSE.
 */
bool ChangesWidget::isReloaded() const {

    return m_reloadState;
}
//----------------------------------------------------------------------------------------------------------

/*!
 * \brief Изменение конфигурации
 * \param cfg Новая конфигурация
 */
void ChangesWidget::setConfig( const Config& cfg ) {
    m_config = cfg;
}
//----------------------------------------------------------------------------------------------------------

