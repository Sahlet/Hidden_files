#include "rulesmanager.h"

const QStringList RulesManager::GetRules(){
    return GetLocalRules() + GetGlobalRules();
}

const QStringList RulesManager::GetLocalRules(){
    return this->_localRules;
}

const QStringList RulesManager::GetGlobalRules(){
    return this->_globalRules;
}

void RulesManager::add_local_rule(QString s){
    this->_localRules.push_back(s);
    this->PopulateRulesList();
}

void RulesManager::AddGlobalRule(QString s){
    this->_globalRules.push_back(s);
}

void RulesManager::ToogleHidding(bool m){
    this->_hiddingIsOn = m;
}

bool RulesManager::HiddingIsOn(){
    return this->_hiddingIsOn;
}

void RulesManager::ToogleVisibleRules(char i){
    this->_visibleRules = i;
}

char RulesManager::GetVisibleTypeOfRules(){
    return this->_visibleRules;
}

RulesManager::RulesManager(){
    this->_localRules.push_back("First rule");
    this->_globalRules.push_back("Second rule");
    this->_visibleRules = 0;
}

void RulesManager::PopulateRulesList(){
    this->SelectChb(this->GetVisibleTypeOfRules());
}

RulesManager::RulesManager(const RulesManager &rulesManager)
{
    this->_globalRules = rulesManager._globalRules;
    this->_localRules = rulesManager._localRules;
    this->_hiddingIsOn = rulesManager._hiddingIsOn;
    this->_visibleRules = rulesManager._visibleRules;
}
