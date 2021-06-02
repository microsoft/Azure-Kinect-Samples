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

#include "MqttToTex.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <cmath>
#include <random>
#include <chrono>


// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

	DLLEXPORT
		void
		FillTOPPluginInfo(TOP_PluginInfo* info)
	{
		// This must always be set to this constant
		info->apiVersion = TOPCPlusPlusAPIVersion;

		// Change this to change the executeMode behavior of this plugin.
		info->executeMode = TOP_ExecuteMode::CPUMemWriteOnly;

		// The opType is the unique name for this TOP. It must start with a 
		// capital A-Z character, and all the following characters must lower case
		// or numbers (a-z, 0-9)
		info->customOPInfo.opType->setString("Mqtt2tex");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("MQTT to TEX from Azure Kinect");

		// Will be turned into a 3 letter icon on the nodes
		info->customOPInfo.opIcon->setString("MQT");

		// Information about the author of this OP
		info->customOPInfo.authorName->setString("Thomas Wester");
		info->customOPInfo.authorEmail->setString("thomas@glowbox.io");

		// This TOP works with 0 or 1 inputs connected
		info->customOPInfo.minInputs = 0;
		info->customOPInfo.maxInputs = 0;
	}

	DLLEXPORT
		TOP_CPlusPlusBase*
		CreateTOPInstance(const OP_NodeInfo* info, TOP_Context* context)
	{
		// Return a new instance of your class every time this is called.
		// It will be called once per TOP that is using the .dll
		return new MqttToTex(info);
	}

	DLLEXPORT
		void
		DestroyTOPInstance(TOP_CPlusPlusBase* instance, TOP_Context* context)
	{
		// Delete the instance here, this will be called when
		// Touch is shutting down, when the TOP using that instance is deleted, or
		// if the TOP loads a different DLL
		delete (MqttToTex*)instance;
	}

};

MqttToTex::MqttToTex(const OP_NodeInfo* info) :
	myNodeInfo(info)
{
	myExecuteCount = 0;
	reconnect = false;

	textureMemoryLocation = 0;
	databuffer.pointcount = -1;

	mqttClient = std::make_unique<MQTTClient>();
}

MqttToTex::~MqttToTex()
{
	mqttClient->Disconnect();
	mqttClient.release();
}

void
MqttToTex::getGeneralInfo(TOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// Set our pixel type here - important for different use cases, all pixel specification follows pixel type format
	ginfo->cookEveryFrame = true;
	ginfo->memPixelType = OP_CPUMemPixelType::RGBA32Float;
	ginfo->clearBuffers = false;

}

bool
MqttToTex::getOutputFormat(TOP_OutputFormat* format, const OP_Inputs* inputs, void* reserved1)
{
	int32_t w;
	int32_t h;
	inputs->getParInt2("Resolution", w, h);

	if (w != outputWidth || h != outputHeight) {
		outputWidth = w;
		outputHeight = h;
		size_t size = (w * h / 2) * 3;


		if (myExecuteCount == 0) {
			databuffer.color_data = new std::vector<uint8_t>(size * 3, 0);
			databuffer.depth_data = new std::vector<int16_t>(size * 3, 0);
		}
		else {
			databuffer.depth_data->resize(size, 0);
			databuffer.color_data->resize(size, 0);
		}
	}

	// Assign variable values to 'format' to specify the pixel format/resolution etc that we want to output
	format->width = outputWidth;
	format->height = outputHeight;
	format->bitsPerChannel = 32;

	format->floatPrecision = true;

	// Return true to tell the TOP to use the settings specified
	return true;
}


void
MqttToTex::execute(TOP_OutputFormatSpecs* output,
	const OP_Inputs* inputs,
	TOP_Context* context,
	void* reserved1)
{

	//check for reset pulse, often to reconnect to a different ip
	if (reconnect || myExecuteCount == 0) {
		std::cout << "Reset received..." << std::endl;
		inputs->getParInt2("Resolution", outputWidth, outputHeight);

		//first time excute is count we initialze the buffer to receive mqtt data in
		int size = (outputWidth * outputHeight) / 2;

		if (myExecuteCount == 0) {
			databuffer.color_data = new std::vector<uint8_t>(size * 3, 0);
			databuffer.depth_data = new std::vector<int16_t>(size * 3, 0);
		}
	
		databuffer.pointcount = 0;
		databuffer.timestamp = 0;
		myPointCount = 0;

		reconnect = false;

		const char* broker_in = inputs->getParString("Broker");
		broker.assign(broker_in);

		const char* client_in = inputs->getParString("Client");
		client.assign(client_in);

		const char* topic_in = inputs->getParString("Topic");
		topic.assign(topic_in);

		this->mqttClient->Connect(broker, client, topic);
	}

	//read the color fill paramter
	inputs->getParDouble4("Fill", this->fillR, this->fillG, this->fillB, this->fillA);

	//get a texture to write to
	float* mem = (float*)output->cpuPixelData[textureMemoryLocation];

	//get the latest mqtt data
	if (mqttClient->GetData(databuffer) > 0) {
		myPointCount = databuffer.pointcount;

		//fill the texture we got from TD
		fillBuffer(mem, outputWidth, outputHeight);

		//let TD know there is a new texture
		output->newCPUPixelDataLocation = textureMemoryLocation;

		textureMemoryLocation++;
		if (textureMemoryLocation > 2) {
			textureMemoryLocation = 0;
		}
	}

	myExecuteCount++;
}

//fill mem with buffer from mqtt
void
MqttToTex::fillBuffer(float* mem, int width, int height)
{
	//clear td texturedata
	memset(&mem[0], 0, width * height * 4 * sizeof(float_t));

	//we use 1 texture of which the top half is color and the both half is depth

	//pointer for depth data
	float* pospixel = &mem[0];

	//pointer for color data
	int colorstart = (width * (height / 2)) * 4;
	float* colorpixel = &mem[colorstart];

	uint32_t positiondataoffset = 0;
	uint32_t colordatabyteoffset = 0;

	uint32_t maxPoints = databuffer.pointcount > (width * height) / 2 ? (width * height) / 2 : databuffer.pointcount;

	//populate the texture buffer
	for (uint32_t i = 0; i < maxPoints; i++)
	{
		//mqtt data comes in as uint values, translate back to meters	
		pospixel[0] = databuffer.depth_data->at(positiondataoffset) * 0.001f;
		pospixel[1] = databuffer.depth_data->at(positiondataoffset + 1) * 0.001f;
		pospixel[2] = databuffer.depth_data->at(positiondataoffset + 2) * 0.001f;
		pospixel[3] = 1;

		positiondataoffset += 3;
		pospixel += 4;

		//fill in color if we are getting color, otherwise use the fill
		if (this->databuffer.hasColor) {
			colorpixel[0] = (databuffer.color_data->at(colordatabyteoffset) / 255.0f);
			colorpixel[1] = (databuffer.color_data->at(colordatabyteoffset + 1) / 255.0f);
			colorpixel[2] = (databuffer.color_data->at(colordatabyteoffset + 2) / 255.0f);
			colorpixel[3] = 1;
		}
		else
		{
			//some form of memcpy / block copy might be fractionally faster
			colorpixel[0] = this->fillR;
			colorpixel[1] = this->fillG;
			colorpixel[2] = this->fillB;
			colorpixel[3] = this->fillA;
		}

		colordatabyteoffset += 3;
		colorpixel += 4;
	}
}

int32_t
MqttToTex::getNumInfoCHOPChans(void* reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the TOP. In this example we are just going to send one channel.
	return 2;
}

void
MqttToTex::getInfoCHOPChan(int32_t index, OP_InfoCHOPChan* chan, void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("pointCount");
		chan->value = (float)myPointCount;
	}

}

bool
MqttToTex::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 5;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
MqttToTex::getInfoDATEntries(int32_t index,
	int32_t nEntries,
	OP_InfoDATEntries* entries,
	void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "executeCount");
#else // macOS
		strlcpy(tempBuffer, "executeCount", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "pointCount");
#else // macOS
		strlcpy(tempBuffer, "pointCount", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myPointCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 2)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "broker");
#else // macOS
		strlcpy(tempBuffer, "broker", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		strcpy_s(tempBuffer, broker.c_str());
#else // macOS
		strlcpy(tempBuffer, "broker", sizeof(tempBuffer));
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 3)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "client");
#else // macOS
		strlcpy(tempBuffer, "client", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		strcpy_s(tempBuffer, client.c_str());
#else // macOS
		strlcpy(tempBuffer, "client", sizeof(tempBuffer));
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 4)
	{
		// Set the value for the first column
#ifdef _WIN32
		strcpy_s(tempBuffer, "topic");
#else // macOS
		strlcpy(tempBuffer, "topic", sizeof(tempBuffer));
#endif
		entries->values[0]->setString(tempBuffer);

		// Set the value for the second column
#ifdef _WIN32
		strcpy_s(tempBuffer, topic.c_str());
#else	// macOS
		strlcpy(tempBuffer, "topic", sizeof(tempBuffer));
#endif
		entries->values[1]->setString(tempBuffer);
	}

}

void
MqttToTex::setupParameters(OP_ParameterManager* manager, void* reserved1)
{

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "(Re)Connect";

		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//broker
	{
		OP_StringParameter sp;

		sp.name = "Broker";
		sp.label = "Broker";
		sp.defaultValue = this->broker.data();
		OP_ParAppendResult res = manager->appendString(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//client
	{
		OP_StringParameter sp;

		sp.name = "Client";
		sp.label = "Client";

		sp.defaultValue = this->client.data();

		OP_ParAppendResult res = manager->appendString(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//topic
	{
		OP_StringParameter sp;

		sp.name = "Topic";
		sp.label = "Topic";

		sp.defaultValue = this->topic.data();

		OP_ParAppendResult res = manager->appendString(sp);
		assert(res == OP_ParAppendResult::Success);
	}

	//color
	{
		OP_NumericParameter np;

		np.name = "Fill";
		np.label = "Color Fill";

		np.defaultValues[0] = 0.0f;
		np.defaultValues[1] = 0.0f;
		np.defaultValues[2] = 0.0f;
		np.defaultValues[3] = 1.0f;

		OP_ParAppendResult res = manager->appendRGBA(np);
		assert(res == OP_ParAppendResult::Success);
	}

	//width
	{
		OP_NumericParameter  np;
		np.name = "Resolution";
		np.label = "Texture Resolution";

		np.defaultValues[0] = 1024;
		np.defaultValues[1] = 1024;


		OP_ParAppendResult res = manager->appendXY(np);
		assert(res == OP_ParAppendResult::Success);

	}

	//height
}


void
MqttToTex::pulsePressed(const char* name, void* reserved1)
{
	if (!strcmp(name, "Reset"))
	{
		reconnect = true;
	}
}

