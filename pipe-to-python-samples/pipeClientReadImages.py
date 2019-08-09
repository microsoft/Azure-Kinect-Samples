
# NamedPipe
import win32file

import numpy as np
import os, os.path

# For visualization
import cv2
import matplotlib.pyplot as plt


# LTN mode, depth/ab
FRAME_WIDTH = 640
FRAME_HEIGHT = 576

# Save the visualized frames or not
save_frames = False


if __name__ == "__main__":

    # Create pipe client
    fileHandle = win32file.CreateFile("\\\\.\\pipe\\mynamedpipe",
        win32file.GENERIC_READ | win32file.GENERIC_WRITE,
        0, None,
        win32file.OPEN_EXISTING,
        0, None)

    # For visualization
    cv2.namedWindow('vis', cv2.WINDOW_NORMAL)
    cv2.resizeWindow('vis', 800, 400)
    
    f_idx =0
    while True:
        # Send request to pipe server
        win32file.WriteFile(fileHandle, np.array([1]).tobytes())
        # Read reply data, need to be in same order/size as how you write them in the pipe server in pipe_streaming_example/main.cpp
        depth_data = win32file.ReadFile(fileHandle, FRAME_WIDTH*FRAME_HEIGHT*2)
        ab_data = win32file.ReadFile(fileHandle, FRAME_WIDTH*FRAME_HEIGHT*2)
        # Reshape for image visualization
        depth_img_full = np.frombuffer(depth_data[1], dtype=np.uint16).reshape(FRAME_HEIGHT, FRAME_WIDTH).copy()
        ab_img_full = np.frombuffer(ab_data[1], dtype=np.uint16).reshape(FRAME_HEIGHT, FRAME_WIDTH).copy()
        
        depth_vis = (plt.get_cmap("gray")(depth_img_full/512.0)[..., :3]*255.0).astype(np.uint8)
        ab_vis = (plt.get_cmap("gray")(ab_img_full/512.0)[..., :3]*255.0).astype(np.uint8)
        
        # Visualize the images
        vis = np.hstack([depth_vis, ab_vis])
   
        vis = cv2.cvtColor(vis, cv2.COLOR_BGR2RGB)
        
        f_idx = f_idx + 1

        cv2.imshow("vis", vis)
        if save_frames:
            vis_frames.append(vis)

        key = cv2.waitKey(1)
        if key == 27: # Esc key to stop
            break 

    if save_frames:
        for f_idx, vis in enumerate(vis_frames):
            cv2.imwrite("C:\Temp\F%06d.png"%f_idx, vis)

    win32file.CloseHandle(fileHandle)


