#Sample Unity Body Tracking Application

###Directions for getting started:

####1) First add these libraries to the Assets/Plugins folder:

- cublas64_100.dll
- cudart64_100.dll
- depthengine_2_0.dll
- k4a.dll
- k4abt.dll
- k4arecord.dll
- Microsoft.Azure.Kinect.BodyTracking.dll
- Microsoft.Azure.Kinect.Sensor.dll
- onnxruntime.dll
- System.Buffers.dll
- System.Numerics.Vectors.dll
- System.Memory.dll
- System.Runtime.CompilerServices.Unsafe.dll
- vcomp140.dll
- Microsoft.Azure.Kinect.BodyTracking.deps.json
- Microsoft.Azure.Kinect.BodyTracking.xml



####2)Then add these libraries to the sample_unity_bodytracking project root directory that contains the Assets folder

- cudnn64_7.dll
- onnxruntime.dll
- dnn_model_2_0.onnx


####3) Open the Unity Project and under Scenes/  select the Kinect4AzureSampleScene

![alt text](./UnitySampleGettingStarted.png)


Press play.

####If you wish to create a new scene just:

1) create a gameobject and add the component for the main.cs script
2) go to the prefab folder and drop in the Kinect4AzureTracker prefab
3) now drag the gameobject for the Kinect4AzureTracker onto the Tracker slot in the main object in the inspector.

