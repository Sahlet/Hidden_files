#ifndef RULESMANAGER_H
#define RULESMANAGER_H

#include <iostream>
#include <QList>
#include <QListWidgetItem>

class RulesManager: public QObject
{
     Q_OBJECT
public:
    const QStringList GetRules();
    const QStringList GetLocalRules();
    const QStringList GetGlobalRules();
    void AddGlobalRule (QString s);
    void ToogleHidding(bool m);
    bool HiddingIsOn();
    void ToogleVisibleRules(char i);
    char GetVisibleTypeOfRules();
    void PopulateRulesList();
    RulesManager();
    RulesManager(const RulesManager &rulesManager);
signals:
    void SetListOfRulesContent(const QStringList listOfRules);
    void SelectChb(char i);
private slots:
    void add_local_rule(QString s);
private:
    QStringList _localRules;
    QStringList _globalRules;
    bool _hiddingIsOn;
    char _visibleRules;
};


#endif // RULESMANAGER_H
