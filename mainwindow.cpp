#include <QAxWidget>
#include <QAxObject>
#include <QMetaEnum>
#include <QTextEdit>
#include <ActiveQt>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

// https://openclassrooms.com/forum/sujet/qt-mise-en-page-document-word-82682
// https://forum.qt.io/topic/45299/solved-reading-ms-word-in-qt-please/2
    //http://main-info.site/2016/06/21/utilisez-microsoft-word-comme-un-generateur-de-codes-a-barres/
    // http://www.fontspace.com/category/barcode

    QAxObject *my_app = new QAxObject("Word.Application");
    // Si tu enlèves cette ligne, Word tourne en tâche de fond
    my_app->dynamicCall("SetVisible(bool)", true);
    QAxObject* my_docs = my_app->querySubObject("Documents()");
    QAxObject* doc = my_docs->querySubObject("Add()");
    QAxObject* content = doc->querySubObject("Content()");
    QAxObject* paragraphs = content->querySubObject("Paragraphs()");
    QAxObject* paragraph = paragraphs->querySubObject("Add()");
    QAxObject* range = paragraph->querySubObject("Range()");
    QAxObject *font = range->querySubObject("Font");
    font->dynamicCall("SetName(QString)", "IDAHC39M Code 39 Barcode");

    range->dynamicCall("SetText(const QString &)", "Pascal=BARREAU");


    //doc->dynamicCall("SaveAs(const QString &)","C:\\Test.doc");

#if 0
    QMetaEnum WdSaveFormat = getMetaEnum(doc, "WdSaveFormat");
    doc->dynamicCall("SaveAs(const QString &, int)",
                     "C:\\Test.doc",
                     WdSaveFormat.keyToValue("wdFormatDocument97"));
#endif

    // Sans cette ligne, Word reste ouvert après la fermeture de l'application
    //my_app->dynamicCall("Quit()");

}

MainWindow::~MainWindow()
{
    delete ui;
}
