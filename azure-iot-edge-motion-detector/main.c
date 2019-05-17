// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

#include "iothub_module_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "iothubtransportmqtt.h"
#include "iothub.h"
#include "time.h"

#include "parson.h"
#include "k4a/k4a.h"

static int triggerDistance = 1500;
static int sendFrequencyInMin = 10;

typedef struct MOTION_DETECTOR_TAG
{
    char *serialNumber;
    int leftToRightCount;
    int rightToLeftCount;
} MotionDetector;

static void ModuleTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, size_t size, void *userContextCallback)
{
    printf("\r\nTwin callback called with (state=%s, size=%zu):\r\n%s\r\n",
          ENUM_TO_STRING(DEVICE_TWIN_UPDATE_STATE, updateState), size, payLoad);

    JSON_Value *root_value = json_parse_string(payLoad);
    JSON_Object *root_object = json_value_get_object(root_value);

    if (json_object_dotget_value(root_object, "desired.triggerDistance") != NULL)
    {
        triggerDistance = json_object_dotget_number(root_object, "desired.triggerDistance");
    }
    if (json_object_get_value(root_object, "triggerDistance") != NULL)
    {
        triggerDistance = json_object_get_number(root_object, "triggerDistance");
    }
    if (json_object_dotget_value(root_object, "desired.sendFrequencyInMinutes") != NULL)
    {
        sendFrequencyInMin = json_object_dotget_number(root_object, "desired.sendFrequencyInMinutes");
    }
    if (json_object_get_value(root_object, "sendFrequencyInMinutes") != NULL)
    {
        sendFrequencyInMin = json_object_get_number(root_object, "sendFrequencyInMinutes");
    }
}

static char *SerializeToJson(MotionDetector *motionDetector)
{
    char *result;

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    (void)json_object_set_string(root_object, "serialNumber", motionDetector->serialNumber);
    (void)json_object_dotset_number(root_object, "leftToRightCount", motionDetector->leftToRightCount);
    (void)json_object_dotset_number(root_object, "rightToLeftCount", motionDetector->rightToLeftCount);

    result = json_serialize_to_string(root_value);

    json_value_free(root_value);

    return result;
}

static void ReportedStateCallback(int statusCode, void *userContextCallback)
{
    (void)userContextCallback;
    printf("Module Twin reported properties update completed with result: %d\r\n", statusCode);
}

static int SetupCallbacksForModule(IOTHUB_MODULE_CLIENT_LL_HANDLE iotHubModuleClientHandle)
{
    int ret;

    if (IoTHubModuleClient_LL_SetModuleTwinCallback(iotHubModuleClientHandle, ModuleTwinCallback, (void *)iotHubModuleClientHandle) != IOTHUB_CLIENT_OK)
    {
        printf("ERROR: IoTHubModuleClient_LL_SetModuleTwinCallback(default)..........FAILED!\r\n");
        ret = __FAILURE__;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

static int CalculateAverageDepth(k4a_image_t depthImage, int positionX, int positionY, int width, int height)
{
    int count = 0;
    int total = 0;
    int average = 0;

    uint16_t *depthImageData = (uint16_t *)(void *)k4a_image_get_buffer(depthImage);
    int depthImageWidth = k4a_image_get_width_pixels(depthImage);

    for (int x = positionX - width / 2; x < positionX + width / 2; x++)
    {
        for (int y = positionY - height / 2; y < positionY + height / 2; y++)
        {
            uint16_t depthPixel = depthImageData[y * depthImageWidth + x];
            if (depthPixel != 0)
            {
                total += depthPixel;
                count++;
            }
        }
    }

    if (count != 0)
    {
        average = total / count;
    }

    return average;
}

static IOTHUB_MODULE_CLIENT_LL_HANDLE InitializeConnection()
{
    IOTHUB_MODULE_CLIENT_LL_HANDLE iotHubModuleClientHandle;

    if (IoTHub_Init() != 0)
    {
        printf("Failed to initialize the platform.\r\n");
        iotHubModuleClientHandle = NULL;
    }
    else if ((iotHubModuleClientHandle = IoTHubModuleClient_LL_CreateFromEnvironment(MQTT_Protocol)) == NULL)
    {
        printf("ERROR: IoTHubModuleClient_LL_CreateFromEnvironment failed\r\n");
    }
    else
    {
        // Uncomment the following lines to enable verbose logging.
        // bool traceOn = true;
        // IoTHubModuleClient_LL_SetOption(iotHubModuleClientHandle, OPTION_LOG_TRACE, &trace);
    }

    return iotHubModuleClientHandle;
}

static void DeInitializeConnection(IOTHUB_MODULE_CLIENT_LL_HANDLE iotHubModuleClientHandle)
{
    if (iotHubModuleClientHandle != NULL)
    {
        IoTHubModuleClient_LL_Destroy(iotHubModuleClientHandle);
    }
    IoTHub_Deinit();
}

static void iothub_module()
{
    IOTHUB_MODULE_CLIENT_LL_HANDLE iotHubModuleClientHandle;

    const int32_t TIMEOUT_IN_MS = 1000;
    const int32_t TIMEOUT_COUNT_TO_EXIT = 100;
    k4a_device_t device = NULL;
    k4a_capture_t capture = NULL;
    k4a_image_t depthImage = NULL;
    int depthImageWidth;
    int depthImageHeight;
    TICK_COUNTER_HANDLE tickHandle;
    tickcounter_ms_t currentMs = 0;
    int timeoutCount = 0;

    const int DEPTH_SAMPLE_WIDTH = 20;
    const int DEPTH_SAMPLE_HEIGHT = 10;
    bool triggered = false;

    setbuf(stdout, NULL);
    srand((unsigned int)time(NULL));

    tickHandle = tickcounter_create();

    uint32_t device_count = k4a_device_get_installed_count();

    if (device_count == 0)
    {
        printf("No K4A devices found\n");
        goto Exit;
    }

    if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
    {
        printf("Failed to open device\n");
        goto Exit;
    }

    char *serialNumber = NULL;
    size_t serialNumberLength = 0;

    if (K4A_BUFFER_RESULT_TOO_SMALL != k4a_device_get_serialnum(device, NULL, &serialNumberLength))
    {
        printf("Failed to get serial number length\n");
        goto Exit;
    }

    serialNumber = malloc(serialNumberLength);
    if (serialNumber == NULL)
    {
        printf("Failed to allocate memory for serial number (%zu bytes)\n", serialNumberLength);
        goto Exit;
    }

    if (K4A_BUFFER_RESULT_SUCCEEDED != k4a_device_get_serialnum(device, serialNumber, &serialNumberLength))
    {
        printf("Failed to get serial number\n");
        goto Exit;
    }

    printf("Device \"%s\"\n", serialNumber);

    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.depth_mode = K4A_DEPTH_MODE_NFOV_2X2BINNED;
    config.camera_fps = K4A_FRAMES_PER_SECOND_30;

    if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
    {
        printf("Failed to start cameras\n");
        goto Exit;
    }

    if ((iotHubModuleClientHandle = InitializeConnection()) != NULL && SetupCallbacksForModule(iotHubModuleClientHandle) == 0)
    {
        tickcounter_ms_t lastReportedTimeMs;
        MotionDetector motionDetector = {0};
        motionDetector.serialNumber = serialNumber;

        char *reportedProperties = SerializeToJson(&motionDetector);

        (void)IoTHubModuleClient_LL_SendReportedState(iotHubModuleClientHandle, (const unsigned char *)reportedProperties, strlen(reportedProperties), ReportedStateCallback, NULL);
        json_free_serialized_string(reportedProperties);

        tickcounter_get_current_ms(tickHandle, &currentMs);
        lastReportedTimeMs = currentMs;

        while (true)
        {
            // Get a depth frame.
            switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
            {
            case K4A_WAIT_RESULT_SUCCEEDED:
                timeoutCount = 0;
                break;
            case K4A_WAIT_RESULT_TIMEOUT:
                printf("Timed out waiting for a capture\n");
                timeoutCount++;
                if (timeoutCount > TIMEOUT_COUNT_TO_EXIT)
                {
                    printf("Timed out frame count exceeded, exiting.\n");
                    goto Exit;
                }
                continue;
                break;
            case K4A_WAIT_RESULT_FAILED:
                printf("Failed to read a capture\n");
                goto Exit;
            }

            // Get the depth image.
            depthImage = k4a_capture_get_depth_image(capture);
            if (depthImage == 0)
            {
                printf("Failed to get depth image from capture\n");
                goto Exit;
            }

            depthImageWidth = k4a_image_get_width_pixels(depthImage);
            depthImageHeight = k4a_image_get_height_pixels(depthImage);

            // Get the average depth value for an area to the left and to the right of the center.
            int leftDepthAverage = CalculateAverageDepth(depthImage, depthImageWidth / 2 - DEPTH_SAMPLE_WIDTH / 2, depthImageHeight / 2, DEPTH_SAMPLE_WIDTH, DEPTH_SAMPLE_HEIGHT);
            int rightDepthAverage = CalculateAverageDepth(depthImage, depthImageWidth / 2 + DEPTH_SAMPLE_WIDTH / 2, depthImageHeight / 2, DEPTH_SAMPLE_WIDTH, DEPTH_SAMPLE_HEIGHT);

            if (leftDepthAverage != 0 && rightDepthAverage != 0)
            {
                if (leftDepthAverage < triggerDistance && rightDepthAverage < triggerDistance)
                {
                    // There is something closer then triggerDistance in the center of the image, so enter the triggered state.
                    triggered = true;
                }
                else if (triggered)
                {
                    // If there was something in the center of the image, but now there is not then infer
                    // which direction it is moving by assuming it is still in either the left or right area.
                    if (leftDepthAverage < rightDepthAverage)
                    {
                        motionDetector.rightToLeftCount++;
                        printf("right to left movement detected\n");
                    }
                    else
                    {
                        motionDetector.leftToRightCount++;
                        printf("left to right movement detected\n");
                    }
                    triggered = false;
                }
            }

            k4a_image_release(depthImage);
            k4a_capture_release(capture);

            IoTHubModuleClient_LL_DoWork(iotHubModuleClientHandle);

            tickcounter_get_current_ms(tickHandle, &currentMs);
            if (currentMs - lastReportedTimeMs > 1000 * 60 * sendFrequencyInMin)
            {
                reportedProperties = SerializeToJson(&motionDetector);
                (void)IoTHubModuleClient_LL_SendReportedState(iotHubModuleClientHandle, (const unsigned char *)reportedProperties, strlen(reportedProperties), ReportedStateCallback, NULL);
                json_free_serialized_string(reportedProperties);
                lastReportedTimeMs = currentMs;

                // Only report the change in count since the last time it was reported.
                motionDetector.rightToLeftCount = 0;
                motionDetector.leftToRightCount = 0;
            }
        }
    }

Exit:

    if (device != NULL)
    {
        k4a_device_stop_cameras(device);
        k4a_device_close(device);
    }

    if (serialNumber)
    {
        free(serialNumber);
    }

    tickcounter_destroy(tickHandle);

    DeInitializeConnection(iotHubModuleClientHandle);
}

int main(void)
{
    iothub_module();
    return 0;
}
