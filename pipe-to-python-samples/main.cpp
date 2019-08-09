// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <stdio.h>
#include <stdlib.h>
#include <k4a/k4a.h>

// NamedPipe
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

// Buffer size for client requests
#define BUFSIZE 512
// Buffer size for writing frame data to pipe
// Assuming depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED, change it otherwise
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 576
#define BYTES_PER_PIXEL 2
// Assuming streaming 2 channels: depth & ab, change it otherwise
#define FRAME_CHANNEL 2
#define FRAME_BUFSIZE FRAME_CHANNEL * FRAME_WIDTH * FRAME_HEIGHT * BYTES_PER_PIXEL


DWORD WINAPI InstanceThread(LPVOID);
VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD);

k4a_device_t device = NULL;

int _tmain(VOID)
{
    // Open Eden camera
    const int32_t TIMEOUT_IN_MS = 1000;

    uint32_t device_count = k4a_device_get_installed_count();

    if (device_count == 0)
    {
        printf("No K4A devices found\n");
        return 0;
    }

    if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
    {
        printf("Failed to open device\n");
        if (device != NULL)
        {
            k4a_device_close(device);
        }
    }

    // Setup camera modes
    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
    config.color_resolution = K4A_COLOR_RESOLUTION_2160P;
    config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    config.camera_fps = K4A_FRAMES_PER_SECOND_30;

    if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
    {
        printf("Failed to start device\n");
        if (device != NULL)
        {
            k4a_device_close(device);
        }
    }

    printf("Start to create NamedPipe Server!\n");

    // Create pipe server
    BOOL fConnected = FALSE;
    DWORD dwThreadId = 0;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
    LPTSTR lpszPipename = const_cast<LPSTR>(TEXT("\\\\.\\pipe\\mynamedpipe"));

    // The main loop creates an instance of the named pipe and
    // then waits for a client to connect to it. When the client
    // connects, a thread is created to handle communications
    // with that client, and this loop is free to wait for the
    // next client connect request. It is an infinite loop.

    for (;;)
    {
        _tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
        hPipe = CreateNamedPipe(lpszPipename,               // pipe name
            PIPE_ACCESS_DUPLEX,         // read/write access
            PIPE_TYPE_MESSAGE |         // message type pipe
            PIPE_READMODE_MESSAGE | // message-read mode
            PIPE_WAIT,              // blocking mode
            PIPE_UNLIMITED_INSTANCES,   // max. instances
            FRAME_BUFSIZE,              // output buffer size
            FRAME_BUFSIZE,              // input buffer size
            0,                          // client time-out
            NULL);                      // default security attribute

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            _tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
            return -1;
        }

        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.

        fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected)
        {
            printf("Client connected, creating a processing thread.\n");

            // Create a thread for this client.
            hThread = CreateThread(NULL,           // no security attribute
                0,              // default stack size
                InstanceThread, // thread proc
                (LPVOID)hPipe,  // thread parameter
                0,              // not suspended
                &dwThreadId);   // returns thread ID

            if (hThread == NULL)
            {
                _tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
                return -1;
            }
            else
                CloseHandle(hThread);
        }
        else
            // The client could not connect, so close the pipe.
            CloseHandle(hPipe);
    }

    if (device != NULL)
    {
        k4a_device_close(device);
    }

    return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
{
    HANDLE hHeap = GetProcessHeap();
    TCHAR *pchRequest = (TCHAR *)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
    TCHAR *pchReply = (TCHAR *)HeapAlloc(hHeap, 0, FRAME_BUFSIZE);

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    HANDLE hPipe = NULL;

    // Do some extra error checking since the app will keep running even if this
    // thread fails.

    if (lpvParam == NULL)
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
        printf("   InstanceThread exitting.\n");
        if (pchReply != NULL)
            HeapFree(hHeap, 0, pchReply);
        if (pchRequest != NULL)
            HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    if (pchRequest == NULL)
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchReply != NULL)
            HeapFree(hHeap, 0, pchReply);
        return (DWORD)-1;
    }

    if (pchReply == NULL)
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchRequest != NULL)
            HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    // Print verbose messages. In production code, this should be for debugging only.
    printf("InstanceThread created, receiving and processing messages.\n");

    // The thread's parameter is a handle to a pipe object instance.

    hPipe = (HANDLE)lpvParam;

    // Loop until done reading
    while (1)
    {
        // Read client requests from the pipe. This simplistic code only allows messages
        // up to BUFSIZE characters in length.
        fSuccess = ReadFile(hPipe,                   // handle to pipe
                            pchRequest,              // buffer to receive data
                            BUFSIZE * sizeof(TCHAR), // size of buffer
                            &cbBytesRead,            // number of bytes read
                            NULL);                   // not overlapped I/O

        if (!fSuccess || cbBytesRead == 0)
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                _tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
            }
            else
            {
                _tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
            }
            break;
        }

        // Process the incoming message.
        GetAnswerToRequest(pchRequest, pchReply, &cbReplyBytes);

        // Write the reply to the pipe.
        fSuccess = WriteFile(hPipe,        // handle to pipe
            pchReply,     // buffer to write from
            cbReplyBytes, // number of bytes to write
            &cbWritten,   // number of bytes written
            NULL);        // not overlapped I/O

        if (!fSuccess || cbReplyBytes != cbWritten)
        {
            _tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
            break;
        }
    }

    // Flush the pipe to allow the client to read the pipe's contents
    // before disconnecting. Then disconnect the pipe, and close the
    // handle to this pipe instance.

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    HeapFree(hHeap, 0, pchRequest);
    HeapFree(hHeap, 0, pchReply);

    printf("InstanceThread exitting.\n");
    return 1;
}

VOID GetAnswerToRequest(LPTSTR pchRequest,
    LPTSTR pchReply,
    LPDWORD pchBytes)
    // This routine is a simple function to print the client request to the console
    // and populate the reply buffer with a default data string. This is where you
    // would put the actual client request processing code that runs in the context
    // of an instance thread. Keep in mind the main thread will continue to wait for
    // and receive other client connections while the instance thread is working.
{
    _tprintf(TEXT("Client Request String:\"%s\"\n"), pchRequest);

    // Check the outgoing message to make sure it's not too long for the buffer.


    const int32_t TIMEOUT_IN_MS = 1000;
    k4a_capture_t capture = NULL;
    if (1)
    {
        k4a_image_t depth_image, ir_image;

        // Get a depth frame
        switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
        {
        case K4A_WAIT_RESULT_SUCCEEDED:
            break;
        case K4A_WAIT_RESULT_TIMEOUT:
            printf("Timed out waiting for a capture\n");
            break;
        case K4A_WAIT_RESULT_FAILED:
            printf("Failed to read a capture\n");
            return;
        }

        printf("Capture");

        // Probe for a IR16 and depth image
        depth_image = k4a_capture_get_depth_image(capture);
        ir_image = k4a_capture_get_ir_image(capture);
        if ((depth_image != NULL) && (ir_image != NULL))
        {
            printf(" | Depth16 res:%4dx%4d stride:%5d\n",
                k4a_image_get_height_pixels(depth_image),
                k4a_image_get_width_pixels(depth_image),
                k4a_image_get_stride_bytes(depth_image));

            printf(" | Ir16 res:%4dx%4d stride:%5d ",
                k4a_image_get_height_pixels(ir_image),
                k4a_image_get_width_pixels(ir_image),
                k4a_image_get_stride_bytes(ir_image));

            printf("\n");
            // Write depth image data to reply
            uint8_t *depth_image_buf = k4a_image_get_buffer(depth_image);
            size_t depth_buf_size = k4a_image_get_size(depth_image);
            printf("Write depth image to buffer, data size = %d\n", static_cast<int>(depth_buf_size));
            memcpy(&pchReply[0], depth_image_buf, depth_buf_size);

            // Write ir/ab image data to reply
            uint8_t *ir_image_buf = k4a_image_get_buffer(ir_image);
            size_t ir_buf_size = k4a_image_get_size(ir_image);
            printf("Write ir/ab image to buffer, data size = %d\n", static_cast<int>(ir_buf_size));
            memcpy(&pchReply[FRAME_BUFSIZE / 2], ir_image_buf, ir_buf_size);

            k4a_image_release(depth_image);
            k4a_image_release(ir_image);

        }
        else
        {
            printf(" | Ir16 or Depth None                    ");
            *pchBytes = 0;
            pchReply[0] = 0;
            printf("StringCchCopy failed, no outgoing message.\n");
        }

        *pchBytes = FRAME_BUFSIZE;

        // release capture
        k4a_capture_release(capture);
    }
}
