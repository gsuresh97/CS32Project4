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
    initiator.createNew(filePrefix+"_initiator.dat", maxDataItems);
    target.createNew(filePrefix+"_target.dat", maxDataItems);
    return 0;
}

bool IntelWeb::openExisting(const string &filePrefix){
    return 0;
}

void IntelWeb::close(){
    
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
        cout << machine << "\t" << init << "\t" << targ << endl;
        initiator.insert(init, targ, machine);
        target.insert(targ, init, machine);
    }
    return true;
}