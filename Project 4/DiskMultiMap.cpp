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
    int sizePosKeyNode = (int)(key.size() + 1 + 3*sizeof(int));
    int posKey = getNewPos(sizePosKeyNode);//======================================modify
    if (posCurKey < m_numBuckets){
        int kSize = (int)(strlen(k) + 1);
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
            int kSize = (int)(strlen(k) + 1);
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
    int posNodeSize = (int)(value.size() + 1 + context.size() + 1 + 3*sizeof(int));
    int posNode = getNewPos(posNodeSize); //===================================================modify
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

    int vSize = (int)(strlen(v) + 1);
    int cSize = (int)(strlen(c) + 1);
    bf.write(0, posNode); // write 0 since there are no nodes after this.
    bf.write(vSize, posNode + sizeof(int)); //write the size of the value
    //cout << "Value Offset: " << posNode + 2*sizeof(int) << "\t" << v << endl;
    bf.write(v, vSize, posNode + 2*sizeof(int)); // write the value
    //cout << c << endl;
    bf.write(cSize, posNode + 2*sizeof(int) + vSize); // write the size of the context
    bf.write(c, cSize, posNode + 3*sizeof(int) + vSize);
    //cout << "Context Offset: " << posNode + 3*sizeof(int) + vSize << "\t" << c << endl;
    //cout << endl;
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
    int count = 0;
    do{
        
        m = *(++it);
        if (m.value == value && m.context == context)
            count++;
    } while (it.isValid());
    
    int plh = (m_hash(key) + 2) * 4;
    int posCurKey;
    bf.read(posCurKey, plh);
    
    char k[key.size() + 1];
    char v[value.size() + 1];
    char c[context.size() + 1];
    strcpy(k, key.c_str());
    strcpy(v, value.c_str());
    strcpy(c, context.c_str());
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
    int prev = posCurKey + sizeof(int);
    int node;
    bf.read(node, posCurKey + sizeof(int)); // node has the offset of the first value context pair for that key.
    //bf.read(next, node);
    while (node != 0){
        int s1;
        bf.read(s1, node + sizeof(int));
        char v1[s1];
        bf.read(v1, s1, node + 2*sizeof(int));
        int s2;
        bf.read(s2, node + 2* sizeof(int) + s1);
        char c1[s2];
        bf.read(c1, s2, node + 3*sizeof(int) + s1);
        bool rmv = false;
        int x;
        if (strcmp(v, v1) == 0 && strcmp(c, c1) == 0) {
            x = remove(node, prev); // next is the node that you want to remove, node is the node before that.
            rmv = true;
        }
        if (rmv) {
            node = x;
        } else{
            prev = node;
            bf.read(node, prev);
        }
    }
    
    
    return count;
}

//Private Disk Multi Map Methods
/*--------------------------------------------------------------------------------------*/
int DiskMultiMap::getNewPos(int size){
    int nextFree;
    bf.read(nextFree, sizeof(int));
    if (nextFree == 0) { // if there are no deleted node return the file length
        return bf.fileLength();
    }
    int delSize = 0;
    int prev = sizeof(int);
    int free = nextFree;
    bf.read(nextFree, free); //nextFree is the next deleted node that can be filled.
    
    int chosenNode = -1;
    
    while (free != 0 && delSize < size) { // loop through the deleted nodes until one with the correct size is found.
        bf.read(delSize, sizeof(int) + free);
        if (delSize >= size) {
            chosenNode = free;
            break;
        }
        prev = free;
        bf.read(free, free);
        //bf.read(nextFree, free);
    }
    //nextFree is either 0 or has the node after the one that is the correct size
    //free is the node that is the correct size
    //prev has the  node before free.
    //delSize has the size of the free node.

    if (delSize < size && nextFree == 0) { // if no such node exists, return file length.
        return bf.fileLength();
    }
    bf.read(nextFree, chosenNode);    
    if (delSize - size < 3*sizeof(int) + 2) { // if there is not enough space for the node and an extra key in that space
        bf.write(nextFree, prev); // link previous to next.
        return chosenNode;
    } else{ // there is space for multiple nodes
        
        bf.write(nextFree, chosenNode + size); //write the next free offset shifted over in the current deleted node
        //int pi;
        //bf.read(pi, chosenNode + size);
        //cout << pi << " has been written to " << chosenNode + size << endl;
        bf.write(chosenNode + size, prev); // linke the previous deleted node to the shifted one.
        bf.write(delSize - size, chosenNode + size + sizeof(int)); // write the next free removed node in the remaining free space.
        return chosenNode;
    }
}


int DiskMultiMap::remove(int node, int prev){
    int nextPoint = sizeof(int);
    int point;
    do {
        point = nextPoint;
        bf.read(nextPoint, point);
    }while (nextPoint != 0);
    
    //point now contains the last node of the linked list marking the deleted nodes.
    int pro;
    bf.read(pro, node); // collect the index of the node following the next node
    
    bf.write(node, point); // write the index of the node that should be deleted at the end of the deletion linked list.
    int f = 0;
    bf.write(f, node); // write 0 to the new node of the deletion linked list since there are no more deleted nodes.
    int s1;
    bf.read(s1, sizeof(int) + node); //get the size of the deleted linked list.
    int s2;
    bf.read(s2, 2*sizeof(int) + node + s1);
    int size = s1 + s2 + 3*sizeof(int);
    
    bf.write(size, node + sizeof(int)); // write the size of the deleted node on the linked list.
    
    bf.write(pro, prev); // write the next node after the deleted one to the node before the deleted one.
    return pro;
}

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
        cout << "Value Offset: " << nn + 2*sizeof(int) <<"\tSize: " << s1 << "\t" << v << endl;
        int s2;
        bf.read(s2, nn + 2* sizeof(int) + s1);
        char c[s1];
        bf.read(c, s2, nn + 3*sizeof(int) + s1);
        cout << "Context Offset: " << nn + 3*sizeof(int) + s1 << "\tSize: " << s2 << "\t" << c <<"\t";/*<< endl << endl;*/
        for (int i = 0; i < s2; i++) {
            cout << (int)c[i] << " ";
        }
        cout << endl << endl;
        //cout << "\t" << count << ": Value is " << v << "\t Context is " << c <<endl;
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
    //cout << "C- String value : " << v << endl;
    int s2;
    bf->read(s2, nodeOffset + 2* sizeof(int) + s1);
    char c[s2];
    bf->read(c, s2, nodeOffset + 3*sizeof(int) + s1);
    //cout << "C- String context : " << c << endl;
    
    m.value = (string)v;
    m.context = (string)c;
    
    //cout << "Context from Multimap: " << m.context << endl << "Value from Multimap: " << m.value << endl <<endl;
    
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
