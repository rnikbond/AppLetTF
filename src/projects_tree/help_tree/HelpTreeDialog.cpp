//----------------------------------------
#include <QDebug>
#include <QTimer>
#include <QScreen>
#include <QMainWindow>
#include <QGuiApplication>
//----------------------------------------
#include "HelpTreeDialog.h"
#include "ui_HelpTreeDialog.h"
//----------------------------------------

HelpTreeDialog::HelpTreeDialog( QWidget* parent ) : QDialog(parent), ui(new Ui::HelpTreeDialog)
{
    ui->setupUi( this );
    setText();
}
//----------------------------------------------------------------------------------------------------------

HelpTreeDialog::~HelpTreeDialog() {
    delete ui;
}
//----------------------------------------------------------------------------------------------------------

void HelpTreeDialog::setText() {

    struct HelpItem {
        QString title, icon, text, cmd;
    };

    QList<HelpItem> items;

    HelpItem itemLast;
    itemLast.title = tr("Получить");
    itemLast.icon  = "<img src=\":/save.png\"width=\"22\" height=\"22\">";
    itemLast.text  = tr("Получение изменений или загрузка элемента, если он ранее не был получен");
    itemLast.cmd   = "";
    items.append( itemLast );

    HelpItem itemRewrite;
    itemRewrite.title = tr("Получить элемент"  );
    itemRewrite.icon  = "<img src=\":/rewrite.png\"width=\"22\" height=\"22\">";
    itemRewrite.text  = tr("Получение элемента заново.<br/>"\
                          "Все ранее загруженные файлы будут перезаписаны.<br/>"\
                          "Эта команда может быть решением, если проект был получен и затем локально удален. "\
                          "В таком случае команда %1 <b>Получить</b> может не сработать.").arg(itemLast.icon);
    itemRewrite.cmd   = "tf get $/[path_dir] -recursive -overwrite -noprompt -login:[login],[pwd]";
    items.append( itemRewrite );

    HelpItem itemVersion;
    itemVersion.title = tr("Получить версию...");
    itemVersion.icon  = "<img src=\":/save.png\"width=\"22\" height=\"22\">";
    itemVersion.text  = tr("Получение определенной версии элемента");
    itemVersion.cmd   = "tf get $/[path_dir] -recursive -overwrite -noprompt -login:<login>,<pwd> -version:C<N>";
    items.append( itemVersion );

    HelpItem itemHistory;
    itemHistory.title = tr("Журнал");
    itemHistory.icon  = "<img src=\":/list.png\"width=\"22\" height=\"22\">";
    itemHistory.text  = tr("Просмотр истории коммитов элемента");
    itemHistory.cmd   = "tf history recursive $/[path_dir]";
    items.append( itemHistory );

    HelpItem itemUpdate;
    itemUpdate.title = tr("Обновить");
    itemUpdate.icon  = "<img src=\":/reload.png\"width=\"22\" height=\"22\">";
    itemUpdate.text  = tr("Перезагрузка данных в дереве. Синхронизация с сервером.");
    itemUpdate.cmd   = "tf dir $/[path_dir] -collection:[url] -login:[login],[pwd] -version:[N]";
    items.append( itemUpdate );

    QString text = "<table cellpadding=10, cellspacing=-1 border=0 width=100% height=100%>";
    foreach( const HelpItem& item, items ) {

        text += QString("<tr>"\
                        "<td valign=middle>%1 <b>%2</b></td>"\
                        "<td width=2%></td>"\
                        "<td valign=middle>%3</td>"\
                        "</tr>"
                        ).arg(item.icon, item.title, item.text);

        if( !item.cmd.isEmpty() ) {
            text += QString("<tr>"\
                            "<td></td>"\
                            "<td></td>"\
                            "<td valign=middle><code>%1</code><td>"\
                            "</tr>").arg(item.cmd);
        }

        text += "<tr></tr>";
    }
    text += "</table>";

    ui->helpText->setText( text );
}
//----------------------------------------------------------------------------------------------------------

void HelpTreeDialog::moveToCenter() {

    QMainWindow* windowApp = nullptr;
    for( QWidget* pWidget : QApplication::topLevelWidgets() ) {
        QMainWindow* pMainWindow = qobject_cast<QMainWindow*>(pWidget);
        if( pMainWindow ) {
            windowApp = pMainWindow;
        }
    }

    if( windowApp == nullptr ) {
        return;
    }

    QRect windowGeometry = windowApp->geometry();
    int pos_x = windowGeometry.x() + ((windowGeometry.width () - width ()) / 2);
    int pos_y = windowGeometry.y() + ((windowGeometry.height() - height()) / 2);

    move( pos_x, pos_y );
}
//----------------------------------------------------------------------------------------------------------

void HelpTreeDialog::showEvent( QShowEvent* event ) {

    QTimer::singleShot( 0, this, &HelpTreeDialog::moveToCenter );
    QDialog::showEvent( event );
}
//----------------------------------------------------------------------------------------------------------

