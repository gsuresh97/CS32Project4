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

using namespace std;


int main() {
    IntelWeb h;
    h.createNew("trial", 100);
    h.ingest("/Users/family/Desktop/Gopi's Stuff/UCLA/CS32/Project 4/Project 4/output.dat");
    
}