#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include <string>
#include <vector>
#include "DiskMultiMap.h"

class IntelWeb
{
public:
        IntelWeb();
        ~IntelWeb();
        bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
        bool openExisting(const std::string& filePrefix);
        void close();
        bool ingest(const std::string& telemetryFile);
        unsigned int crawl(const std::vector<std::string>& indicators,
                	   unsigned int minPrevalenceToBeGood,
                	   std::vector<std::string>& badEntitiesFound,
                	   std::vector<InteractionTuple>& interactions
                	  );
        bool purge(const std::string& entity);

private:
	// Your private member declarations will go here
    //DiskMultiMap machine;
    DiskMultiMap initiator;
    DiskMultiMap target;
    BinaryFile bf;
    MultiMapTuple crawlMMT;
    int maxItems;
    bool isPairEqual(pair<string, int> element);
};

#endif // INTELWEB_H_
                
