# Troubleshooting Tips

## The programs are saying they can't connect:

Ensure that the MQTT broker is actively running.

## Can't connect to a camera:

Ensure that the camera is properly connected and try viewing the camera(s) through the [Microsoft Kinect for Azure SDK](https://docs.microsoft.com/en-us/azure/kinect-dk/sensor-sdk-download)

You can run multiple instances of the Azure Kinect Viewer to verify that multiple Azure Kinect cameras can be run at the same time on your system (not all USB controllers work well with more than a single camera).

## Camera and merger are sending 0 points:

The camera may be configured such that all of the 3D points fall outside of the culling boundaries, try adjusting the camera position in the visualizer to see if this is the case.

Alternatively, you can increase the size of the bounding box (via `min` and `max` in the cameraposition.toml file). Make sure to change the threshold back to a reasonable size (if desired) once you have adjusted the camera to center the content correctly.

## No points present in the visualizer: 

Look at the top left of the screen to see if the pointCount is zero and it is showing cameras connected, try moving a camera source around in space. 

If there are no camera sources shown on the left side, pressing `M` will restart the system in the event that the viewer did not recieve the proper camera source(s).

Using the default settings, _usually_ the camera is too far forward, pressing `End` will bring the camera into frame if that is the case.

## Visualizer is not seeing all cameras:

Restart the system by pressing `M` to restart the system and allow for the visualizer to recieve the required startup messages from all of the cameras which may have been missed previously due to start up timing/sequencing.

## Visualizer wireframe thresholding box is not accurate

The thresholding box on the visualizer is simply a visual guide to align the cameras to a level space (i.e, the ground). The size of this wireframe box is not guaranteed to be the same as the min/max boundaries of each camera. If the min/max values of the visualizer and camera are not the same, they will not be aligned in the viewport.

## Merger isn't sending points while some cameras are running:

The Merger will only send points when it recieves a frame from each camera it has subscribed to. If one of the cameras stops sending points for some reason, the merger will stop sending frames since it will be waiting for the unresponsive camera.

If a camera has become unresponsive, try restarting the system (the `M` key in the the visualizer window).

## Visual studio says it can't find the libraries installed from Vcpkg:

Ensure that the target architecture is set to x64.

Ensure that you have run `vcpkg integrate install` after installing the dependencies, then restart Visual Studio.

## The program is running slowly

These applications rely heavily on compiler optimization to perform well, ensure that Visual Studio build configuration is set to `release` and not `debug`.