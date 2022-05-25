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

Update-Package -reinstall

The latest libraries will be put in the Packages folder under sample_unity_bodytracking


#### 2) Next make sure you have all the required DLLs for ONNX Runtime execution 

[Required dlls for ONNX Runtime execution](https://docs.microsoft.com/en-us/azure/kinect-dk/body-sdk-setup#required-dlls-for-onnx-runtime-execution-environments)

You can install an appropriate version of the CUDA/cuDNN/TRT and add a path to the PATH environment variable.


#### 3) Next add these libraries to the Assets/Plugins folder:

You can do this by hand or just run the batch file MoveLibraryFile.bat in the sample_unity_bodytracking directory

From Packages/Microsoft.Azure.Kinect.BodyTracking.1.1.2/lib/netstandard2.0

- Microsoft.Azure.Kinect.BodyTracking.deps.json
- Microsoft.Azure.Kinect.BodyTracking.xml
- Microsoft.Azure.Kinect.BodyTracking.dll
- Microsoft.Azure.Kinect.BodyTracking.pdb

From Packages/Microsoft.Azure.Kinect.BodyTracking.1.1.2/lib/native/amd64/release/

- k4abt.dll

From Packages/Microsoft.Azure.Kinect.BodyTracking.ONNXRuntime.1.10.0/lib/native/amd64/release

- directml.dll
- onnxruntime.dll
- onnxruntime_providers_cuda.dll
- onnxruntime_providers_shared.dll
- onnxruntime_providers_tensorrt.dll

From Packages/Microsoft.Azure.Kinect.Sensor.1.4.1/lib/netstandard2.0

- Microsoft.Azure.Kinect.Sensor.deps.json
- Microsoft.Azure.Kinect.Sensor.xml
- Microsoft.Azure.Kinect.Sensor.dll
- Microsoft.Azure.Kinect.Sensor.pdb

From Packages/Microsoft.Azure.Kinect.Sensor.1.4.1/lib/native/amd64/release

- depthengine_2_0.dll
- k4a.dll
- k4arecord.dll

From Packages/System.Buffers.4.4.0/lib/netstandard2.0

- System.Buffers.dll

From Packages/System.Memory.4.5.3/lib/netstandard2.0

- System.Memory.dll

From Packages/System.Reflection.Emit.Lightweight.4.6.0/lib/netstandard2.0

- System.Reflection.Emit.Lightweight.dll

From Packages/System.Runtime.CompilerServices.Unsafe.4.5.2/lib/netstandard2.0

- System.Runtime.CompilerServices.Unsafe.dll


#### 4) Then add these libraries to the sample_unity_bodytracking project root directory that contains the Assets folder

You can do this by hand or just run the batch file MoveLibraryFile.bat in the sample_unity_bodytracking directory

From Packages/Microsoft.Azure.Kinect.BodyTracking.1.1.2/content

- dnn_model_2_0_op11.onnx

From Packages/Microsoft.Azure.Kinect.BodyTracking.ONNXRuntime.1.10.0/lib/native/amd64/release

- directml.dll
- onnxruntime.dll
- onnxruntime_providers_cuda.dll
- onnxruntime_providers_shared.dll
- onnxruntime_providers_tensorrt.dll


#### 5) Then copy DirectML.dll to unity editor installation folder (...\Unity\Hub\Editor\version\Editor).

From Packages/Microsoft.Azure.Kinect.BodyTracking.ONNXRuntime.1.10.0/lib/native/amd64/release

- directml.dll


#### 6) Open the Unity Project and under Scenes/  select the Kinect4AzureSampleScene

![alt text](./UnitySampleGettingStarted.png)


Press play.

#### If you wish to create a new scene just:

1) create a gameobject and add the component for the main.cs script
2) go to the prefab folder and drop in the Kinect4AzureTracker prefab
3) now drag the gameobject for the Kinect4AzureTracker onto the Tracker slot in the main object in the inspector.


### Finally if you Build a Standalone Executable 

You will need to put these files in the same directory with the .exe:

- directml.dll
- onnxruntime.dll
- onnxruntime_providers_cuda.dll
- onnxruntime_providers_shared.dll
- onnxruntime_providers_tensorrt.dll
- dnn_model_2_0_op11.onnx

