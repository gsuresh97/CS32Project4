//
//  DiskMultiMap.cpp
//  Project 4
//
//  Created by Gopi Suresh on 3/5/16.
//  Copyright © 2016 Gopi. All rights reserved.
//

#include <stdio.h>
#include "BinaryFile.h"
#include <iostream>
#include "DiskMultiMap.h"
#include <cstring>
#include <functional>
#include <string>

using namespace std;

DiskMultiMap::DiskMultiMap(){
    fileCreated = false;
}

DiskMultiMap::~DiskMultiMap(){
    close();
    cout << "closed"<<endl;
}

bool DiskMultiMap::createNew(const std::string &filename, unsigned int numBuckets){
    if (fileCreated) {
        bf.close();
    }
    
    if (!bf.createNew(filename))
        return false;
    fileCreated = true;
    
    m_numBuckets = numBuckets;
    empties = 0;
    bf.write(numBuckets, 0);
    bf.write(empties, sizeof(int));
    
    for (int i = 0; i < numBuckets; i++) {
        bf.write(i, sizeof(int) * (i + 2));
    }
    
    
    return true;
}

bool DiskMultiMap::openExisting(const std::string &filename){
    if (!bf.openExisting(filename)) {
        return false;
    }
    int b;
    bf.read(b, 0);
    m_numBuckets = b;
    return true;
}

void DiskMultiMap::close(){
    if (bf.isOpen()) {
        bf.close();
    }
}

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string &context){
    if (key.size() >= 120 || value.size() >= 120 || context.size() >= 120) {
        return false;
    }
    char k[key.size() + 1];
    char v[value.size() + 1];
    char c[context.size() + 1];
    strcpy(k, key.c_str());
    strcpy(v, value.c_str());
    strcpy(c, context.c_str());
    
    
    int plh = (m_hash(key) + 2) * 4;
    int posCurKey;
    bf.read(posCurKey, plh);
    int posKey = bf.fileLength();//======================================modify
    if (posCurKey < m_numBuckets){
        int kSize = strlen(k) + 1;
        bf.write(posKey, plh);
        bf.write(0, posKey); // next key
        bf.write(0, posKey + sizeof(int)); // next node
        bf.write(kSize, posKey + 2*sizeof(int)); // size of key
        bf.write(k, kSize, posKey + 3*sizeof(int)); // key
        posCurKey = posKey;
    }
    else{ // a key with the same hashvalue as this key already exists
        //find correct key
        int size;
        bf.read(size, posCurKey + sizeof(int)*2); // read size of key at posCurKey
        char ky[size + 1];
        int nextKey;
        bf.read(nextKey, posCurKey); // read the offset value of the next key.
        bf.read(ky, size, posCurKey + 3*sizeof(int)); // read key at posCurKey
        
        //loop through all the keys to find the input key in case multiple keys have the same hash value.
        while (strcmp(ky, k) != 0 && nextKey != 0) { // while the keys dont match and there are more keys.
            posCurKey = nextKey;
            bf.read(nextKey, posCurKey);
            bf.read(ky, size, posCurKey + 3*sizeof(int));
        }
        if (strcmp(ky, k) != 0 && nextKey == 0) { //there is no key.
            bf.write(posKey, posCurKey); // add a new key to the list of keys. (replace the 0 with the offset of new key.)
            posCurKey = posKey;
            int kSize = strlen(k) + 1;
            bf.write(0, posCurKey); // there is no key after this.
            bf.write(0, posCurKey + sizeof(int)); // the key has no node.
            bf.write(kSize, posCurKey + 2*sizeof(int)); // size of key
            bf.write(k, kSize, posCurKey + 3*sizeof(int)); // write key
        }
        
        //while (nextKey != 0) {
        //    posCurKey = nextKey;
        //    bf.read(nextKey, posCurKey);
        //}
        //bf.write(posKey, posCurKey);
    }
    

    //posCurKey holds the offset value for the key.
    int curNode;
    int posNode = bf.fileLength(); //===================================================modify
    bf.read(curNode, posCurKey + sizeof(int));

    if (curNode == 0) { // This key has no nodes
        bf.write(posNode, posCurKey + sizeof(int)); // write the position of the new node in the key node.
    } else{ // this key has nodes.
        int nextNode;
        bf.read(nextNode, curNode);
        
        while (nextNode != 0) { //Traverse the nodes until the last one is reached.
            curNode = nextNode;
            bf.read(nextNode, curNode);
        }
        bf.write(posNode, curNode);
    }

    int vSize = strlen(v) + 1;
    int cSize = strlen(c) + 1;
    bf.write(0, posNode); // write 0 since there are no nodes after this.
    bf.write(vSize, posNode + sizeof(int)); //write the size of the value
    bf.write(v, vSize, posNode + 2*sizeof(int)); // write the value
    bf.write(strlen(c), posNode + 2*sizeof(int) + vSize); // write the size of the context
    bf.write(c, cSize, posNode + 3*sizeof(int) + vSize);
    
    return true;
    
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string &key){
    int plh = (m_hash(key) + 2) * 4;
    int posCurKey;
    bf.read(posCurKey, plh);
    if (posCurKey < m_numBuckets) {
        Iterator it;
        return it;
    }
    
    //posCurKey has the offset of the bucket list wherein the key may exist
    char k[key.size() + 1];
    strcpy(k, key.c_str());
    int size;
    bf.read(size, posCurKey + sizeof(int)*2); // read size of key at posCurKey
    char ky[size + 1];
    int nextKey;
    bf.read(nextKey, posCurKey); // read the offset value of the next key.
    bf.read(ky, size, posCurKey + 3*sizeof(int)); // read key at posCurKey
    
    //loop through all the keys to find the input key in case multiple keys have the same hash value.
    while (strcmp(ky, k) != 0 && nextKey != 0) { // while the keys dont match and there are more keys.
        posCurKey = nextKey;
        bf.read(nextKey, posCurKey);
        bf.read(ky, size, posCurKey + 3*sizeof(int));
    }
    if (strcmp(ky, k) != 0 && nextKey == 0) { //there is no key.
        Iterator it;
        return it;
    }
    int node;
    bf.read(node, posCurKey + sizeof(int));
    Iterator it(node, key, this);
    return it;
}

int DiskMultiMap::erase(const std::string &key, const std::string &value, const std::string &context){
    Iterator it = search(key);
    if (!it.isValid()) {
        return 0;
    }
    MultiMapTuple m;
    int count;
    do{
        m = *(++it);
        if (m.value == value && m.context == context)
            count++;
    } while (it.isValid());
    
    int plh = (m_hash(key) + 2) * 4;
    int posCurKey;
    bf.read(posCurKey, plh);
    
    char k[key.size() + 1];
    strcpy(k, key.c_str());
    int size;
    bf.read(size, posCurKey + sizeof(int)*2); // read size of key at posCurKey
    char ky[size + 1];
    int nextKey;
    bf.read(nextKey, posCurKey); // read the offset value of the next key.
    bf.read(ky, size, posCurKey + 3*sizeof(int)); // read key at posCurKey
    
    //loop through all the keys to find the input key in case multiple keys have the same hash value.
    while (strcmp(ky, k) != 0 && nextKey != 0) { // while the keys dont match and there are more keys.
        posCurKey = nextKey;
        bf.read(nextKey, posCurKey);
        bf.read(ky, size, posCurKey + 3*sizeof(int));
    }
    int node;
    bf.read(node, posCurKey + sizeof(int)); // node has first value context pair for that key.
    int next;
    do{
        bf.read(next, node);
        
        if (<#condition#>) {
            <#statements#>
        }
    }while (next != 0);
    
    
    return count;
}

//Private Disk Multi Map Methods
/*--------------------------------------------------------------------------------------*/

void DiskMultiMap::print(){
    int nb;
    bf.read(nb, 0);
    cout << "Number of Buckets: " << nb << endl;
    
    int buck;
    for (int i = 0; i < m_numBuckets; i++) {
        bf.read(buck, sizeof(int) * (i + 2));
        if (buck > nb) {
            printKeys(buck);
        }
    }
}

void DiskMultiMap::printList(int offset){
    int count = 1;
    int nn = offset;
    do{
        int s1;
        bf.read(s1, nn + sizeof(int));
        char v[s1];
        bf.read(v, s1, nn + 2*sizeof(int));
        int s2;
        bf.read(s2, nn + 2* sizeof(int) + s1);
        char c[s1];
        bf.read(c, s2, nn + 3*sizeof(int) + s1);
        cout << "\t" << count << ": Value is " << v << "\t Context is " << c <<endl;
        bf.read(nn, nn);
        count++;
    } while (nn != 0);
    cout << endl;
    
}

void DiskMultiMap::printKeys(int offset){
    int count = 1;
    int nk = offset;
    do{
        int s;
        bf.read(s, nk + 2*sizeof(int));
        char v[s];
        bf.read(v, s, nk + 3*sizeof(int));
        
        cout << count << ": The key is " << v <<endl;
        
        int nn;
        bf.read(nn, nk + sizeof(int));
        printList(nn);
        
        bf.read(nk, nk);
        count++;
    } while (nk != 0);
    cout << endl;
    
}

int DiskMultiMap::m_hash(const string &str){
    hash<string> str_hash;
    return str_hash(str) % m_numBuckets;
}


//Iterator Methods
/*------------------------------------------------------------------------------------*/
DiskMultiMap::Iterator::Iterator(int moffset, string mkey, DiskMultiMap* mdmm){
    dmm = mdmm;
    nodeOffset = moffset;
    key = mkey;
    valid = true;
}

DiskMultiMap::Iterator::Iterator(){
    dmm = nullptr;
    nodeOffset = 0;
    key = "";
    valid = false;
}

MultiMapTuple DiskMultiMap::Iterator::operator*(){
    MultiMapTuple m;
    m.key = key;
    int s1;
    BinaryFile* bf = &(dmm->bf);
    
    bf->read(s1, nodeOffset + sizeof(int));
    char v[s1];
    bf->read(v, s1, nodeOffset + 2*sizeof(int));
    int s2;
    bf->read(s2, nodeOffset + 2* sizeof(int) + s1);
    char c[s1];
    bf->read(c, s2, nodeOffset + 3*sizeof(int) + s1);

    m.value = v;
    m.context = c;
    
    return m;
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++(){
    BinaryFile* bf = &(dmm->bf);
    bf->read(nodeOffset, nodeOffset);
    if (nodeOffset == 0)
        valid = false;
    
    return *this;
}

bool DiskMultiMap::Iterator::isValid() const{
    return valid;
}

void DiskMultiMap::Iterator::remove(int &nodeOff, int &next){
    nodeOff = nodeOffset;
    BinaryFile* bf = &(dmm->bf);
    bf->read(next, nodeOffset);
}