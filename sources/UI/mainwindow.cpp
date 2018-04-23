#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addruledialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), _RulesManager(new RulesManager)
{

    ui->setupUi(this);

    ConnectionsInit();
    TableInit();

    bool hiddingIsOn = this->_RulesManager->HiddingIsOn();

    on_chb_HiddingIsOn_clicked(hiddingIsOn);   

    _RulesManager->PopulateRulesList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_chb_HiddingIsOn_clicked(bool checked)
{
    this->_RulesManager->ToogleHidding(checked);
    if(checked)
    {
        ui->chb_HiddingIsOn->setChecked(checked);
        ui->chb_HiddingIsOn->setText("Сокрытие файлов включено");
    }
    else
    {
        ui->chb_HiddingIsOn->setChecked(checked);
        ui->chb_HiddingIsOn->setText("Сокрытие файлов выключено");
    }
}

void MainWindow::on_rbtn_LocalRules_clicked()
{
    this->_RulesManager->ToogleVisibleRules(1);
    ui->rbtn_LocalRules->setChecked(true);
    SetListOfRulesContent(this->_RulesManager->GetLocalRules());
}

void MainWindow::on_rbtn_AllRules_clicked()
{
    this->_RulesManager->ToogleVisibleRules(0);
    ui->rbtn_AllRules->setChecked(true);
    SetListOfRulesContent(this->_RulesManager->GetRules());
}

void MainWindow::on_rbtn_GlobalRules_clicked()
{
    this->_RulesManager->ToogleVisibleRules(2);
    ui->rbtn_GlobalRules->setChecked(true);
    SetListOfRulesContent(this->_RulesManager->GetGlobalRules());
}

void MainWindow::on_btn_AddRule_clicked()
{
    AddRuleDialog d;
    QObject::connect(&d, SIGNAL(add_local_rule(QString const)),
           _RulesManager, SLOT(add_local_rule(QString const)));
    d.exec();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 1)
    {
        //Проходимся по списку файлов
        //Будет сделано, когда пойму в каком виде список буду получать
        //Создаём иконку
        table->setHorizontalHeaderLabels(QString("Icon;File path; Size").split(";"));
        QTableWidgetItem *icon_item = new QTableWidgetItem;
           QIcon icon("List.png");
           icon_item->setIcon(icon);

          //Добавляем иконку
          table->setItem(0, 0, icon_item);

          table->setItem(0,1,new QTableWidgetItem("Е:/"));
          table->setItem(0,2,new QTableWidgetItem("100 кб."));
    }
}

void MainWindow::SetListOfRulesContent(const QStringList listOfRules){
    ui->listWidget->clear();
    ui->listWidget->addItems(listOfRules);
}

void MainWindow::SelectChb(char i){
    switch(i){
    case(0):
        on_rbtn_AllRules_clicked();
        break;
    case(1):
        on_rbtn_LocalRules_clicked();
        break;
    case(2):
        on_rbtn_GlobalRules_clicked();
        break;
    }
}


void MainWindow::ConnectionsInit(){
    QObject::connect(_RulesManager, SIGNAL(SetListOfRulesContent(const QStringList)),
           this, SLOT(SetListOfRulesContent(const QStringList)));
    QObject::connect(_RulesManager, SIGNAL(SelectChb(char)),
           this, SLOT(SelectChb(char)));
}

void MainWindow::TableInit(){
    table = ui->tableWidget;
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    //Создаем шапку
    table->setRowCount(1);
    table->setColumnCount(3);
    table->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}
