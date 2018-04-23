#include "addruledialog.h"
#include "ui_addruledialog.h"
#include <QFileDialog>
AddRuleDialog::AddRuleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddRuleDialog)
{
    ui->setupUi(this);

}

AddRuleDialog::~AddRuleDialog()
{
    delete ui;
}

void AddRuleDialog::on_rbtn_LocalRule_clicked()
{
    ui->txb_LocalRulePath->setEnabled(true);
    ui->btn_SelectPath->setEnabled(true);
    ui->txb_GlobalRulePath->setEnabled(false);
}

void AddRuleDialog::on_rbtn_GlobalRule_clicked()
{
    ui->txb_LocalRulePath->setEnabled(false);
    ui->btn_SelectPath->setEnabled(false);
    ui->txb_GlobalRulePath->setEnabled(true);
}

void AddRuleDialog::on_btn_SelectPath_clicked()
{
    ui->txb_LocalRulePath->setText(QFileDialog::getOpenFileName(this, tr("Select file to hidden")));
}

void AddRuleDialog::on_buttonBox_accepted()
{
    if(ui->rbtn_LocalRule->isChecked())
    {
       add_local_rule(ui->txb_LocalRulePath->text());
    }
}
