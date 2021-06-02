#include "kinectCamera.h"
#include <fstream>

//-----------------------------------------public functions-----------------------------------------//

//-----KinectCamera default contstructor------//
/*
Set all members to their respective 0 default
Have the message buffer be the max possible buffer required for an image
Add 180 degrees to the x-axis for rotation
*/
kinectCamera::kinectCamera() {
	this->deviceIndex = 0;
	this->color = false;
	this->kinectDeviceCaptureTimeStamp = 0;
	this->kinectDevice = nullptr;
	this->kinectDeviceCapture = nullptr;
	this->kinectDeviceCaptureDepthImage = nullptr;
	this->kinectDeviceCaptureXYZImage = nullptr;
	this->kinectDeviceCaptureColorImage = nullptr;
	this->kinectDeviceCaptureTransformedColorImage = nullptr;
	this->kinectDeviceConfiguration = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	this->kinectDeviceCalibration = k4a_calibration_t();
	this->kinectDeviceTransformation = nullptr;
	this->kinectDeviceStarted = false;
	this->kinectDeviceDone = false;
	this->kinectDeviceRestart = false;
	this->kinectDeviceReady = false;
	this->messageBuffer = new char[8 + 4 + 1 + (1024 * 1024 * 9)];
	this->transformation = std::vector<Eigen::Vector3f>(4, Eigen::Vector3f(0.0,0.0,0.0));
	transformation[1][0] = 180.0;
}

//-----KinectCamera config contstructor------//
/*
Call the default constructor
Apply the configuration supplied from cameraConfig to the camera's settings
*/
kinectCamera::kinectCamera(cameraConfiguration cameraConfig) {
	this->kinectCamera::kinectCamera();
	this->deviceIndex = cameraConfig.deviceIndex;
	this->color = cameraConfig.color;
	this->kinectDeviceConfiguration.depth_mode = cameraConfig.depthMode;
	this->kinectDeviceConfiguration.color_resolution = cameraConfig.colorResolution;
	//Make sure that the fps isn't greater than the max allowed for wfov_unbinned or 3072p for the cameras
	if ((cameraConfig.depthMode == K4A_DEPTH_MODE_WFOV_UNBINNED || cameraConfig.colorResolution == K4A_COLOR_RESOLUTION_3072P) && cameraConfig.fpsMode == K4A_FRAMES_PER_SECOND_30) {
		this->kinectDeviceConfiguration.camera_fps = K4A_FRAMES_PER_SECOND_15;
	}
	else {
		this->kinectDeviceConfiguration.camera_fps = cameraConfig.fpsMode;
	}
}

//-----KinectCamera decontstructor------//
/*
Free up memory and set everything to their zero respective value

Have to check every image if it is null to avoid the sdk outputting error messages.
*/
kinectCamera::~kinectCamera(){
	this->deviceIndex = 0;
	this->color = false;
	this->kinectDeviceStarted = false;
	this->kinectDeviceDone = false;
	this->kinectDeviceRestart = false;
	delete[]this->messageBuffer;
	if (this->kinectDevice != nullptr) {
		k4a_device_stop_cameras(this->kinectDevice);
		k4a_device_close(this->kinectDevice);
		this->kinectDevice = nullptr;
	}
	if (this->kinectDeviceCapture != nullptr) {
		k4a_capture_release(this->kinectDeviceCapture);
		this->kinectDeviceCapture = nullptr;
	}
	if (this->kinectDeviceCaptureDepthImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureDepthImage);
		this->kinectDeviceCaptureDepthImage = nullptr;
	}
	if (this->kinectDeviceCaptureXYZImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureXYZImage);
		this->kinectDeviceCaptureXYZImage = nullptr;
	}
	if (this->kinectDeviceCaptureColorImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureColorImage);
		this->kinectDeviceCaptureDepthImage = nullptr;
	}
	if (this->kinectDeviceCaptureTransformedColorImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureTransformedColorImage);
		this->kinectDeviceCaptureTransformedColorImage = nullptr;
	}
	this->kinectDeviceConfiguration = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	if (this->kinectDeviceTransformation != nullptr) {
		k4a_transformation_destroy(this->kinectDeviceTransformation);
		this->kinectDeviceTransformation = nullptr;
	}
}

//-----KinectCamera setCameraIndex------//
/*
Set the device index to the value of the argument
*/
bool kinectCamera::setCameraIndex(int index) {
	this->deviceIndex = index;

	return true;
}

//-----KinectCamera parseToml------//
/*
Grab all of the configurations from the toml files and set the available values

Also passes mqtt's table over so it can grab the configurations
*/
bool kinectCamera::parseToml(mqtt& mosquittoWrapper) {
	std::string settingsFile("settings");
	settingsFile.append(std::to_string(this->deviceIndex));
	settingsFile.append(".toml");
	std::ifstream file(settingsFile.c_str());
	if (!file.good()) {
		std::cout << "kinectCamera::parseToml() Error reading settings file!\n" << std::endl;
		std::cout << "File Name: " << settingsFile << std::endl;
		return false;
	}
	toml::Data data = toml::parse(file);
	file.close();

	//Getting the translations/rotation file to be used
	std::string cameraConfigFile = toml::get<toml::String>(data.at("file"));

	//Passing over the [MqttInfo] section to the mosquittoWrapper
	mosquittoWrapper.ParseArgs(toml::get<toml::Table>(data.at("MqttInfo")));

	//Opening the cameraConfig File
	file.open(cameraConfigFile, std::ios::in | std::ios::out);
	if (!file.good()) {
		std::cout << "kinectCamera::parseToml() Error reading camera position file!\n" << std::endl;
		std::cout << "File Name: " << cameraConfigFile << std::endl;
		return false;
	}
	data = toml::parse(file);
	file.close();

	//Grab the translation, rotation, min, max values off of the toml file
	//min and max have to be multiplied by 1000 in order to scale down to mm (camera's units)
	//Converts to floats here to take advantage of Eigen's librarys for rotation with quaterionf
	toml::Table table = toml::get<toml::Table>(data.at("Camera"));
	std::vector<std::int_least64_t> temp = toml::get<std::vector<toml::Integer>>(table.at("translation"));
	this->transformation[0] += Eigen::Vector3f((float)temp[0], (float)temp[1], (float)temp[2]);
	temp = toml::get<std::vector<toml::Integer>>(table.at("rotation"));
	this->transformation[1] += Eigen::Vector3f((float)temp[0], (float)temp[1], (float)temp[2]);
	temp = toml::get<std::vector<toml::Integer>>(table.at("min"));
	this->transformation[2] += Eigen::Vector3f((float)temp[0], (float)temp[1], (float)temp[2]);
	temp = toml::get<std::vector<toml::Integer>>(table.at("max"));
	this->transformation[3] += Eigen::Vector3f((float)temp[0], (float)temp[1], (float)temp[2]);

	return true;
}

//-----KinectCamera connectCamera------//
/*
Error out if there is already a device connected

Calls k4a_device_open if this->kinectDevice was null
*/
bool kinectCamera::connectCamera() {
	if (this->kinectDevice != nullptr) {
		fprintf(stderr, "kinectCamera::connectCamera() KinectDevice must be null, make sure to call closeCamera() prior to opening a new camera!\n");
		return false;
	}

	//Try to connect to the device error out if it fails to open
	if (K4A_FAILED(k4a_device_open(this->deviceIndex, &this->kinectDevice))) {
		fprintf(stderr, "kinectCamera::connectCamera() Failed to open the device with index %d", this->deviceIndex);
		this->kinectDevice = nullptr;
		return false;
	}

	return true;
}

//-----KinectCamera setCameraConfiguration------//
/*
Makes sure the device is connected and not null

Sets the device to the configuration, 
it will error correct if a user put 30fps when it should've been 15fps due to either 
color resolution or the depth mode
*/
bool kinectCamera::setCameraConfiguration(cameraConfiguration config) {
	if (this->kinectDevice == nullptr) {
		return false;
	}

	//Settings conflict with each other, lower the fps so it will still run
	if (config.depthMode == K4A_DEPTH_MODE_WFOV_UNBINNED || config.colorResolution == K4A_COLOR_RESOLUTION_3072P) {
		if (config.fpsMode == K4A_FRAMES_PER_SECOND_30) {
			config.fpsMode = K4A_FRAMES_PER_SECOND_15;
		}
	}
	//Determine if there is color capture and set the kinectDeviceConfiguration device
	this->color = config.color;
	this->kinectDeviceConfiguration.synchronized_images_only = true;
	this->kinectDeviceConfiguration.depth_mode = config.depthMode;
	this->kinectDeviceConfiguration.color_resolution = config.colorResolution;
	this->kinectDeviceConfiguration.camera_fps = config.fpsMode;

	this->checkSyncCables();

	//No need for extra cpu overhead if color isn't being captured, so use MJPG instead of bgra32
	if (this->color) {
		this->kinectDeviceConfiguration.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	}
	else {
		this->kinectDeviceConfiguration.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
	}

	return true;
}

//-----KinectCamera setCameraConfiguration------//
/*
Information was sent over from the merger, parse the modes that were sent.

Pass it over to setCameraConfiguration with the newly built cameraConfiguration structure
*/
void kinectCamera::setCameraConfiguration(uint8_t arr[]) {
	cameraConfiguration config;
	config.fpsMode = parseFpsMode(arr[0]);
	config.depthMode = parseDepthMode(arr[1]);
	config.colorResolution = parseColorResolution(arr[2]);
	config.color = parseColorCapture(arr[3]);

	this->setCameraConfiguration(config);
}

void kinectCamera::checkSyncCables() {
	bool sync_in = false;
	bool sync_out = false;
	//Get the sync jacks on the back of the device to determine
	//Master, Subordinate, or standalone status
	k4a_device_get_sync_jack(this->kinectDevice, &sync_in, &sync_out);
	if (sync_in == true) {
		this->kinectDeviceConfiguration.wired_sync_mode = K4A_WIRED_SYNC_MODE_SUBORDINATE;
	}
	else if (sync_out == true) {
		this->kinectDeviceConfiguration.wired_sync_mode = K4A_WIRED_SYNC_MODE_MASTER;
	}
	else {
		this->kinectDeviceConfiguration.wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE;
	}
}

//-----KinectCamera setCameraCalibrationandTransforamtion------//
/*
First, make sure that the device has been connected and not started, 
if it is not. Then get out of the function.

If there is, go ahead and gather the calibration and transformation variables from the camera
in order to morph the bgra image into the depth space.
*/
bool kinectCamera::setCameraCalibrationandTransformation() {
	if (this->kinectDevice == nullptr || this->kinectDeviceStarted == false) {
		fprintf(stderr, "kinectCamera::setCameraCalibration() Must be connected to a KinectDevice and started already\n");
		return false;
	}

	this->kinectDeviceCalibration = k4a_calibration_t();
	k4a_device_get_calibration(this->kinectDevice, this->kinectDeviceConfiguration.depth_mode, this->kinectDeviceConfiguration.color_resolution, &this->kinectDeviceCalibration);
	if (this->kinectDeviceTransformation) {
		k4a_transformation_destroy(this->kinectDeviceTransformation);
	}
	this->kinectDeviceTransformation = k4a_transformation_create(&this->kinectDeviceCalibration);

	return true;
}

//-----KinectCamera getCameraIndex------//
/*
Getter for the deviceIndex
*/
int kinectCamera::getCameraIndex() {
	return this->deviceIndex;
}

//-----KinectCamera startCamera------//
/*
Get the camera in the standby state
*/
bool kinectCamera::startCamera() {
	//Get the cameras on standby, ready to capture
	if (K4A_FAILED(k4a_device_start_cameras(this->kinectDevice, &this->kinectDeviceConfiguration))) {
		fprintf(stderr, "kinectCamera::startCamera() failed to start device at index %d\n", this->deviceIndex);
		this->kinectDevice = nullptr;
		return false;
	}

	this->kinectDeviceStarted = true;
	return true;
}

//-----KinectCamera setCameraIndex------//
/*
First checks if kinectDevice is not null, if it is null, error out of the function.

If the capture is not null, free up memory first.

Finally return the result if the capture succeeded within the timeout time
*/
bool kinectCamera::takeCaptureCamera(int timeoutMS) {
	if (this->kinectDevice == nullptr) {
		fprintf(stderr, "kinectCamera::takeCaptureCamera() needs to have device connected and running to take a picture\n");
		return false;
	}
	if (this->kinectDeviceCapture != nullptr) {
		k4a_capture_release(this->kinectDeviceCapture);
		this->kinectDeviceCapture = nullptr;
	}
	return (K4A_WAIT_RESULT_SUCCEEDED == k4a_device_get_capture(this->kinectDevice, &this->kinectDeviceCapture, timeoutMS));
}

//-----KinectCamera processCaptureCamera------//
/*
First checks that kinectDeviceCapture has a valid image

Then depending if color is active, process either depth and/or color images

If there was an error processing the images, pointCount will be set to -1
*/
bool kinectCamera::processCaptureCamera(mqtt &mosquittoWrapper, char* topic) {
	if (this->kinectDeviceCapture == nullptr) {
		fprintf(stderr, "kinectCamera::processCaptureCamera() needs to have a capture present to process\n");
		return false;
	}
	bool result = true;
	uint32_t pointCount = 0;
	if (this->color) {
		pointCount = this->processCaptureColorCamera();
	}
	else {
		pointCount = this->processCaptureDepthCamera();
	}

	if (pointCount != -1) {
		size_t size = 0;
		memcpy(&this->messageBuffer[size], &this->kinectDeviceCaptureTimeStamp, 8); // 8 is the sizeof(uint64_t)
		size = 8;
		memcpy(&this->messageBuffer[size], &pointCount, 4); // 4 is the sizeof(uint32_t)
		size += 4;
		memcpy(&this->messageBuffer[size], &this->color, 1); // 1 is the sizeof(bool)
		size += 1;
		memcpy(&this->messageBuffer[size], this->smallDepth.data(), this->smallDepth.size() * 2); // 2 is the sizeof(int16_t)
		size += (this->smallDepth.size() * 2);
		//If color was not captured, this will be a memcpy of 0 bytes
		memcpy(&this->messageBuffer[size], this->smallColor.data(), this->smallColor.size()); // 1 is the sizeof(uint8_t)
		size += this->smallColor.size();
		//Send off the message through mqtt
		mosquittoWrapper.sendMessage((int)size, this->messageBuffer, topic);

		//Display the number of points sent.
		
		std::cout << "Points sent : " << pointCount << "                                        \r";
	}  
	else {
		result = false;
	}

	//Free up all the memory that is always used (depth data) and the capture
	if (this->kinectDeviceCaptureDepthImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureDepthImage);
		this->kinectDeviceCaptureDepthImage = nullptr;
	}
	if (this->kinectDeviceCaptureXYZImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureXYZImage);
		this->kinectDeviceCaptureXYZImage = nullptr;
	}
	if (this->kinectDeviceCapture != nullptr) {
		k4a_capture_release(this->kinectDeviceCapture);
		this->kinectDeviceCapture = nullptr;
	}
	//Clear out the vectors
	this->smallDepth.clear();
	this->smallColor.clear();

	return result;
}

//-----KinectCamera updateCameraTransformation------//
/*
add the adjustmentVector (translation/rotation) vectors, this will normally only come from the message callback mqtt
*/
bool kinectCamera::updateCameraTransformation(std::vector<Eigen::Vector3f> adjustmentVector) {
	for (int i = 0; i < 2; ++i) {
		this->transformation[i] += adjustmentVector[i];
	}

	return true;
}

//-----KinectCamera isDone, setDone, isRestart, setRestart, isReady, setReady------//
/*
Simple getters and setters for the respective member variables
*/
bool kinectCamera::isDone() {
	return this->kinectDeviceDone;
}

bool kinectCamera::setDone(bool setDone) {
	return this->kinectDeviceDone = setDone;
}

bool kinectCamera::isRestart() {
	return this->kinectDeviceRestart;
}

bool kinectCamera::setRestart(bool setRestart) {
	return this->kinectDeviceRestart = setRestart;
}

bool kinectCamera::isReady() {
	return this->kinectDeviceReady;
}

bool kinectCamera::setReady(bool setReady) {
	return this->kinectDeviceReady = setReady;
}

//-----KinectCamera disconnectCamera------//
/*
Checks to see if there is a valid device, error out if not.

If there is, ensure all the images are freed before stoping and closing the cameras

Clear out the vectors as well.
*/
bool kinectCamera::disconnectCamera() {
	if (this->kinectDevice == nullptr) {
		fprintf(stderr, "kinectCamera::disconnectCamera() Needs to already be connected to a device\n");
		return false;
	}
	if (this->kinectDeviceCapture != nullptr) {
		k4a_capture_reference(this->kinectDeviceCapture);
		this->kinectDeviceCapture = nullptr;
	}
	if (this->kinectDeviceCaptureDepthImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureDepthImage);
		this->kinectDeviceCaptureDepthImage = nullptr;
	}
	if (this->kinectDeviceCaptureXYZImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureXYZImage);
		this->kinectDeviceCaptureXYZImage = nullptr;
	}
	if (this->kinectDeviceCaptureColorImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureColorImage);
		this->kinectDeviceCaptureDepthImage = nullptr;
	}
	if (this->kinectDeviceCaptureTransformedColorImage != nullptr) {
		k4a_image_release(this->kinectDeviceCaptureTransformedColorImage);
		this->kinectDeviceCaptureTransformedColorImage = nullptr;
	}
	if (this->kinectDeviceTransformation != nullptr) {
		k4a_transformation_destroy(this->kinectDeviceTransformation);
		this->kinectDeviceTransformation = nullptr;
	}
	k4a_device_stop_cameras(this->kinectDevice);
	k4a_device_close(this->kinectDevice);
	this->kinectDevice = nullptr;
	this->vec.clear();
	this->smallColor.clear();
	this->smallDepth.clear();
	return true;
}

//-----KinectCamera writeToml------//
/*
The toml file has the device index attached to it as well
Build up the toml file and open it

Ensure the file is still there and parse the toml file to get the configFileName
*/
bool kinectCamera::writeToml() {
	std::string settingsFile("settings");
	settingsFile.append(std::to_string(this->deviceIndex));
	settingsFile.append(".toml");
	std::ifstream file(settingsFile.c_str());
	if (!file.good()) {
		std::cout << "kinectCamera::writeToml() Error reading settings file!" << std::endl;
		std::cout << "File Name: " << settingsFile << std::endl;
		return false;
	}
	toml::Data data = toml::parse(file);
	file.close();

	std::string cameraConfigFile = toml::get<toml::String>(data.at("file"));

	//Visual feedback of the cameraConfigFile
	std::cout << cameraConfigFile << std::endl;
	return this->writeSection(cameraConfigFile, "Camera");
}

//-----KinectCamera isSubordinate------//
/*
Simple 'getter' that will check to see if the wired_sync_mode was set to Subordinate
*/
bool kinectCamera::isSubordinate() {
	return this->kinectDeviceConfiguration.wired_sync_mode == K4A_WIRED_SYNC_MODE_SUBORDINATE;
}

//-----------------------------------------private functions-----------------------------------------//

//-----KinectCamera processCaptureColorCamera------//
/*
Process the depthCapture first, if that failed no point in continuing

Then get the color image and transform it.

return the amount of points
*/
uint32_t kinectCamera::processCaptureColorCamera() {
	if (!this->processDepth()) {
		return -1;
	}

	//get color image from the device capture
	//if it failed to get the color image capture, return out of the function.
	this->kinectDeviceCaptureColorImage = k4a_capture_get_color_image(this->kinectDeviceCapture);
	if (this->kinectDeviceCaptureColorImage == nullptr) {
		return -1;
	}
	int w, h = 0;
	//Get the height and width rom the depthImage
	w = k4a_image_get_width_pixels(this->kinectDeviceCaptureDepthImage);
	h = k4a_image_get_height_pixels(this->kinectDeviceCaptureDepthImage);
	//Create a new image that is 4 time the width and size of a uint8_t data type
	//Then use the color and depth image to make the transformed_color_image be in the same coordinate space as the depth camera
	k4a_image_create(K4A_IMAGE_FORMAT_COLOR_BGRA32, w, h, w * 4, &this->kinectDeviceCaptureTransformedColorImage);
	//If the image failed to be allocated, free the color image and exit the function
	if (this->kinectDeviceCaptureTransformedColorImage == nullptr) {
		k4a_image_release(this->kinectDeviceCaptureColorImage);
		this->kinectDeviceCaptureColorImage = nullptr;
		return -1;
	}

	k4a_transformation_color_image_to_depth_camera(this->kinectDeviceTransformation, this->kinectDeviceCaptureDepthImage, this->kinectDeviceCaptureColorImage, this->kinectDeviceCaptureTransformedColorImage);
	//Get the raw buffer from the transformed color image
	uint8_t* colorbuffer = (uint8_t*)k4a_image_get_buffer(this->kinectDeviceCaptureTransformedColorImage);

	//'free' the vec before reassigning it to this->currentPoints
	this->vec.clear();
	this->vec.shrink_to_fit();
	this->vec = this->currentPoints.give_points();
	size_t size = this->vec.size();

	for (int i = 0; i < size; ++i) {
		//Vector.data() gives the raw xyz array that the points are stored in
		float* xyz = vec[i].data();
		if (xyz[0] == 0.0 && xyz[1] == 0.0 && xyz[2] == 0.0) {
			continue;
		}

		//Change the Eigen::Vector3f into xyz points
		this->smallDepth.emplace_back((int16_t)xyz[0]);
		this->smallDepth.emplace_back((int16_t)xyz[1]);
		this->smallDepth.emplace_back((int16_t)xyz[2]);

		//Change bgra into rgb
		this->smallColor.emplace_back((uint8_t)colorbuffer[i * 4 + 2]);
		this->smallColor.emplace_back((uint8_t)colorbuffer[i * 4 + 1]);
		this->smallColor.emplace_back((uint8_t)colorbuffer[i * 4 + 0]);
	}

	//Free up the memory and set color image related variables to null
	k4a_image_release(this->kinectDeviceCaptureColorImage);
	k4a_image_release(this->kinectDeviceCaptureTransformedColorImage);
	this->kinectDeviceCaptureColorImage = nullptr;
	this->kinectDeviceCaptureTransformedColorImage = nullptr;
	return (uint32_t)((this->smallDepth.size() / 3));
}

//-----KinectCamera processCaptureDepthCamera------//
/*
Process the depth first, if that fails. No point in continuing

Otherwise set the smallDepth vecotr to the culled points.

//Return the amount of points
*/
uint32_t kinectCamera::processCaptureDepthCamera() {
	if (!this->processDepth()) {
		return -1;
	}

	this->vec.clear();
	this->vec.shrink_to_fit();
	this->vec = this->currentPoints.give_points();
	size_t size = this->vec.size();
	for (int i = 0; i < size; ++i) {
		//Vector.data() gives the raw xyz array that the points are stored in
		float* xyz = vec[i].data();
		if (xyz[0] == 0.0 && xyz[1] == 0.0 && xyz[2] == 0.0) {
			continue;
		}

		//Change the Eigen::Vector3f into xyz points
		this->smallDepth.emplace_back((int16_t)xyz[0]);
		this->smallDepth.emplace_back((int16_t)xyz[1]);
		this->smallDepth.emplace_back((int16_t)xyz[2]);

	}

	return (uint32_t)(this->smallDepth.size() / 3);
}

//-----KinectCamera processDepth------//
/*
Checks to see if depthImage was valid, if not error out.

The main function that gets the depth image into an xyz buffer to be transformed into a vector of Eigen::Vector3f

Once it is in a vector of Eigen::Vector3f, it can be imported into the points class to run
translateRotateThreshold and sparseFilter functions to the points

return true
*/
bool kinectCamera::processDepth() {
	this->kinectDeviceCaptureDepthImage = k4a_capture_get_depth_image(this->kinectDeviceCapture);
	if (this->kinectDeviceCaptureDepthImage == nullptr) {
		return false;
	}

	this->kinectDeviceCaptureTimeStamp = k4a_image_get_timestamp_usec(this->kinectDeviceCaptureDepthImage);
	int w, h = 0;
	w = k4a_image_get_width_pixels(this->kinectDeviceCaptureDepthImage);
	h = k4a_image_get_height_pixels(this->kinectDeviceCaptureDepthImage);
	//XYZ point image needs to be a custom image that is 3 times the width of a int16_t (size of each point)
	//Transform the depth image into the xyz image
	k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM, w, h, w * 6, &this->kinectDeviceCaptureXYZImage);
	k4a_transformation_depth_image_to_point_cloud(this->kinectDeviceTransformation,
		this->kinectDeviceCaptureDepthImage,
		K4A_CALIBRATION_TYPE_DEPTH,
		this->kinectDeviceCaptureXYZImage);

	int16_t* xyzbuffer = (int16_t*)k4a_image_get_buffer(this->kinectDeviceCaptureXYZImage);
	size_t size = w * h;
	if (this->vec.size() == 0) {
		this->vec = std::vector<Eigen::Vector3f>(size, Eigen::Vector3f(0, 0, 0));
	}
	this->convertToEigen(this->vec, xyzbuffer, size);

	//Import the vector into the points class and do the translating, thresholding, and rotation to the points
	//Note: This does not mess with the ordering of the points.
	this->currentPoints.importPoints(vec);
	this->currentPoints.translateRotateThreshold(transformation[0], transformation[1], transformation[2], transformation[3]);
	this->currentPoints.sparseFilter(w, h);

	return true;
}

//-----KinectCamera writeSection------//
/*
Writes a section into the toml file. It will overwrite anything that was in there before.


*/
bool kinectCamera::writeSection(std::string fileName, std::string sectionName) {
	std::ofstream file_out;
	file_out.open(fileName, std::ios::out);
	//Output the section name surrounded by '[' ']'
	file_out << "[" << sectionName << "]\n";
	//Create the translation, rotation, min, max arrays of ints
	//Min and max need to be divided by 1000 to be scaled up to meters for the user to change by hand
	std::string string = createVariableString("translation", this->transformation[0]);
	file_out << string.c_str();
	Eigen::Vector3f tempRotation(this->transformation[1][0] - 180, this->transformation[1][1], this->transformation[1][2]);
	string = createVariableString("rotation", tempRotation);
	file_out << string.c_str();
	string = createVariableString("min", this->transformation[2]);
	file_out << string.c_str();
	string = createVariableString("max", this->transformation[3]);
	file_out << string.c_str();
	file_out.close();

	return true;
}

//-----KinectCamera createVariableString------//
/*
Helper function to get the array variable, so the Eigen::Vector3f is in array format for TOML to understand.
*/
std::string kinectCamera::createVariableString(std::string variableName, Eigen::Vector3f vector) {
	std::string variable;
	variable.append(variableName);
	float* arr = vector.data();
	variable.append(" = [" + std::to_string((int)arr[0]) + ", " + std::to_string((int)arr[1]) + ", "
		+ std::to_string((int)arr[2]) + "]\n");
	return variable;
}

//-----KinectCamera parseDepthMode------//
/*
Helper function to determine which depthMode was sent in from mqtt
*/
k4a_depth_mode_t kinectCamera::parseDepthMode(uint8_t arg) {
	k4a_depth_mode_t ret = K4A_DEPTH_MODE_OFF;
	switch (arg) {
		case 0:
			ret = K4A_DEPTH_MODE_OFF;
			break;
		case 1:
			ret = K4A_DEPTH_MODE_NFOV_2X2BINNED;
			break;
		case 2:
			ret = K4A_DEPTH_MODE_NFOV_UNBINNED;
			break;
		case 3:
			ret = K4A_DEPTH_MODE_WFOV_2X2BINNED;
			break;
		case 4:
			ret = K4A_DEPTH_MODE_WFOV_UNBINNED;
			break;
		case 5:
			ret = K4A_DEPTH_MODE_PASSIVE_IR;
			break;
		default:
			fprintf(stderr, "Error parsing depthMode\n");
	}

	return ret;
}

//-----KinectCamera parseColorResolution------//
/*
Helper function to determine which color resolution was sent in from mqtt
*/
k4a_color_resolution_t kinectCamera::parseColorResolution(uint8_t arg) {
	k4a_color_resolution_t ret = K4A_COLOR_RESOLUTION_OFF;
	switch (arg) {
	case 0:
		ret = K4A_COLOR_RESOLUTION_OFF;
		break;
	case 1:
		ret = K4A_COLOR_RESOLUTION_720P;
		break;
	case 2:
		ret = K4A_COLOR_RESOLUTION_1080P;
		break;
	case 3:
		ret = K4A_COLOR_RESOLUTION_1440P;
		break;
	case 4:
		ret = K4A_COLOR_RESOLUTION_1536P;
		break;
	case 5:
		ret = K4A_COLOR_RESOLUTION_2160P;
		break;
	case 6:
		ret = K4A_COLOR_RESOLUTION_3072P;
		break;
	default:
		fprintf(stderr, "Error parsing depthMode\n");
	}

	return ret;
}

//-----KinectCamera parseFpsMode------//
/*
Helper function to determine which camera fps mode was sent in from mqtt
*/
k4a_fps_t kinectCamera::parseFpsMode(uint8_t arg) {
	k4a_fps_t ret = K4A_FRAMES_PER_SECOND_5;
	switch (arg) {
		case 0:
			ret = K4A_FRAMES_PER_SECOND_5;
			break;
		case 1:
			ret = K4A_FRAMES_PER_SECOND_15;
			break;
		case 2:
			ret = K4A_FRAMES_PER_SECOND_30;
			break;
		default:
			fprintf(stderr, "Error parsing fpsMode\n");
	}

	return ret;
}

//-----KinectCamera parseColorCapture------//
/*
Helper function to determine whcich color capture setting was sent in from mqtt
*/
bool kinectCamera::parseColorCapture(uint8_t arg) {
	bool ret = false;
	switch (arg) {
	case 0:
		ret = false;
		break;
	case 1:
		ret = true;
		break;
	default:
		fprintf(stderr, "Error parsing colorCapture setting\n");
	}

	return ret;
}