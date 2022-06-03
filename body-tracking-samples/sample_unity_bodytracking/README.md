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

#### 2) Next add these libraries to the Assets/Plugins folder:

You can do this by hand or just **run the batch file MoveLibraryFile.bat** in the sample_unity_bodytracking directory

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


#### 3) Then add these libraries to the sample_unity_bodytracking project root directory that contains the Assets folder:

You can do this by hand or just **run the batch file MoveLibraryFile.bat** in the sample_unity_bodytracking directory

From Packages/Microsoft.Azure.Kinect.BodyTracking.1.1.2/content

- dnn_model_2_0_op11.onnx

From Packages/Microsoft.Azure.Kinect.BodyTracking.ONNXRuntime.1.10.0/lib/native/amd64/release

- directml.dll
- onnxruntime.dll
- onnxruntime_providers_cuda.dll
- onnxruntime_providers_shared.dll
- onnxruntime_providers_tensorrt.dll


#### 4) Next make sure you have all the [required DLLs for ONNX Runtime execution](https://docs.microsoft.com/en-us/azure/kinect-dk/body-sdk-setup#required-dlls-for-onnx-runtime-execution-environments):

First, download and install [Visual C++ Redistributable](https://docs.microsoft.com/en-us/azure/kinect-dk/body-sdk-setup#visual-c-redistributable-for-visual-studio-2015).

Additionally:

**For CUDA**:
* Download and install appropriate version of CUDA and make sure that CUDA_PATH exists as an environment variable (e.g C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.4).
* Download and install appropriate version of cuDNN and add a value to the PATH environment variable for it (e.g C:\Program Files\NVIDIA GPU Computing Toolkit\cuda-8.2.2.6\bin).

**For TensorRT**:
* Download and install appropriate version of CUDA and make sure that CUDA_PATH exists as an environment variable (e.g C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.4).
* Download and install appropriate version of TensorRT and add a value to the PATH environment variable for it (e.g C:\Program Files\NVIDIA GPU Computing Toolkit\TensorRT-8.2.1.8\lib).

**For DirectML**:
* Copy the **directml.dll** from the sample_unity_bodytracking folder to the unity editor directory (e.g C:\Program Files\Unity\Hub\Editor\2019.1.2f1\Editor)


#### 5) Then specify Execution Provider for the tracking:

In the ...\sample_unity_bodytracking\Assets\Scripts\SkeletalTrackingProvider.cs change the ProcessingMode to the one you want.

* TrackerProcessingMode.GPU (Defaults to DirectML for Windows)
* TrackerProcessingMode.CPU
* TrackerProcessingMode.Cuda
* TrackerProcessingMode.TensorRT
* TrackerProcessingMode.DirectML


#### 6) Open the Unity Project and under Scenes/  select the Kinect4AzureSampleScene:

![alt text](./UnitySampleGettingStarted.png)


Press play.


#### If you wish to create a new scene:

* Create a gameobject and add the component for the main.cs script.
* Go to the prefab folder and drop in the Kinect4AzureTracker prefab.
* Now drag the gameobject for the Kinect4AzureTracker onto the Tracker slot in the main object in the inspector.


### Finally if you Build a Standalone Executable:

You will need to put [required DLLs for ONNX Runtime execution](https://docs.microsoft.com/en-us/azure/kinect-dk/body-sdk-setup#required-dlls-for-onnx-runtime-execution-environments) in the same directory with the .exe:

You can copy ONNXRuntime and DirectML files from nuget package by hand or from sample_unity_bodytracking directory after running **the batch file MoveLibraryFile.bat** (Step #3)

For the CUDA/cuDNN/TensorRT DLLs (Step #4) you can either have them in the PATH environment variable or copy required set of DLLs from the installation locations:

e.g. 
* from C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.4\bin for the CUDA files.
* from C:\Program Files\NVIDIA GPU Computing Toolkit\cuda-8.2.2.6\bin for the cuDNN files.
* from C:\Program Files\NVIDIA GPU Computing Toolkit\TensorRT-8.2.1.8\lib for the TensorRT files.
