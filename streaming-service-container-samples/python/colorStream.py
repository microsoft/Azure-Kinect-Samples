import asyncio
import websockets
import cv2
import numpy as np

async def client_socket():
        async with websockets.connect(
                'ws://localhost:8888/getStreams?device=k4a&stream=colorCamera&format=MJPG&resolution=720p&framerate=30') as clientSocket:
                 while True:
                        message = await clientSocket.recv()
                        data = np.frombuffer(message, dtype='uint8')
                        decimg = cv2.imdecode(data, 1)
                        cv2.imshow("Azure Kinect - Get Stream in Pyton Sample", decimg)

                        if cv2.waitKey(25) & 0xFF == ord('q'):
                                asyncio._get_running_loop().close()
                                exit()
                                break

asyncio.get_event_loop().run_until_complete(client_socket())
        
                


