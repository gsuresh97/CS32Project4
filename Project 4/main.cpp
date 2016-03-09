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


int main(int argc, const char * argv[]) {
    //hash<string> hs;
    //cout << hs("bulks") % 10 << endl;
    
    DiskMultiMap d;
    d.createNew("f.txt" , 10);
    d.insert("Hello", "World", "Ethel");
    d.insert("Hello", "King", "queenkjsbdfsbggdfgdfgdsfgslknfkdjsnfkjbgkldfbslingsgfdfgdsgdhgh");
    d.insert("Hello", "King", "queenkjsbdfsbggdfgdfgdsfgslknfkdjsnfkjbgkldfbslingsgfdfgdsgdhgh");
    //d.print();
    d.erase("Hello", "King", "queenkjsbdfsbggdfgdfgdsfgslknfkdjsnfkjbgkldfbslingsgfdfgdsgdhgh");
    //d.print();
    d.insert("bulks", "Snow", "mammoth");
    d.insert("Hello", "King", "queenling");
    d.insert("sulks", "Many", "Flowers");
    d.print();
}
