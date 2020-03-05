#pragma once
#pragma warning(disable : 26812) //Disable enum class warning from the sdk api

#include "points.h"
#include "k4a/k4a.h"
#include "k4a/k4atypes.h"
#include "mqtt.h"

struct cameraConfiguration {
	int deviceIndex = 0;
	bool color = false;
	k4a_depth_mode_t depthMode = K4A_DEPTH_MODE_NFOV_2X2BINNED;
	k4a_color_resolution_t colorResolution = K4A_COLOR_RESOLUTION_720P;
	k4a_fps_t fpsMode = K4A_FRAMES_PER_SECOND_30;

};

class kinectCamera {
public: //functions
	kinectCamera();
	kinectCamera(cameraConfiguration cameraConfig);
	~kinectCamera();
	bool setCameraIndex(int index);
	bool parseToml(mqtt& mosquittoWrapper);
	bool connectCamera();
	bool setCameraConfiguration(cameraConfiguration config);
	void setCameraConfiguration(uint8_t arr[]);
	void checkSyncCables();
	bool setCameraCalibrationandTransformation();
	int getCameraIndex();
	bool startCamera();
	bool takeCaptureCamera(int timeoutMS);
	bool processCaptureCamera(mqtt& mosquittoWrapper, char* topic = nullptr);
	bool updateCameraTransformation(std::vector<Eigen::Vector3f> adjustmentVector);
	bool isDone();
	bool setDone(bool setDone);
	bool isRestart();
	bool setRestart(bool setRestart);
	bool isReady();
	bool setReady(bool setReady);
	bool disconnectCamera();
	bool writeToml();
	bool isSubordinate();

private: //functions
	inline void convertToEigen(std::vector<Eigen::Vector3f>& depthVec, int16_t*& xyzbuffer, size_t size) {
		for (int i = 0; i < size; ++i) {
			depthVec[i] = Eigen::Vector3f(xyzbuffer[i * 3 + 0], xyzbuffer[i * 3 + 1], xyzbuffer[i * 3 + 2]);
		}
	}

	uint32_t processCaptureColorCamera();
	uint32_t processCaptureDepthCamera();
	bool processDepth();
	bool writeSection(std::string file_name, std::string section_name);
	std::string createVariableString(std::string variableName, Eigen::Vector3f vector);
	k4a_depth_mode_t parseDepthMode(uint8_t arg);
	k4a_color_resolution_t parseColorResolution(uint8_t arg);
	k4a_fps_t parseFpsMode(uint8_t arg);
	bool parseColorCapture(uint8_t arg);

private: //variables
	k4a_device_t kinectDevice;
	k4a_capture_t kinectDeviceCapture;
	uint64_t kinectDeviceCaptureTimeStamp;

	k4a_image_t kinectDeviceCaptureDepthImage;
	k4a_image_t kinectDeviceCaptureXYZImage;
	k4a_image_t kinectDeviceCaptureColorImage;
	k4a_image_t kinectDeviceCaptureTransformedColorImage;
	k4a_device_configuration_t kinectDeviceConfiguration;
	k4a_transformation_t kinectDeviceTransformation;
	k4a_calibration_t kinectDeviceCalibration;
	std::vector<Eigen::Vector3f> transformation;

	points currentPoints;
	int deviceIndex;
	bool color;
	bool kinectDeviceStarted;
	bool kinectDeviceDone;
	bool kinectDeviceRestart;
	bool kinectDeviceReady;

	std::vector<int16_t> smallDepth;
	std::vector<uint8_t> smallColor;
	std::vector<Eigen::Vector3f> vec;
	char* messageBuffer;
};