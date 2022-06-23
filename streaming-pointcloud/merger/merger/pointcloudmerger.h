#pragma once
#include <unordered_map>
#include "concurrent_queue.h"
#include "messageBuffer.h"

class pointCloudMerger {
public: //Functions
	pointCloudMerger();
	pointCloudMerger(uint32_t buffSize);
	~pointCloudMerger();
	bool setBufferSize(uint32_t buffSize);
	bool createNewIndex(std::string indexName);
	bool addMessage(std::string indexName, messageBuffer* &msg);
	bool readMessage(std::string indexName, messageBuffer*& msg);
	bool eraseList();
	bool eraseIndex(std::string& indexName);
	bool merge();
	bool setRestart(bool toSet);
	bool isRestart();
	bool setDone(bool toSet);
	bool isDone();
	bool giveIndexNames(std::string &indexNames);

private: //Functions
	bool errorCantFindEntry(std::string indexName);
	bool merge(std::vector<messageBuffer*>& frames);

private: //Variables
	std::unordered_map<std::string, concurrency::concurrent_queue<messageBuffer*>*> buffer;
	uint32_t framePointCount;
	uint64_t frameBufferSize;
	uint64_t frameTimestamp;
	bool frameColor;
	uint32_t bufferSize;
	bool done;
	bool restart;
	bool syncMode;
};