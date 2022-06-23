/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include "FrameQueue.h"
#include <assert.h>

FrameQueue::FrameQueue() :
	myUpdateBuffer(nullptr)
{

}

FrameQueue::~FrameQueue()
{
	assert(!myUpdateBuffer);
}

void
FrameQueue::sync(TOP_OutputFormatSpecs * output)
{
	myLock.lock();

	// First clear out buffers that are no longer valid
	for (auto itr = myUpdatedBuffers.begin(); itr != myUpdatedBuffers.end(); )
	{
		bool found = false;
		for (int j = 0; j < NumCPUPixelDatas; j++)
		{
			if (*itr == output->cpuPixelData[j])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			itr = myUpdatedBuffers.erase(itr);
		}
		else
		{
			itr++;
		}
	}

	for (auto itr = myUnusedBuffers.begin(); itr != myUnusedBuffers.end(); )
	{
		bool found = false;
		for (int j = 0; j < NumCPUPixelDatas; j++)
		{
			if (*itr == output->cpuPixelData[j])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			itr = myUnusedBuffers.erase(itr);
		}
		else
		{
			itr++;
		}
	}

	// Now if there are new buffers given to use, take hold of them
	for (int i = 0; i < NumCPUPixelDatas; i++)
	{
		void *buf = output->cpuPixelData[i];

		bool found = false;
		// Look for buf somewhere in our current data

		if (buf == myUpdateBuffer)
			found = true;
		else
		{
			for (const auto e : myUnusedBuffers)
			{
				if (e == buf)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				for (const auto e : myUpdatedBuffers)
				{
					if (e == buf)
					{
						found = true;
						break;
					}
				}
			}

			// Not found in our buffers, so add it as an available buffer.
			if (!found)
			{
				myUnusedBuffers.push_back(buf);

			}
		}
	}
	// Finally, clear out the active update buffer it it's become invalid
	if (myUpdateBuffer)
	{ 
		bool found = false;

		for (int i = 0; i < NumCPUPixelDatas; i++)
		{
			if (myUpdateBuffer == output->cpuPixelData[i])
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			myUpdateBufferLock.lock();

			// Throw it away
			myUpdateBuffer = nullptr;

			myUpdateBufferLock.unlock();
		}
	}

	myWidth = output->width;
	myHeight = output->height;

	myLock.unlock();
}

void*
FrameQueue::getBufferForUpdate(int *width, int *height)
{
	// If this occurs it means a updateComplete/updateCancelled call wasn't
	// done to match the previous call to getFrameForUpdate
	assert(!myUpdateBuffer);
	myLock.lock();
	
	void *buf = nullptr;
	// Try to get an unused buffer first
	if (!myUnusedBuffers.empty())
	{
		buf = myUnusedBuffers.front();
		myUnusedBuffers.pop_front();
	}

	// If there wasn't an unused buffer, then replace the content of the oldest filled buffer
	if (!buf && !myUpdatedBuffers.empty())
	{
		buf = myUpdatedBuffers.front();
		myUpdatedBuffers.pop_front();
	}
	if (buf)
	{
		myUpdateBufferLock.lock();
		myUpdateBuffer = buf;
	}
	*width = myWidth;
	*height = myHeight;
	myLock.unlock();
	return buf;
}

void
FrameQueue::updateComplete()
{
	myUpdateBufferLock.unlock();

	myLock.lock();
	if (myUpdateBuffer)
	{
		myUpdatedBuffers.push_back(myUpdateBuffer);
		myUpdateBuffer = nullptr;
	}
	myLock.unlock();
}

void
FrameQueue::updateCancelled()
{
	myUpdateBufferLock.unlock();

	myLock.lock();
	if (myUpdateBuffer)
	{ 
		myUnusedBuffers.push_back(myUpdateBuffer);
		myUpdateBuffer = nullptr;
	}
	myLock.unlock();
}

void
FrameQueue::sendBufferForUpload(TOP_OutputFormatSpecs* output)
{
	myLock.lock();

	if (!myUpdatedBuffers.empty())
	{
		void *buf = myUpdatedBuffers.front();
		myUpdatedBuffers.pop_front();

		for (int i = 0; i < NumCPUPixelDatas; i++)
		{
			if (output->cpuPixelData[i] == buf)
			{
				output->newCPUPixelDataLocation = i;
				buf = nullptr;
				break;
			}
		}
	}
	myLock.unlock();
}
