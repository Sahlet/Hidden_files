#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "rulesmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_chb_HiddingIsOn_clicked(bool checked);

    void on_rbtn_LocalRules_clicked();

    void on_rbtn_AllRules_clicked();

    void on_rbtn_GlobalRules_clicked();

    void on_btn_AddRule_clicked();

    void on_tabWidget_currentChanged(int index);

    void SetListOfRulesContent(const QStringList listOfRules);

    void SelectChb(char i);

private:
    Ui::MainWindow *ui;
    RulesManager *_RulesManager;
    QTableWidget *table;
    void ConnectionsInit();
    void TableInit();
};

#endif // MAINWINDOW_H
