# Azure Kinect - OpenCV KinectFusion Sample

## Introduction

The Azure Kinect - OpenCV KinectFusion sample shows how to use the Azure Kinect SDK and device with the KinectFusion example from opencv_contrib's rgbd module (https://github.com/opencv/opencv_contrib/tree/master/modules/rgbd). This sample demonstrates how to feed calibration and undistorted depth images from Azure Kinect SDK to the OpenCV KinectFusion module. We render live KinectFusion results and generate fused point cloud as a ply file when quitting application.

If the user has OpenCV/OpenCV_Contrib/VTK installed, the OpenCV-specific code in this example can be enabled by uncommenting the HAVE_OPENCV pound define. The OpenCV code in this example has been tested using OpenCV 4.1.0. The following steps are what we tried on Windows (the user can take this as a reference and configure the right path of OpenCV dependencies):
- Download and install opencv-4.1.0 from https://opencv.org/releases/ (e.g. on windows, download the opencv-4.1.0-vc14_vc15.exe, and extract contents), then, copy opencv\build\include\* to the opencv-kinfu-samples\extern\opencv-4.1.0\include\* (please create extern directory and subdirectories accordingly)
- Download opencv_contrib-4.1.0 source from https://github.com/opencv/opencv_contrib/releases and extract contents, then copy opencv_contrib-4.1.0\modules\rgbd\include\* to extern\opencv_contrib-4.1.0\modules\rgbd\include\*, and copy opencv_contrib-4.1.0\modules\viz\include\* to extern\opencv_contrib-4.1\modules\viz\include\*
- Download VTK-8.2.0 from https://vtk.org/download/ and build the source accordingly
- Please follow the instruction from opencv_contrib (https://github.com/opencv/opencv_contrib) to build opencv with extra modules (we used cmake-gui to generate opencv sln file and built opencv and opencv_contrib modules with Visual Studio 2017, user needs to configure the WITH_VTK and VTK_DIR in the cmake-gui before generating sln)
- We pre-configured kinfu_example.vcxproj with opencv/opencv_contrib include/lib dependencies paths, the following is a list of dependencies we need for this sample:
    ```
    Includes:
        extern\opencv-4.1.0\include
        extern\opencv_contrib-4.1.0\modules\rgbd\include
        extern\opencv_contrib-4.1.0\modules\viz\include
    Libs:
        extern\lib\Debug\opencv_calib3d410d.lib
        extern\lib\Debug\opencv_core410d.lib
        extern\lib\Debug\opencv_highgui410d.lib
        extern\lib\Debug\opencv_imgproc410d.lib
        extern\lib\Debug\opencv_rgbd410d.lib
        extern\lib\Debug\opencv_viz410d.lib
        extern\lib\Release\opencv_calib3d410.lib
        extern\lib\Release\opencv_core410.lib
        extern\lib\Release\opencv_highgui410.lib
        extern\lib\Release\opencv_imgproc410.lib
        extern\lib\Release\opencv_rgbd410.lib
        extern\lib\Release\opencv_viz410.lib
    ```
- You need to copy the opencv/opencv_contrib dlls as well as VTK dlls to the Visual Studio output bin folder which contains kinfu_example.exe and Azure Kinect binaries before running the application.

## Usage Info

    Usage: kinfu_example.exe
    Keys:	q - Quit
        r - Reset KinFu
        v - Enable Viz Render Cloud (default is OFF, enable it slows down frame rate)
    The application will generate a file (kinectfusion_output.ply) in the same folder when you quit use key q

Example:

    Usage: kinfu_example.exe
