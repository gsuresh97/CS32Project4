#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"
class DiskMultiMap
{
public:

	class Iterator
	{
	public:
		Iterator();
		// You may add additional constructors
        Iterator(int offset, string key, DiskMultiMap* mdmm);
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();

	private:
		// Your private member declarations will go here
        DiskMultiMap* dmm;
        int nodeOffset;
        string key;
        bool valid;
        void remove(int& nodeOff, int& next);
	};

	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);
    void print(); //=========================================================================remove

private:
	// Your private member declarations will go here
    BinaryFile bf;
    bool fileCreated;
    int m_numBuckets;
    int empties;
    int m_hash(const string& str);
    void printList(int offset);
    void printKeys(int offset);
};

#endif // DISKMULTIMAP_H_
