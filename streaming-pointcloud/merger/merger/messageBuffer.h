#pragma once
#include <string> //string library imports memcpy and uint64_t

//Message buffer is a struct that is used to hold the mqtt messages that are sent
//Has a constructor and deconstructor for the concurrent queue
struct messageBuffer {
	uint64_t timestamp;
	char * points;
	size_t size;
	uint32_t pointCount;
	bool color;

	messageBuffer() {
		this->timestamp = 0;
		this->points = nullptr;
		this->size = 0;
		this->pointCount = 0;
		this->color = false;
	}
	~messageBuffer() {
		this->timestamp = 0;
		delete[]this->points;
		this->points = nullptr;
		this->size = 0;
		this->pointCount = 0;
		this->color = false;
	}

	messageBuffer*& clone() {
		messageBuffer* ret = new messageBuffer;

		ret->timestamp = this->timestamp;
		ret->points = new char[this->size];
		memcpy(ret->points, this->points, this->size);
		ret->size = this->size;
		ret->pointCount = this->pointCount;
		ret->color = this->color;

		return ret;
	}

	messageBuffer operator&=(messageBuffer other) {
		if (this != &other) {
			this->timestamp = other.timestamp;
			this->size = other.size;
			delete[]this->points;
			this->points = new char[this->size];
			memcpy(this->points, other.points, this->size);
			this->pointCount = other.pointCount;
			this->color = other.color;
		}
		return *this;
	}
};