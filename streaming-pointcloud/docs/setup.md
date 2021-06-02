# Azure Kinect Point Cloud Streaming

This project will capture and merge 3D points (including RGB color for each point) from multiple Azure Kinect cameras and broadcast the resulting point cloud via MQTT for use in other applications.

## MQTT Broker

The components in this system communicate via MQTT so they will need to connect to an MQTT broker such as [Mosquitto](https://mosquitto.org/download/). The bandwidth required may be quite high so it is recommended to run a dedicated broker on a local wired network.

## Applications

The system is comprised of three applications which work in concert to capture, configure, merge, and transmit depth data (point cloud frames) to be rendered in 3rd-party applications. The three applications are as follows:

### Kinect capture
This application connects to the Azure Kinect to read the depth data, applies translation, rotation, and min/max thresholding, then sends the point data to the Merger. There should be one instance of this app for each Azure Kinect.

`kinect capture\kinect capture.exe`

### Merger
The merger application receives depth data from one or more capture instances and combines the points into single frames, then broadcasts the resulting merged set of points out over MQTT to be used in 3rd party applications.

`merger\merger.exe`

### Visualizer
This application is used to preview the merged point cloud and adjust the relative alignment of data from multiple cameras. The viewer application is only needed for debugging and configuration/calibration. Once the system is set up correctly this application is not needed for normal use.

`viewer\Mqtt visualizer.exe`


# Physical Setup and Launch

## 1. Prepare MQTT Broker

Verify that your MQTT broker is running, this can be on the local computer or a remote computer so long as the network can handle a high volume of traffic (a dedicated wired network is recommended).


## 2.a Using a Single Azure Kinect

With a single Azure Kinect, you only need to connect the device normally (via a USB 3.0 port, and the power cable).


## 2.b Using Multiple Azure Kinects

If you are using multiple Azure Kinects it's recommended that you arrange them radially and aimed inward at a central point. This will give good coverage from multiple points of view around your subject.

For the best performance, connect all of the cameras to the same computer if possible, and run all of the applications on that same computer. If multiple computers are used, a wired gigabit network is recommended. When using multiple Azure Kinects, you must first determine the master/subordinate configuration and connect the sync cables accordingly.


1. To access the sync in and sync out ports, remove the white plastic housings using the torx wrench that was included with the Azure Kinect. The sync ports are located on the back of the device on either side of the USB and power ports.

1. Decide which Azure Kinect will be designated as the **master** and daisy chain the devices (using standard 3.5mm audio cables) so the sync-out port of the master connects to the next Azure Kinect sync-in port.

1. Connect the sync-out port of each Azure Kinect to the sync-in port of the next device until all the devices are connected.


### 3. Run the `Merger` application

The merger application should be running before any of the kinect capture applications are launched.


### 4. Run the `Kinect Capture` application(s)

Run the `kinect capture` application.

**NOTE:** If you are using multiple Azure Kinects, start the `kinect capture` instances of all **subordinate** devices first, and the **master** device last. This will ensure that frame synchronization is enabled.




### 5. Run the `visualizer` application

You should see a 3D preview of the points in the Visualizer window. If you do, the system is working and the point cloud data is being broadcast correctly on the MQTT topic `points/pointmerger`.