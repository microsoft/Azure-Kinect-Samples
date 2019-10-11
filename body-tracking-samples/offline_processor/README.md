# Azure Kinect Body Tracking OfflineProcessor Sample

## Introduction

The Azure Kinect Body Tracking OfflineProcessor sample demonstrates how to playback a recording Azure Kinect MKV file,
run through the body tracking SDK and store the body tracking results in a json file.

Notice: the current implementation is not the most efficient way to process a recording file offline. It simply
synchronously pushes and pops the capture to/from the tracker queue. To achieve the best performance, you would need to
create separate reader thread and process thread. And keep pushing new frames to the tracker without waiting for the
k4abt_pop_result to return successfully.

## Usage Info

```
offline_processor.exe <input_mkv_file> <output_json_file>
```
