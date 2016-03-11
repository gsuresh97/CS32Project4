//
//  main.cpp
//  Project 4
//
//  Created by Gopi Suresh on 3/5/16.
//  Copyright © 2016 Gopi. All rights reserved.
//

#include <iostream>
#include "DiskMultiMap.h"
#include <functional>
#include "IntelWeb.h"
#include <vector>

using namespace std;


int main() {
    IntelWeb h;
    h.createNew("trial", 100);
    h.ingest("/Users/family/Desktop/Gopi's Stuff/UCLA/CS32/Project 4/Project 4/malicious.txt");
    vector<string> bob;
    bob.push_back("www.virus.com/downloads");
    //bob.push_back("www.attackercontrolnetwork.com");
    
    vector<string> bads;
    vector<InteractionTuple> badInters;
    h.crawl(bob, 5, bads, badInters);
    for (int i = 0; i < bads.size(); i++) {
        cout << bads[i] << endl;
    }
    for (int i = 0; i < badInters.size(); i++) {
        cout << "From: " << badInters[i].from  << "\tTo: " << badInters[i].to << "\tContext: " << badInters[i].context<< endl;
    }
}