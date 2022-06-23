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

#pragma once

#include <deque>
#include <queue>
#include <mutex>

#include "TOP_CPlusPlusBase.h"

class FrameQueue
{
public:

	FrameQueue();
	~FrameQueue();

	// This will fill the class with buffers that can be updated. It should
	// be called every frame to ensure the buffers this class have is in
	// sync with what the TOP_OutputFormatSpecs is providing as output buffers.
	void				sync(TOP_OutputFormatSpecs *output);

	// Call this to get a buffer to fill with new buffer data.
	// You MUST call either updateComplete() or updateCancelled() when done with the buffer.
	// This may return nullptr if there is no buffer available for update.
	// width and height will be filled in with the width/height of the buffer
	void*				getBufferForUpdate(int *width, int *height);

	// Call this to tell the class that the data from the last getBufferForUpdate()
	// is ready to be used by the TOP
	void				updateComplete();

	// Call this to tell the class that the data from the last getBufferForUpdate()
	// did not get filled so it should not be queued for upload to the TOP
	void				updateCancelled();

	// Call this from execute() to send a new buffer (if available) to the TOP to output.
	void				sendBufferForUpload(TOP_OutputFormatSpecs *output);


private:

	std::mutex			myLock;
	std::mutex			myUpdateBufferLock;
	std::deque<void*>	myUnusedBuffers;
	std::deque<void*>	myUpdatedBuffers;

	int					myWidth;
	int					myHeight;

	void*				myUpdateBuffer;			
};
