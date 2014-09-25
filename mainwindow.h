#include <QMainWindow>
#include "ui_mainwindow.h"
class QTabWidget;
class QAction;
class QMenu;
class FilePanel;
class QLabel;
class SearchPanel;
class FileController;
class QWebView;
class QWebFrame;
class SearchPanel;
class TextEdit;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    QString checkSchemaPath(FileController*);
    TextEdit* currentTextEdit();

protected:
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *);

private:
    QList<FileController*> fileControllersList;
    SearchPanel* searchPanel;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction* closeAct;
    QAction* formatFileAct;
    QAction* openLastClosedAct;
    QAction* clearRecentAct;
    QAction* closeAllFilesAct;
    QAction* validateAction;
    QAction* separatorAct;
    QAction* searchPanelAct;

    QMenu *fileMenu;
    QMenu* openRecentMenu;
    int previousIndex;

    QWebFrame* frame;
    QWebView* webView;

    enum { MaxRecentFiles = 10 };
    QAction *recentFileActs[MaxRecentFiles];
    QList<QString> closedFileNames;

    void updateRecentFileList();
    void newFileController(FileController*, QString title = "untitled");
    void createActions();
    void createMenus();

private slots:
    void newFile();
    void open(QString absoluteFilePath = "");
    void openLast();
    void closeTab(int n = -1);
    void closeAllFiles();
    bool saveAs();
    bool save();
    bool saveFile(const QString&);
    bool maybeSave();
    void openRecentFile();
    void clearRecentFiles();
    void updateButtonsEnabled(int);
    void on_chooseSchemaButton_clicked();
    void on_openSchemaButton_clicked();
    void on_validateButton_clicked();
    void isSchemaCheckBoxClicked();
    void goToError(QListWidgetItem*);
    void formatFile();
    void searchPanelShowOrHide();
    void updateSearchPanelPosition(int,int);

};



