This sample illustrates how to stream Azure Kinect image data to python.
It first creates a pipe server in c++ to read depth and ab captures from Azure Kinect, and then on python code create a pipe client to read the images.

(1) PipeServer in c++: open pipe_streaming_example.sln in VisualStudio 2019, Release x64, build the solution, you can find 
     pipe_streaming_example.exe in F:\CNTKRepo\Azure-Kinect-Samples\pipe-to-python-samples\x64\Release

    (PS: it also works in VS2017, just need to change Platform Toolset from v142 to v141)

PS: It is created by combining the NamedPipe example from https://docs.microsoft.com/en-us/windows/win32/ipc/multithreaded-pipe-server
and Azure-Kinect-Sensor-SDK streaming example from https://github.com/microsoft/Azure-Kinect-Sensor-SDK/tree/develop/examples/streaming

() PipeClient in python (tested with Python3.6): pipeClientReadImages.py creates a pipe client to read the streamed images and visualize them in opencv, you need to:
     pip install matplotlib, opencv-python, Pypiwin32

Use on the command line as follows:

    pipe_streaming_example.exe // start a pipe server to stream depth/ab frames
    python.exe pipeClientReadImages.py // read the streamed images out in python, then do your stuff on these images in python, exit by pressing "Esc"....
