# Azure Kinect Floor Plane Detection Sample

## Introduction

The Azure Kinect Floor Detection sample demonstrates one way to get an estimate of the floor plane leveraging the Sensor SDK.

The approach taken assumes the floor is the lowest horizontal structure in the scene, and performs the following steps:

1. Use the IMU acceleration to determine when the device is not moving and to estimate gravity vector.
2. Detect floor plane elevation using the point cloud from a depth frame and the gravity vector as floor normal.

## Usage Info

```
floor_detector_sample.exe
```

## Instruction

### Basic Navigation:
* Rotate: Rotate the camera by moving the mouse while holding mouse left button
* Pan: Translate the scene by holding Ctrl key and drag the scene with mouse left button
* Zoom in/out: Move closer/farther away from the scene center by scrolling the mouse scroll wheel

### Key Shortcuts
* ESC: quit
* h: help
