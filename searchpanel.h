#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QWidget>
#include "ui_searchpanel.h"
class TextEdit;

class SearchPanel : public QWidget, public Ui::Form
{
    Q_OBJECT
public:
    explicit SearchPanel(QWidget *parent = 0);
private:
    TextEdit* textEdit;
    QList<int> positions;
    int currentPositionIndex;
    QString str;
signals:

public slots:
    void searchMatches(QString);
       void on_nextButton_clicked();
       void on_previousButton_clicked();
       void on_replaceButton_clicked();
       void on_replaceAllButton_clicked();
       void drawCurrent();
       void updateSearch();
       virtual void	hideEvent(QHideEvent * event);
       virtual void	showEvent(QShowEvent * event);
};

#endif // SEARCHPANEL_H
