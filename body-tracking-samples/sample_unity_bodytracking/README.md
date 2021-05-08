# Sample Unity Body Tracking Application

### Directions for getting started:


#### 1) First get the latest nuget packages of libraries:

Open the sample_unity_bodytracking project in Unity.
Open the Visual Studio Solution associated with this project.
If there is no Visual Studio Solution yet you can make one by opening the Unity Editor
and selecting one of the csharp files in the project and opening it for editing.
You may also need to set the preferences->External Tools to Visual Studio

In Visual Studio:
Select Tools->NuGet Package Manager-> Package Manager Console

On the command line of the console at type the following command:

Install-Package Microsoft.Azure.Kinect.BodyTracking -Version 1.0.1

The body tracking libraries will be put in the Packages folder under sample_unity_bodytracking


#### 2) Next add these libraries to the Assets/Plugins folder:

You can do this by hand or just run the batch file MoveLibraryFile.bat in the sample_unity_bodytracking directory


From Packages/Microsoft.Azure.Kinect.BodyTracking.1.0.1/lib/netstandard2.0

- Microsoft.Azure.Kinect.BodyTracking.deps.json
- Microsoft.Azure.Kinect.BodyTracking.xml
- Microsoft.Azure.Kinect.BodyTracking.dll
- Microsoft.Azure.Kinect.BodyTracking.pdb

From Packages/Microsoft.Azure.Kinect.Sensor.1.3.0/lib/netstandard2.0

- Microsoft.Azure.Kinect.Sensor.deps.json
- Microsoft.Azure.Kinect.Sensor.xml
- Microsoft.Azure.Kinect.Sensor.dll
- Microsoft.Azure.Kinect.Sensor.pdb

From Packages/Microsoft.Azure.Kinect.BodyTracking.Dependencies.0.9.1/lib/native/amd64/release
- cublas64_100.dll
- cudart64_100.dll
- vcomp140.dll

From Packages/System.Buffers.4.4.0/lib/netstandard2.0

- System.Buffers.dll

From Packages/System.Memory.4.5.3/lib/netstandard2.0

- System.Memory.dll

From Packages/System.Reflection.Emit.Lightweight.4.6.0/lib/netstandard2.0

- System.Reflection.Emit.Lightweight.dll

From Packages/System.Runtime.CompilerServices.Unsafe.4.5.2/lib/netstandard2.0

- System.Runtime.CompilerServices.Unsafe.dll

From Packages/Microsoft.Azure.Kinect.Sensor.1.3.0/lib/native/amd64/release

- depthengine_2_0.dll
- k4a.dll
- k4arecord.dll

From Packages/Microsoft.Azure.Kinect.BodyTracking.1.0.1/lib/native/amd64/release

- k4abt.dll
- onnxruntime.dll



#### 2) Then add these libraries to the sample_unity_bodytracking project root directory that contains the Assets folder

From Packages/Microsoft.Azure.Kinect.BodyTracking.Dependencies.cuDNN.0.9.1/lib/native/amd64/release

- cudnn64_7.dll

From Packages/Microsoft.Azure.Kinect.BodyTracking.Dependencies.0.9.1/lib/native/amd64/release

- cublas64_100.dll
- cudart64_100.dll

From Packages/Microsoft.Azure.Kinect.BodyTracking.1.0.1/lib/native/amd64/release

- onnxruntime.dll

From Packages/Microsoft.Azure.Kinect.BodyTracking.1.0.1/content

- dnn_model_2_0.onnx


#### 3) Open the Unity Project and under Scenes/  select the Kinect4AzureSampleScene

![alt text](./UnitySampleGettingStarted.png)


Press play.

#### If you wish to create a new scene just:

1) create a gameobject and add the component for the main.cs script
2) go to the prefab folder and drop in the Kinect4AzureTracker prefab
3) now drag the gameobject for the Kinect4AzureTracker onto the Tracker slot in the main object in the inspector.

