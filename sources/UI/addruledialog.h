#ifndef ADDRULEDIALOG_H
#define ADDRULEDIALOG_H

#include <QDialog>

namespace Ui {
class AddRuleDialog;
}

class AddRuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddRuleDialog(QWidget *parent = 0);
    ~AddRuleDialog();

signals:
    void add_local_rule(QString const s);

private slots:
    void on_rbtn_LocalRule_clicked();

    void on_rbtn_GlobalRule_clicked();

    void on_btn_SelectPath_clicked();

    void on_buttonBox_accepted();

private:
    Ui::AddRuleDialog *ui;
};

#endif // ADDRULEDIALOG_H
