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

using namespace std;


int main() {
    DiskMultiMap x;
    x.createNew("myhashtable.dat",100); // empty, with 100 buckets
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m52902");
    x.insert("hmm.exe", "pfft.exe", "m10001");
    x.insert("blah.exe", "bletch.exe", "m0003");
    
    x.print();
    
    // line 1
    if (x.erase("hmm.exe", "pfft.exe", "m52902") == 2) cout << "Just erased 2 items from the table!\n";
    // line 2
    if (x.erase("hmm.exe", "pfft.exe", "m10001") > 0)
        cout << "Just erased at least 1 item from the table!\n";
    // line 3
    if (x.erase("blah.exe", "bletch.exe", "m66666") == 0)
        cout << "I didn't erase this item cause it wasn't there\n";
}