if not exist Assets\Plugins mkdir Assets\Plugins
set BODY_TRACKING_SDK_PATH=C:\Program Files\Azure Kinect Body Tracking SDK\
set BODY_TRACKING_TOOLS_PATH="%BODY_TRACKING_SDK_PATH%tools\"
set BODY_TRACKING_LIB_PATH="%BODY_TRACKING_SDK_PATH%sdk\netstandard2.0\release\"
copy %BODY_TRACKING_LIB_PATH%Microsoft.Azure.Kinect.BodyTracking.dll Assets\Plugins
copy %BODY_TRACKING_LIB_PATH%Microsoft.Azure.Kinect.BodyTracking.pdb Assets\Plugins
copy %BODY_TRACKING_LIB_PATH%Microsoft.Azure.Kinect.BodyTracking.deps.json Assets\Plugins
copy %BODY_TRACKING_LIB_PATH%Microsoft.Azure.Kinect.BodyTracking.xml Assets\Plugins
copy packages\Microsoft.Azure.Kinect.Sensor.1.4.1\lib\netstandard2.0\Microsoft.Azure.Kinect.Sensor.dll Assets\Plugins
copy packages\Microsoft.Azure.Kinect.Sensor.1.4.1\lib\netstandard2.0\Microsoft.Azure.Kinect.Sensor.pdb Assets\Plugins
copy packages\Microsoft.Azure.Kinect.Sensor.1.4.1\lib\netstandard2.0\Microsoft.Azure.Kinect.Sensor.deps.json Assets\Plugins
copy packages\Microsoft.Azure.Kinect.Sensor.1.4.1\lib\netstandard2.0\Microsoft.Azure.Kinect.Sensor.xml Assets\Plugins
copy %BODY_TRACKING_TOOLS_PATH%cublas64_11.dll Assets\Plugins
copy %BODY_TRACKING_TOOLS_PATH%cublasLt64_11.dll Assets\Plugins
copy %BODY_TRACKING_TOOLS_PATH%cudart64_110.dll Assets\Plugins
copy packages\System.Buffers.4.4.0\lib\netstandard2.0\System.Buffers.dll Assets\Plugins
copy packages\System.Memory.4.5.3\lib\netstandard2.0\System.Memory.dll Assets\Plugins
copy packages\System.Runtime.CompilerServices.Unsafe.4.5.2\lib\netstandard2.0\System.Runtime.CompilerServices.Unsafe.dll Assets\Plugins
copy packages\System.Reflection.Emit.Lightweight.4.6.0\lib\netstandard2.0\System.Reflection.Emit.Lightweight.dll Assets\Plugins
copy packages\Microsoft.Azure.Kinect.Sensor.1.4.1\lib\native\amd64\release\depthengine_2_0.dll Assets\Plugins
copy packages\Microsoft.Azure.Kinect.Sensor.1.4.1\lib\native\amd64\release\k4a.dll Assets\Plugins
copy packages\Microsoft.Azure.Kinect.Sensor.1.4.1\lib\native\amd64\release\k4arecord.dll Assets\Plugins
copy %BODY_TRACKING_TOOLS_PATH%onnxruntime.dll Assets\Plugins
copy %BODY_TRACKING_TOOLS_PATH%k4abt.dll Assets\Plugins
copy %BODY_TRACKING_TOOLS_PATH%cudnn64_8.dll .\
copy %BODY_TRACKING_TOOLS_PATH%cudnn_cnn_infer64_8.dll .\
copy %BODY_TRACKING_TOOLS_PATH%cudnn_ops_infer64_8.dll .\
copy %BODY_TRACKING_TOOLS_PATH%onnxruntime.dll .\
copy %BODY_TRACKING_TOOLS_PATH%dnn_model_2_0_op11.onnx .\
copy %BODY_TRACKING_TOOLS_PATH%cublas64_11.dll .\
copy %BODY_TRACKING_TOOLS_PATH%cublasLt64_11.dll .\
copy %BODY_TRACKING_TOOLS_PATH%cudart64_110.dll .\
copy %BODY_TRACKING_TOOLS_PATH%cufft64_10.dll .\