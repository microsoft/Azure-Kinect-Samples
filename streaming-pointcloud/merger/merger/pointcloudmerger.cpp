#include "pointcloudmerger.h"
#include "mqtt.h"
#include <iostream>

extern mqtt mosquittoWrapper;

//-------------------------------public functions-------------------------------//

//-----pointCloudMerger default contstructor------//
/*
Set all members to their respective 0 default except for the bufferSize
5 is the sweetspot to have some extra frames just in case one camera is taking
too long to send over
*/
pointCloudMerger::pointCloudMerger() {
	this->framePointCount = 0;
	this->frameBufferSize = 0;
	this->frameTimestamp = 0;
	this->frameColor = false;
	this->bufferSize = 5; //Minimum size for the buffer before just deleting them all 
	this->done = false;
	this->restart = false;
	this->syncMode = false;
}

//-----pointCloudMerger buffSize contstructor------//
/*
Calls the default constructor and sets the buffer size to a specific value instead
*/
pointCloudMerger::pointCloudMerger(uint32_t buffSize) {
	this->pointCloudMerger::pointCloudMerger();
	this->setBufferSize(buffSize);
}

//-----pointCloudMerger decontstructor------//
/*
Frees up all memory and sets members to their zero equivalent values
*/
pointCloudMerger::~pointCloudMerger() {
	this->framePointCount = 0;
	this->frameBufferSize = 0;
	this->frameTimestamp = 0;
	this->frameColor = 0;
	this->bufferSize = 0;
	this->done = false;
	this->restart = false;
	this->syncMode = false;

	if (this->buffer.size() > 0) {
		this->eraseList();
	}
}

//-----pointCloudMerger setBufferSize------//
/*
Sets the bufferSize to anything that is greater than or equal to 5 frames.
If not, error out and don't change the size.
*/
bool pointCloudMerger::setBufferSize(uint32_t buffSize) {
	if (buffSize < 5) {
		fprintf(stderr, "pointCloudMerger::setBufferSize() error, buffer size needs to be at least 5\n");
		fprintf(stderr, "Size entered was: %d\n", buffSize);
		return false;
	}

	this->bufferSize = buffSize;

	return true;
}

//-----pointCloudMerger createNewIndex------//
/*
Create a new entry in the unordered_map
This must be called first before attempting to insert into an index.
Outputs an error if the index is already found
*/
bool pointCloudMerger::createNewIndex(std::string indexName) {
	if (this->buffer.find(indexName) != this->buffer.end()) {
		fprintf(stderr, "pointCloudMerger::createNewEntry() error, entry with the index already exists\n");
		return false;
	}

	this->buffer[indexName] = new concurrency::concurrent_queue<messageBuffer*>;

	std::cout <<"Subscribed to '" << indexName <<"'" <<std::endl;

	return true;
}

//-----pointCloudMerger addMessage------//
/*
Checks to make sure that the entry is already found, if not. Output an error message and return false

Otherwise push the frame onto the concurrentqueue
*/
bool pointCloudMerger::addMessage(std::string indexName, messageBuffer* &msg) {
	if (this->errorCantFindEntry(indexName)) {
		fprintf(stderr, " 'pointCloudMerger::addMessage()'\n");
		return false;
	}

	this->buffer[indexName]->push(msg);

	return true;
}

//-----pointCloudMerger readMessage------//
/*
Checks to make sure that the entry is already found, if not. Output an error message and return false

Otherwise pops off the most recent frame from that index
*/
bool pointCloudMerger::readMessage(std::string indexName, messageBuffer*& msg) {
	if (this->errorCantFindEntry(indexName)) {
		fprintf(stderr, " 'pointCloudMerger::readMessage()'\n");
		return false;
	}

	if (!this->buffer[indexName]->try_pop(msg)) {
		return false;
	}

	return true;
}

//-----pointCloudMerger eraseList------//
/*
Checks to make sure that the buffer has at least 1 entry, errors out if it is an empty list

Otherwise it grabs all of the index names and then erases each Index item
*/
bool pointCloudMerger::eraseList() {
	if (this->buffer.size() == 0) {
		fprintf(stderr, "error, list is empty, 'pointCloudMerger::emptyList()'\n");
		return false;
	}

	std::vector<std::string> indexNames;

	for (auto iter : this->buffer) {
		indexNames.emplace_back(iter.first);
	}


	for (int i = 0; i < indexNames.size(); ++i) {
		this->eraseIndex(indexNames[i]);
	}

	return true;
}

//-----pointCloudMerger eraseIndex------//
/*
Checks to make sure that the entry is already found, if not. Output an error message and return false

unsubscribe from that topic
empty the concurrentqueue
delete the concurrentqueue
erase the entry from the unordered_map
*/
bool pointCloudMerger::eraseIndex(std::string& indexName) {
	if (this->errorCantFindEntry(indexName)) {
		fprintf(stderr, " 'pointCloudMerger::eraseItem()'\n");
		return false;
	}

	mosquittoWrapper.unsubscribe((char*)indexName.c_str());
	messageBuffer* temp = nullptr;
	concurrency::concurrent_queue<messageBuffer*>* queuBuffer = this->buffer[indexName];
	while (queuBuffer->try_pop(temp)) {
		delete temp;
	}
	delete this->buffer[indexName];
	this->buffer.erase(indexName);

	return true;
}

//-----pointCloudMerger merge------//
/*
Checks to make sure that the buffer is nonzero, if is zero error out

Checks to make sure that each buffer has at least 1 frame, doesn't merge if one is missing

Otherwise, make a vector to hold the most recent frame from each camera and combine them all to one buffer
*/
bool pointCloudMerger::merge() {
	bool result = true;
	if (this->buffer.size() == 0) {
		result = false;
	}
	for (auto iter : this->buffer) {
		if (result == true && iter.second->unsafe_size() == 0) {
			result = false;
		}
	}

	//Each buffer had at least one frame
	if (result == true) {
		std::vector<messageBuffer*> frames(this->buffer.size());
		//Set counters back to zero
		this->framePointCount = 0;
		this->frameBufferSize = 0;
		this->frameTimestamp = 0;

		{
			//Scope will drop count
			int count = 0;
			//Go through each buffer entry and add to the total of the points and size
			//Insert the message into the vector
			for (auto iter : this->buffer) {
				messageBuffer* msgPtr = nullptr;
				if (iter.second->try_pop(msgPtr)) {
					this->framePointCount += msgPtr->pointCount;
					this->frameBufferSize += msgPtr->size;
					this->frameColor = msgPtr->color;
					frames[count] = msgPtr;
					++count;
				}
			}
		}

		//If result is still good and the first frame is a valid point
		if (result == true && frames[0]) {
			//Copy the timestamp only once
			this->frameTimestamp = frames[0]->timestamp;

			//Preform the actual merge
			if (!merge(frames)) {
				result = false;
			}

		}
		//Clear the memory out of the vector
		frames.clear();

		//Empty the queues, so older frames are not displayed next time
		for (auto iter : this->buffer) {
			messageBuffer* temp = nullptr;
			while (iter.second->try_pop(temp)) {
				delete temp;
			}
		}
	} //Not all cameras have sent a frame since last time
	else {
		//Make sure all the queues are less than the max allowed bufferSize
		//If its too big, empty the whole queue.
		for (auto iter : this->buffer) {
			if (iter.second->unsafe_size() > this->bufferSize) {
				messageBuffer* temp = nullptr;
				while (iter.second->try_pop(temp)) {
					delete temp;
				}
			}

		}
	}

	//Return the result of merge
	return result;
}

//-----pointCloudMerger setRestart, isRestart, setDone, isDone------//
/*
These are all simple setters and getters that keep track of program state
*/
bool pointCloudMerger::setRestart(bool toSet) {
	this->restart = toSet;

	return true;
}

bool pointCloudMerger::isRestart() {
	return this->restart;
}

bool pointCloudMerger::setDone(bool toSet) {
	this->done = toSet;

	return true;
}

bool pointCloudMerger::isDone() {
	return this->done;
}

//-----pointCloudMerger giveIndexNames------//
/*
Checks to make sure that the buffer is nonzero, if it is zero error out

Otherwise, return all the names of the different topics in a space separated format.
*/
bool pointCloudMerger::giveIndexNames(std::string &indexNames) {

	int activeTopics = this->buffer.size();
	
	if (activeTopics == 0) {
		fprintf(stderr, "error, buffer is empty, 'pointCloudMerger::giveIndexNames()'\n");
		return false;
	}
	
	for (auto iter : this->buffer) {
		indexNames.append(iter.first);
		
		if (--activeTopics != 0) {
			indexNames.append(" ");
		}
	}

	return true;
}

//-------------------------------private functions-------------------------------//

//-----pointCloudMerger errorCantFindEntry------//
/*
Helper function to make sure that an index is present in the unordered_map, otherwise output the trouble index and return true to
be picked by the other funtions.
*/
bool pointCloudMerger::errorCantFindEntry(std::string indexName) {
	if (this->buffer.find(indexName) == this->buffer.end()) {
		fprintf(stderr, "error, cannot find entry for buffer[%s]", indexName.c_str());
		return true;
	}
	return false;
}

//-----pointCloudMerger merge------//
/*
Merges all the points together

If the frame bool was present, sets the color size from 0 to 3, otherwise it stays 0 bytes.
*/
bool pointCloudMerger::merge(std::vector<messageBuffer*>& frames) {
	char* messageBuffer = new char[this->frameBufferSize + 13]; //To be sent out
	char* mergedDepthBuffer = new char[this->frameBufferSize];
	char* mergedColorBuffer = new char[this->frameBufferSize]; 
	int mergedDepthBufferOffset = 0;
	int mergedColorBufferOffset = 0;
	
	int colorSize = 0;
	if (this->frameColor) {
		colorSize = 3;
	}
	//Go through and make a combined buffer of depth and color info, keeping track of their offsets.
	for (auto iter : frames) {
		memcpy(&mergedDepthBuffer[mergedDepthBufferOffset], &iter->points[0], iter->pointCount*6);
		mergedDepthBufferOffset += iter->pointCount * 6;
		memcpy(&mergedColorBuffer[mergedColorBufferOffset], &iter->points[iter->pointCount * 6], iter->pointCount * colorSize);
		mergedColorBufferOffset += iter->pointCount * colorSize;
	}

	//Put the header together on the mergedMessageBuffer
	int mergedMessageBufferOffset = 0;
	memcpy(&messageBuffer[mergedMessageBufferOffset], &this->frameTimestamp, 8);
	mergedMessageBufferOffset += 8;
	memcpy(&messageBuffer[mergedMessageBufferOffset], &this->framePointCount, 4);
	mergedMessageBufferOffset += 4;
	memcpy(&messageBuffer[mergedMessageBufferOffset], &this->frameColor, 1);
	mergedMessageBufferOffset += 1;
	//Insert the depth data after the header
	memcpy(&messageBuffer[mergedMessageBufferOffset], &mergedDepthBuffer[0], mergedDepthBufferOffset);
	mergedMessageBufferOffset += mergedDepthBufferOffset;
	//Insert the color data after the depth data
	memcpy(&messageBuffer[mergedMessageBufferOffset], &mergedColorBuffer[0], mergedColorBufferOffset);
	mergedMessageBufferOffset += mergedColorBufferOffset;

	//For debugging purposes
	//std::cout << "\nTimeStamp:\t" << this->frameTimestamp << "\n"
	//	<< "Points:\t\t" << this->framePointCount << "\n"
	//	<< "Color:\t\t" << this->frameColor << "\n"
	//	<< "SIZE:\t\t" << mergedMessageBufferOffset << std::endl;

	//Output how many points are going to be sent
	std::cout << "Points sent : " << this->framePointCount << "                                        \r";

	//MosquittoWrapper send here
	mosquittoWrapper.sendMessage(mergedMessageBufferOffset, messageBuffer, (char*)mosquittoWrapper.giveTopic().c_str());

	//Clear out the frames
	for (auto iter : frames) {
		delete[]iter->points;
		iter->points = nullptr;
		delete iter;
	}

	//Free up the memory
	delete[]messageBuffer;
	delete[]mergedDepthBuffer;
	delete[]mergedColorBuffer;

	//Return true, merge was a success
	return true;
}