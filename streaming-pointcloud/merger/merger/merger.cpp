// merger.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "pointcloudmerger.h"
#include "mqtt.h"
#include <fstream>

mqtt mosquittoWrapper;
pointCloudMerger merger;

int main(int argc, char* argv[]) {
	//Get the filename from command line argument
	std::string fileName("settings.toml");
	if (argc < 2) {
		std::cout << "Did not supply settings file argument, using 'settings.toml' as the default" << std::endl;
	}
	else {
		fileName = argv[1];
	}
	std::ifstream file(fileName.c_str());
	//Error out if file doesn't exist
	if (!file.good()) {
		fprintf(stderr, "error, %s does not exist\n", fileName.c_str());
		exit(-1);
	}
	//Output that the program is parsing the file
	std::cout << "Parsing '" << fileName << "' file" << std::endl;
	toml::Data data = toml::parse(file);
	file.close();
	
	//Initialize the mosquitto library and parse the arguments into the mqtt class
	mosquittoWrapper.initMqttLib();
	mosquittoWrapper.ParseArgs(data);
	std::cout << "Finished parsing" << std::endl;
	mosquittoWrapper.connect();

	//Set callbacks and subscribe to 'cameras' and 'viewer'
	mosquittoWrapper.callbacks();
	mosquittoWrapper.subscribe((char*)TOPIC_CAMERA_AVAILABLE);
	mosquittoWrapper.subscribe((char*)TOPIC_CONTROL);
	mosquittoWrapper.subscribe((char*)TOPIC_VIEWER);

	//Start the loop for mosquitto to keep connection
	mosquitto_loop_start(mosquittoWrapper.giveMosquitto());
	//Main loop
	while (!merger.isDone()) {
		std::cout << "Processing frames..." << std::endl;
		while (!merger.isDone() && !merger.isRestart()) {
			merger.merge();
		}
		//Restarting or ending, clear out the unordered_map if restarting
		merger.setRestart(false);
		merger.eraseList();
	} //End main loop

	//Mosquitto is done, disconnect and clean up the library
	mosquittoWrapper.disconnect();
	mosquittoWrapper.finishMosquitto();

	std::cout << "Exiting program now" << std::endl;



	return 0;
}