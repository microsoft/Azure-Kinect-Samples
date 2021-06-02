#include "mqtt.h"
#include "kinectCamera.h"

mqtt mosquittoWrapper;
kinectCamera camera;

int main(int argc, char* argv[])
{
	int deviceIndex = 0;
	int timeoutMS = 10;

	//Set the device index if the commandline argument was supplied. Keep at 0 if not.
	if (argc < 2) {
		std::cout << "Did not supply deviceIndex argument, using 0 as the default" << std::endl;
	}
	else {
		deviceIndex = std::atoi(argv[1]);
	}
	//Set the timeoutMS values (how long to wait for a capture from the Kinect for Azure camera to have an image ready)
	if (argc < 3) {
		std::cout << "Did not supply timeoutMS argument, using 10 as the default" << std::endl;
	}
	else {
		timeoutMS = std::atoi(argv[2]);
	}

	//Initialize the mqtt library
	mosquittoWrapper.initMqttLib();

	//Set the index of the camera and parse the toml, passes in mosquittoWrapper to have it parse its table as well
	camera.setCameraIndex(deviceIndex);
	camera.parseToml(mosquittoWrapper);

	//Setup callbacks, connect, and subscribe to the necessary topics for the app to run
	mosquittoWrapper.callbacks();
	mosquittoWrapper.connect();

	mosquittoWrapper.subscribe((char*)TOPIC_CAMERA_CONFIGURE);
	mosquittoWrapper.subscribe((char*)TOPIC_CONTROL);
	mosquittoWrapper.subscribe((char*)"calibrate", true);

	//Start the mosquitto loop
	mosquitto_loop_start(mosquittoWrapper.giveMosquitto());
	std::string topic(mosquittoWrapper.giveTopic());

	//While camera is not done capturing
	//Main Loop
	while (!camera.isDone()) {
		//Connect to the camera
		camera.connectCamera();
		camera.checkSyncCables();
		std::cout << "Camera is in ";
		if (camera.isSubordinate()) {
			std::cout << "SUBORDINATE mode." << std::endl;
		}
		else {
			std::cout << "MASTER/STANDALONE mode." << std::endl;
		}
		std::cout << "Waiting for response from the merger" << std::endl;
		do {
			//Wait to get the configuration form the merger, send a message every second
			mosquittoWrapper.sendMessage((int)topic.size(), topic.c_str(), (char*)TOPIC_CAMERA_AVAILBLE);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}  while (!camera.isReady() && !camera.isRestart());
		//Check to see if the while loop was exited because of restarting, or because camera received
		//the configuration from the merger
		std::cout << "Starting capture" << std::endl;
		if (!camera.isRestart()) {
			//If it didn't leave because of restart, reset ready variable and start camera up
			camera.setReady(false);
			camera.startCamera();
			camera.setCameraCalibrationandTransformation();
		}
		//Main loop for capturing the image
		while (!camera.isDone() && !camera.isRestart()) {
			if (camera.takeCaptureCamera(timeoutMS)) {
				camera.processCaptureCamera(mosquittoWrapper);
			}
		}
		//Exit restart Loop
		camera.setRestart(false);
		camera.disconnectCamera();
	}
	//Exit Main Loop

	//Exited main loop, close mosquitto and clean up the library
	mosquittoWrapper.disconnect();
	mosquittoWrapper.finishMosquitto();
	return 0;
}
