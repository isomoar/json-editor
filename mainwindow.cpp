#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QWebFrame>
#include <QUrl>
#include <QWebView>
#include <QWebFrame>
#include <QTabBar>
#include <QTextCursor>

#include "mainwindow.h"
#include "syntaxhighlightening/highlighter.h"
#include "textedit.h"
#include "filecontroller.h"
#include "structurer.h"
#include "searchpanel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setupUi(this);

    setWindowTitle("Schema based JSON editor");
    statusBar()->showMessage(tr("Ready"));
    previousIndex = 0;
    tabWidget->removeTab(0);
    tabWidget->removeTab(0);
    openSchemaButton->setEnabled(false);
    chooseSchemaButton->setEnabled(false);
    isSchemaCheckBox->setEnabled(false);
    validateButton->setEnabled(false);
    FileController *fc = new FileController();
    fileControllersList.append(fc);

    webView = new QWebView();
    QUrl url = QUrl("qrc:/webSources/index_new.html");
    webView->load(url);
    frame = webView->page()->mainFrame();

    searchPanel = new SearchPanel(this);
    searchPanel->hide();
   // searchPanel->move(0, splitter->sizes().at(0));
    QFile styleFile( ":/style.qss" );
    styleFile.open( QFile::ReadOnly );
    QString style( styleFile.readAll());
    setStyleSheet(style);

    connect(isSchemaCheckBox, SIGNAL(clicked()), this, SLOT(isSchemaCheckBoxClicked()));
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateButtonsEnabled(int)));
    connect(errorView, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(goToError(QListWidgetItem*)));
    connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(updateSearchPanelPosition(int,int)));


    QPalette pal;
    pal.setBrush(QPalette::HighlightedText, QColor(Qt::black));
    pal.setBrush(QPalette::Highlight, QColor(Qt::lightGray));
    errorView->setPalette(pal);

    tabWidget->addTab(fc->textEdit, "untitled");
    createActions();
    createMenus();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    searchPanel->setFixedWidth(e->size().width());
    searchPanel->move(0,splitter->sizes().at(0)-searchPanel->searchWidget->height()+5);
    QMainWindow::resizeEvent(e);
}

void MainWindow::updateSearchPanelPosition(int pos, int index)
{
   searchPanel->move(0,pos-searchPanel->searchWidget->height()+5);

}

void MainWindow::searchPanelShowOrHide()
{
    if(searchPanel->isHidden()) {
        searchPanel->show();
    }else {
        searchPanel->hide();
    }
}

TextEdit* MainWindow::currentTextEdit()
{
    FileController* fc = fileControllersList.at(tabWidget->currentIndex());
    return fc->textEdit;
}

void MainWindow::goToError(QListWidgetItem * item)
{
    FileController* fc = fileControllersList.at(tabWidget->currentIndex());

    QRegExp reg("Data#(.*)\\s*Schema#");
    int index = reg.indexIn(item->text());
    if(index<0)
        return;
    QString path =  reg.cap(1);
    //qDebug() << path;

    path.remove(path.length()-1, 1);

    Structurer st;
    st.createTree(fc->textEdit->toPlainText());
    index = st.getPositionForPath(path);

    QTextCursor tc = fc->textEdit->textCursor();
    tc.setPosition(index);
    fc->textEdit->setTextCursor(tc);
}

void MainWindow::formatFile()
{
    FileController* fc = fileControllersList.at(tabWidget->currentIndex());

    QString t = fc->textEdit->toPlainText();
    QJsonDocument dataDocument =  QJsonDocument::fromJson(t.toUtf8());
    if(dataDocument.isEmpty()) {
        qDebug() << "Invalid JSON, can't format";
        return;
    }
    t = dataDocument.toJson(QJsonDocument::Indented);
    fc->textEdit->setPlainText(t);
    statusBar()->showMessage("File formatted", 2000);
}

void MainWindow::updateButtonsEnabled(int index)
{
    if(previousIndex>=0) {
        fileControllersList.at(previousIndex)->disconnect();
    }

    if(fileControllersList.isEmpty())
        return;

    FileController* fc = fileControllersList.at(index);

    if(fc->absoluteFileName.right(5)==".json") {
         isSchemaCheckBox->setEnabled(true);
         validateButton->setEnabled(true);
        if(fc->isSchema()) {
            isSchemaCheckBox->setChecked(true);
            openSchemaButton->setEnabled(false);
            chooseSchemaButton->setEnabled(false);
        }else {
            isSchemaCheckBox->setChecked(false);
            openSchemaButton->setEnabled(true);
            chooseSchemaButton->setEnabled(true);
        }
    }else {
        validateButton->setEnabled(false);
        openSchemaButton->setEnabled(false);
        chooseSchemaButton->setEnabled(false);
        isSchemaCheckBox->setChecked(false);
    }
    previousIndex = index;
    pathLabel->setText(fc->absoluteFileName);
}

void MainWindow::isSchemaCheckBoxClicked()
{
    FileController* fc =fileControllersList.at(tabWidget->currentIndex());

    validateButton->setEnabled(true);

    if(isSchemaCheckBox->checkState()==Qt::Unchecked) {
        //check schema path and set completer to json data
        openSchemaButton->setEnabled(true);
        chooseSchemaButton->setEnabled(true);

        QString path = checkSchemaPath(fc);
        fc->setIsSchema(false);
        if(path.isEmpty()) {
            fc->textEdit->disconnect();
            fc->clearCompletions();
            return;
        }
        fc->createDataCompleter(path);
    }else {
        openSchemaButton->setEnabled(false);
        chooseSchemaButton->setEnabled(false);
        fc->setIsSchema(true);
    }
    fc->setConnectionsToTextEdit();
}

QString MainWindow::checkSchemaPath(FileController* fc)
{
    QString jsonData = fc->textEdit->toPlainText();
    QString path;

    jsonData.remove('\n');
    jsonData.replace('\\', "\\\\");

    path = frame->evaluateJavaScript(QString("checkFormat('%1')").arg(jsonData)).toString();

     if(path!="valid") {
         qDebug() << "evaluated checkSchemaPath(): " << path;
         return "";
     }

     path = frame->evaluateJavaScript(QString("getSchemaName('%1')").arg(jsonData)).toString();
    if(path.isEmpty()) {
        qDebug() <<  "evaluated checkSchemaPath(): \"$schema\" path is empty";
        return "";
    }

    //absolute path
    QFile schemaFile(path);
    if(!schemaFile.open(QFile::ReadOnly | QFile::Text)) {
        QString curDir = fc->absoluteFileName;
        int index = curDir.lastIndexOf('/');
        int length = curDir.mid(index+1).length();
        curDir = curDir.left(curDir.length()-length).append(path);

    //relative path
        QFile file1(curDir);
        if(!file1.open(QFile::ReadOnly | QFile::Text)) {
            qDebug() << QString("Error: path to \"$schema\" \"%1\" doesn't exists").arg(curDir);
            return "";
        }else
            return curDir;
    } else
        return path;
}

void MainWindow::on_chooseSchemaButton_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("What path use?");
    QPushButton *absoluteButton = msgBox.addButton(tr("Absolute"), QMessageBox::ActionRole);
    QPushButton *relativeButton = msgBox.addButton(tr("Relative"), QMessageBox::ActionRole);
    QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();

    if(msgBox.clickedButton()==cancelButton)
        return;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));

    if(fileName.isEmpty())
        return;

    if(msgBox.clickedButton()==relativeButton)  {
        int index = fileName.lastIndexOf('/');
        fileName = fileName.mid(index+1);
    }
    FileController* fc = fileControllersList.at(tabWidget->currentIndex());
    QJsonDocument dataDocument =  QJsonDocument::fromJson(fc->textEdit->toPlainText().toUtf8());

    if(!dataDocument.isObject()) {
        qDebug() <<  "Invalid JSON instance, can't insert schema path";
        return;
    }

    QJsonObject data = dataDocument.object();
    data.insert("$schema", QJsonValue(fileName));
    dataDocument.setObject(data);
    QString res= dataDocument.toJson(QJsonDocument::Indented);
    if(res!="") {
        fc->textEdit->setPlainText(res);
    }
    else {
        qDebug() << "When insert schema error: " << res;
    }
    fc->createDataCompleter(fileName);
}

void MainWindow::on_openSchemaButton_clicked()
{
    FileController* fc = fileControllersList.at(tabWidget->currentIndex());
    QString path = checkSchemaPath(fc);
    if(path.isEmpty()) {
        qDebug() << "When open schema check schema path :" << path;
        return;
    }
    open(path);
    isSchemaCheckBox->click();
}

void MainWindow::on_validateButton_clicked()
{
    FileController* fc = fileControllersList.at(tabWidget->currentIndex());
    errorView->clear();
    //schema
    if(isSchemaCheckBox->isChecked()) {
        QString schemaJson = fc->textEdit->toPlainText();
        schemaJson.remove('\n');
        schemaJson.replace("\\", "\\\\");
        QString res = frame->evaluateJavaScript(QString("checkFormat('%1')").arg(schemaJson)).toString();
        if(res!="valid")
            errorView->addItem(QString("Schema parsing error: %1\n").arg(res));
        else
            errorView->addItem("Schema valid\n");
     }else {
        //instance
        fc->textEdit->cleanExtraSelections();
        QString dataJson = fc->textEdit->toPlainText();
        dataJson.remove('\n');
        QString datares = frame->evaluateJavaScript(QString("checkFormat('%1')").arg(dataJson)).toString();

        if(datares!="valid") {
             errorView->addItem(QString("%1\n").arg(datares));
             return;
        }
        //outputString.clear();

        QString schemaName = checkSchemaPath(fc);
        if(schemaName.isEmpty()) {
          //  errorView->addItem(QString("%1\n").arg(outputString));
            return;
        }

        QFile file(schemaName);
        file.open(QFile::ReadOnly | QFile::Text);
        QString schemaJson = file.readAll();
        schemaJson.remove('\n');
        schemaJson.replace("\\", "\\\\");
        dataJson.replace("\\", "\\\\");

        QString res = frame->evaluateJavaScript(QString("val('%1', '%2')").arg(dataJson).arg(schemaJson)).toString();

        if(res.contains("valid")) {
             QJsonDocument errorDoc = QJsonDocument::fromJson(res.toUtf8());
             QJsonObject error = errorDoc.object();

             if(error.value("valid").toBool()) {
                 errorView->addItem("Valid success\n");
             }else {
                 QJsonArray errorArray = error.value("errors").toArray();
                 QJsonObject er;

                 QList<QString> path;
                 for(int i=0; i<errorArray.size(); ++i) {
                     er = errorArray.at(i).toObject();
                     QString report = QString("Message: %1\nData#%2\nSchema#%3\n").arg(er.value("message").toString()).arg(er.value("dataPath").toString()).arg(er.value("schemaPath").toString());
                     errorView->addItem(report);
                     path << er.value("dataPath").toString();
                 }
                 fc->textEdit->createStructureForError(path);
                 goToError(errorView->item(0));
                 errorView->setCurrentRow(0);
             }
        }else
            errorView->addItem(res);
     }

}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(formatFileAct);
    fileMenu->addAction(searchPanelAct);

    openRecentMenu = new QMenu();
    openRecentMenu->setTitle("Recent files");
    fileMenu->addMenu(openRecentMenu);
    openRecentMenu->addAction(openLastClosedAct);

    for (int i = 0; i < MaxRecentFiles; ++i)
        openRecentMenu->addAction(recentFileActs[i]);
    openRecentMenu->addAction(clearRecentAct);

    fileMenu->addAction(closeAct);
    fileMenu->addAction(closeAllFilesAct);
}

void MainWindow::createActions()
{
    newAct = new QAction(tr("New"), this);
    newAct->setShortcuts(QKeySequence::New);
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(tr("Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction( tr("Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save as..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    closeAct = new QAction(tr("Close"), this);
    closeAct->setShortcuts(QKeySequence::Close);
    connect(closeAct, SIGNAL(triggered()), this, SLOT(closeTab()));

    openLastClosedAct = new QAction(tr("Open recent closed file"), this);
    openLastClosedAct->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T ));
    connect(openLastClosedAct, SIGNAL(triggered()), this, SLOT(openLast()));


    formatFileAct = new QAction(tr("Format document"), this);
    connect(formatFileAct, SIGNAL(triggered()), this, SLOT(formatFile()));

    searchPanelAct = new QAction(tr("Show search panel"), this);
    searchPanelAct->setShortcut(QKeySequence::Find);
    connect(searchPanelAct, SIGNAL(triggered()), this, SLOT(searchPanelShowOrHide()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }

    clearRecentAct = new QAction(tr("Clear recent files list"), this);
    connect(clearRecentAct, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));

    closeAllFilesAct = new QAction(tr("Close all"), this);
    connect(closeAllFilesAct, SIGNAL(triggered()), this, SLOT(closeAllFiles()));
}

void MainWindow::clearRecentFiles()
{
    closedFileNames.clear();
    updateRecentFileList();
}

void MainWindow::updateRecentFileList()
{
     int numRecentFiles = qMin((int)MaxRecentFiles, closedFileNames.size());

     for (int i = 0; i < numRecentFiles; ++i) {
         recentFileActs[i]->setText(closedFileNames.at(i));
         recentFileActs[i]->setData(closedFileNames.at(i));
        recentFileActs[i]->setVisible(true);
     }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
      separatorAct->setVisible(numRecentFiles > 0);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        open(action->data().toString());
}

void MainWindow::openLast()
{
    if(closedFileNames.size()>0)
    {
        open(closedFileNames.last());
        closedFileNames.removeLast();
    }else
         statusBar()->showMessage("Last closed file list is empty", 6000);
}

void MainWindow::closeAllFiles()
{
    if(tabWidget->count()==0)
        return;
    while(tabWidget->count())
        closeTab(0);
}

//------------------------------------------------------------
//----------------NEW-----SAVE-----OPEN-----CLOSE-------------
//------------------------------------------------------------

void MainWindow::newFile()
{
    FileController* fc = new FileController();
    newFileController(fc);
}

void MainWindow::newFileController(FileController* fc, QString title)
{
    tabWidget->addTab(fc->textEdit, title);
    fileControllersList.append(fc);

    if(fc->absoluteFileName.right(5)==".json") {
        Highlighter* highlighter = new Highlighter(fc->textEdit->document());
        fc->textEdit->setNeedToCheckSyntax(true);
        QString path = checkSchemaPath(fc);
        if(path.isEmpty()) {
            qDebug() << "Cant create data comp:";
        }else {
            connect(fc->textEdit, SIGNAL(currentObjectChanged(QString)), fc, SLOT(updateCompletionsForCurrentObject(QString)));
            connect(fc->textEdit, SIGNAL(currentObjectUndefined()), fc, SLOT(clearCompletions())); //common connection
            fc->createDataCompleter(path);
        }
    }
    tabWidget->setCurrentWidget(fc->textEdit);
    pathLabel->setText(fc->absoluteFileName);
}

void MainWindow::open(QString absoluteFileName)
{
    if(absoluteFileName.isEmpty()) {
        absoluteFileName = QFileDialog::getOpenFileName(this, tr("Open file"), "/Users/liri/Documents/test");
        if (absoluteFileName.isEmpty())
            return;
    }

    //check if file is opened yet
    for(int i=0; i<fileControllersList.size(); ++i) {
        if(fileControllersList.at(i)->absoluteFileName==absoluteFileName) {
            tabWidget->setCurrentWidget(fileControllersList.at(i)->textEdit);
            return;
        }
    }

    //if not opened create new
    QFile file(absoluteFileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Error reading file %1:\n%2.")
                             .arg(absoluteFileName)
                             .arg(file.errorString()));
        return;
    }


    QString text = file.readAll();
    int index = absoluteFileName.lastIndexOf('/');
    QString fileName = absoluteFileName.mid(index+1);

    FileController* fc = new FileController();
    fc->setAbsoluteFileName(absoluteFileName);
    fc->textEdit->setPlainText(text);
    if(fileName.length()>20)
        fileName = fileName.left(12) + "...";
    newFileController(fc, fileName);

     //close untitled tab
    QString untitledText = fileControllersList.at(0)->textEdit->toPlainText();
    if(tabWidget->count()==2 && tabWidget->tabText(0)=="untitled" && untitledText.isEmpty())
        closeTab(0);

    statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::closeTab(int n)
{
    //check tabs count
    if(tabWidget->count()==0)
        return;
    if(n==-1)
        n = tabWidget->currentIndex();

    FileController* fc = fileControllersList.at(n);
    fc->disconnect();

    if(fc->textEdit->document()->isModified()) {
        if(!maybeSave())
            return;
    }

    if(!fc->absoluteFileName.isEmpty() && fc->absoluteFileName!="untitled") {
        if(!closedFileNames.contains(fc->absoluteFileName)) {
            //check size of closedFileNames before inserting new file name
            if(closedFileNames.size() > MaxRecentFiles+1)
                closedFileNames.removeFirst();
             closedFileNames.append(fc->absoluteFileName);
        }
    }
    tabWidget->blockSignals(true);
    fileControllersList.removeAt(n);
    tabWidget->removeTab(n);

    if(previousIndex==n)
        previousIndex = -1;

    //set current previous tab
    if(n>0) {
        tabWidget->setCurrentIndex(n-1);
        updateButtonsEnabled(n-1);
    }
    tabWidget->blockSignals(false);

    //updateRecentFileList();
    if(tabWidget->count()==0)
        close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  // writeSettings();
   event->accept();
}


bool MainWindow::save()
{
    int currentIndex = tabWidget->currentIndex();
    if(fileControllersList.at(currentIndex)->absoluteFileName.isEmpty())
        return saveAs();
    else
        return saveFile(fileControllersList.at(currentIndex)->absoluteFileName);
}

bool MainWindow::maybeSave()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("The document was changed. Do you want to save changes?");
    msgBox.addButton(tr("No"), QMessageBox::ActionRole);
    QPushButton *saveButton = msgBox.addButton(tr("Yes"), QMessageBox::ActionRole);
    QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);
    msgBox.exec();

    if(msgBox.clickedButton()==saveButton)
        return save();
    else if(msgBox.clickedButton()==cancelButton)
        return false;
    return true;
}

bool MainWindow::saveAs()
{
    //check is name valid
    QString absoluteFileName = QFileDialog::getSaveFileName(this);
    if (absoluteFileName.isEmpty())
        return false;

    //set FileController file name from dialog
    int index = absoluteFileName.lastIndexOf('/');
    FileController* fc = fileControllersList.at(tabWidget->currentIndex());
    fc->setAbsoluteFileName(absoluteFileName);

    //check if .json, highlight syntax
    QString fileName = absoluteFileName.mid(index+1);
    tabWidget->setTabText(tabWidget->currentIndex(), fileName);

    if(fileName.right(5)==".json") {
        Highlighter* highlighter = new Highlighter(fc->textEdit->document());
         fc->textEdit->setNeedToCheckSyntax(true);
    }
    return saveFile(absoluteFileName);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                              tr("Error write file %1:\n%2.")
                              .arg(fileName)
                              .arg(file.errorString()));
         return false;
    }

    FileController* fc = fileControllersList.at(tabWidget->currentIndex());
    QTextStream out(&file);
        out << fc->textEdit->toPlainText();

    if(fileName.right(5)==".json" && !isSchemaCheckBox->isChecked()) {
        QString path = checkSchemaPath(fc);
        if(!path.isEmpty()) {
            fc->createDataCompleter(path);
            fc->setConnectionsToTextEdit();
        }
        else
            qDebug() << "when save..." << path;
    }
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}



