//
//  IntelWeb.cpp
//  Project 4
//
//  Created by Gopi Suresh on 3/8/16.
//  Copyright © 2016 Gopi. All rights reserved.
//

#include <stdio.h>
#include "IntelWeb.h"
#include "DiskMultiMap.h"
#include "BinaryFile.h"
#include <iostream> // needed for any I/O
#include <fstream>  // needed in addition to <iostream> for file I/O
#include <sstream>  // needed in addition to <iostream> for string stream I/O
#include <string>
#include <algorithm>
#include <set>


using namespace std;

IntelWeb::IntelWeb(){
    
}

IntelWeb::~IntelWeb(){
    target.close();
    initiator.close();
}

bool IntelWeb::createNew(const string &filePrefix, unsigned int maxDataItems){
    //machine.createNew(filePrefix+"_machine.dat", 100);
    maxItems = maxDataItems;
    if (initiator.createNew(filePrefix+"_initiator.dat", maxDataItems) && target.createNew(filePrefix+"_target.dat", maxDataItems))
        return true;
    return false;
}

bool IntelWeb::openExisting(const string &filePrefix){
    //maxItems = ===================================================come back if items is necessary
    if(initiator.openExisting(filePrefix+"_initiator.dat") && target.openExisting(filePrefix+"_target.dat"))
        return true;
    return false;
}

void IntelWeb::close(){
    initiator.close();
    target.close();
}

bool IntelWeb::ingest(const string &telemetryFile){
    ifstream inf(telemetryFile);
    if( ! inf ){
        cout << "Cannot open the file." << endl;
        return false;
    }
    
    string line;
    while (getline(inf, line)) {
        istringstream iss(line);
        string machine;
        string init;
        string targ;
        if (! (iss >> machine >> init >> targ)) {
            cout << "Ignoring badly formatted input" << endl;
            continue;
        }
        //cout << machine << "\t" << init << "\t" << targ << endl;
        initiator.insert(init, targ, machine);
        target.insert(targ, init, machine);
    }
    //cout << "target:"<<endl;
    //target.print();
    //cout << "Initiator:" << endl;
    //initiator.print();
    return true;
}

bool operator<(const InteractionTuple& one, const InteractionTuple& two) {
    if(one.context == two.context){
        if (one.from == two.from) {
            return one.to < two.to;
        }
        return one.from < two.from;
    }
    return one.context < two.context;
}

unsigned int IntelWeb::crawl(const std::vector<std::string> &indicators, unsigned int minPrevalenceToBeGood, std::vector<std::string> &badEntitiesFound, std::vector<InteractionTuple> &interactions){
    //map<string, int> bads;
    vector<pair<string, vector<InteractionTuple> > > bads;
    for (int i = 0; i < indicators.size(); i++) {
        vector<InteractionTuple> b;
        bads.push_back(pair<string, vector<InteractionTuple> >(indicators[i], b));
    }
    for (int i = 0; i < bads.size() ; i++) {
        //cout << bads[i].first << endl;
        DiskMultiMap::Iterator iti = initiator.search(bads[i].first);
        DiskMultiMap::Iterator itt = target.search(bads[i].first);
        while (iti.isValid()) {
            crawlMMT = *iti;
            
            vector<pair<string, vector<InteractionTuple> > >::iterator it = bads.begin();
            while (it != bads.end()) { // look if the same sketchy string is already found
                if (it->first == crawlMMT.value) {
                    break;
                }
                it++;            }
            
            InteractionTuple ip(crawlMMT.value, crawlMMT.key, crawlMMT.context);
            if (it == bads.end()) { // if the sketchy string is not in the vector
                vector<InteractionTuple> b;
                b.push_back(ip);
                bads.push_back(pair<string, vector<InteractionTuple>>(crawlMMT.value, b));
            } else{
                it->second.push_back(ip); //increment value only not key.
            }
            ++iti;
        }
        while (itt.isValid()) {
            crawlMMT = *itt;
            
            vector<pair<string, int > >::iterator it = bads.begin();
            while (it != bads.end()) {
                if (it->first == crawlMMT.value) {
                    break;
                }
                it++;
            }
            //find_if(bads.begin(), bads.end(), IntelWeb::isPairEqual);
            if (it == bads.end()) { // if the value of the map exists in the list of sketchy strings
                bads.push_back(pair<string, int>(crawlMMT.value, 1));
            } else{
                it->second++; //increment value only not key.
            }
            ++itt;
        }
    }
    for (int i = 0; i < bads.size(); i++) {
        pair<string, int>  bob = bads[i];
        if ((i < indicators.size() && bob.second > 0) || (bob.second > 0 && bob.second < minPrevalenceToBeGood)) { //the item is from the indicator list or the prevalence is in the optimal range.
            badEntitiesFound.push_back(bob.first);
        }
    }
    
    set<InteractionTuple> bill;
    for (int i = 0; i < badEntitiesFound.size(); i++) {
        DiskMultiMap::Iterator iti= initiator.search(badEntitiesFound[i]);
        while (iti.isValid()) {
            crawlMMT = *iti;
            InteractionTuple it(crawlMMT.key, crawlMMT.value, crawlMMT.context);
            bill.insert(it);
            ++iti;
        }
        iti= target.search(badEntitiesFound[i]);
        while (iti.isValid()) {
            crawlMMT = *iti;
            InteractionTuple it(crawlMMT.value, crawlMMT.key, crawlMMT.context);
            bill.insert(it);
            ++iti;
        }
    }
    sort(badEntitiesFound.begin(), badEntitiesFound.end());
    set<InteractionTuple>::iterator it;
    it = bill.begin();
    while (it != bill.end()) {
        interactions.push_back(*it);
        it++;
    }
    //copy(bill.begin(), bill.end(), interactions.begin());
    //sort(interactions.begin(), interactions.end(), pairCompare);
    return (int)(badEntitiesFound.size());
    
}


bool IntelWeb::purge(const std::string &entity){
    vector<pair<string, string > > ls;
    DiskMultiMap::Iterator iti = initiator.search(entity);
    bool removed = false;
    while (iti.isValid()) {
        crawlMMT = *iti;
        removed = true;
        initiator.erase(crawlMMT.key, crawlMMT.value, crawlMMT.context);
        target.erase(crawlMMT.value, crawlMMT.key, crawlMMT.context);
    }
    
    iti = target.search(entity);
    while (iti.isValid()) {
        removed = true;
        crawlMMT = *iti;
        target.erase(crawlMMT.key, crawlMMT.value, crawlMMT.context);
        initiator.erase(crawlMMT.value, crawlMMT.key, crawlMMT.context);
    }
    return removed;
}

