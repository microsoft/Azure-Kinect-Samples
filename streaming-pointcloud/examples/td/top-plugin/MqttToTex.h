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

#include "TOP_CPlusPlusBase.h"
#include "FrameQueue.h"
#include <thread>
#include <atomic>
#include "MQTTClient.cpp"

class MqttToTex : public TOP_CPlusPlusBase
{
public:
    MqttToTex(const OP_NodeInfo *info);
    virtual ~MqttToTex();

    virtual void		getGeneralInfo(TOP_GeneralInfo *, const OP_Inputs*, void*) override;
    virtual bool		getOutputFormat(TOP_OutputFormat*, const OP_Inputs*, void*) override;


    virtual void		execute(TOP_OutputFormatSpecs*,
							const OP_Inputs*,
							TOP_Context* context,
							void* reserved1) override;

	virtual void fillBuffer(float * mem, int width, int height);

    virtual int32_t		getNumInfoCHOPChans(void *reserved1) override;
    virtual void		getInfoCHOPChan(int32_t index,
								OP_InfoCHOPChan *chan, void* reserved1) override;

    virtual bool		getInfoDATSize(OP_InfoDATSize *infoSize, void *reserved1) override;
    virtual void		getInfoDATEntries(int32_t index,
									int32_t nEntries,
									OP_InfoDATEntries *entries,
									void *reserved1) override;

	virtual void		setupParameters(OP_ParameterManager *manager, void *reserved1) override;
	virtual void		pulsePressed(const char *name, void *reserved1) override;

private:

    // We don't need to store this pointer, but we do for the example.
    // The OP_NodeInfo class store information about the node that's using
    // this instance of the class (like its name).
    const OP_NodeInfo*	myNodeInfo;

    // In this example this value will be incremented each time the execute()
    // function is called, then passes back to the TOP 
    int					myExecuteCount;
	int					myPointCount;

	std::unique_ptr<MQTTClient> mqttClient;

	//mqtt broker ip address and port in this format: tcp://127.0.0.1:1883
	std::string broker;
	//mqtt client id
	std::string client;
	//mqtt topic
	std::string topic;

	//color fil;
	double fillR;
	double fillG;
	double fillB;
	double fillA;

	int32_t outputWidth;
	int32_t outputHeight;

	//TD pulse
	bool reconnect;
	
	//buffer to receive mqtt data in
	TextureData databuffer;
	int textureMemoryLocation;
};
