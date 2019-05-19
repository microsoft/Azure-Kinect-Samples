using System;
using System.Net.NetworkInformation;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using WebSocketSharp;

namespace FaceDetection
{
    class Program
    {
        static object   _lock = new object();
        static string   cameraUrl = "localhost";
        static string   framerate = "15";
        static bool     terminate = false;

        /// <summary>
        /// Main thread procedure to drive DisplayStream form message processing.
        /// </summary>
        /// <param name="arg"></param>
        static void DisplayStreamProc(object arg)
        {
            DisplayStream form = arg as DisplayStream;

            try
            {
                Application.Run(form);
            }
            catch (Exception exception)
            {
                System.Diagnostics.Debug.WriteLine(exception);
            }

            Console.WriteLine("Exiting DisplayForm processing thread.");
        }

        /// <summary>
        /// Thread procedure to receive Color image from Azure Kinect DK Sensor and send it to DisplayForm for processing. 
        /// </summary>
        /// <param name="arg"></param>
        static void ColorStreamProc(object arg)
        {
            DisplayStream displayStream = arg as DisplayStream;

            using (var ws = new WebSocket($"ws://{cameraUrl}:8888/getStreams?device=k4a&stream=colorCamera&format=MJPG&resolution=720p&framerate={framerate}"))
            {
                EventHandler<MessageEventArgs> eventHandler = (sender, e) =>
                {
                    if (e.IsText == true)
                    {
                        Console.WriteLine("Received text");
                    }
                    else if (e.IsBinary == true && displayStream.Visible == true)
                    {
                        try
                        {
                            displayStream.BeginInvoke(displayStream.displayImage, new object[] { e.RawData });
                        }
                        catch (Exception exception)
                        {
                            Console.WriteLine(exception.ToString());
                        }
                    }
                };

                ws.OnMessage += eventHandler;
                ws.Connect();

                lock (_lock)
                {
                    try
                    {
                        Console.WriteLine("Exiting Color Stream processing thread.");
                        ws.OnMessage -= eventHandler;
                        ws.Close(CloseStatusCode.Normal, "Closing C# Sample Application");
                    }
                    catch (Exception exception)
                    {
                        Console.WriteLine(exception.ToString());
                    }
                }
            }
        }

        /// <summary>
        /// Thread procedure to receive Depth stream from Azure Kinect DK Sensor and calculate distance from closest object.
        /// </summary>
        /// <param name="arg"></param>
        static void DepthStreamProc(object arg)
        {
            DisplayStream displayStream = arg as DisplayStream;

            using (var ws = new WebSocket($"ws://{cameraUrl}:8888/getStreams?device=k4a&stream=depthCamera&format=NFOVUnbinned&framerate={framerate}"))
            {
                EventHandler<MessageEventArgs> eventHandler = (sender, e) =>
                {
                    if (e.IsText == true)
                    {
                        Console.WriteLine("Received text");
                    }
                    else if (e.IsBinary == true && displayStream.Visible == true)
                    {
                        DepthCalculation calculation = new DepthCalculation();
                        UInt16 distance = calculation.CalculateDepth(e.RawData);

                        displayStream.distanceFromCamera = (UInt16)distance;
                    }
                }; ;

                ws.OnMessage += eventHandler;

                ws.Connect();
                lock (_lock)
                {
                    try
                    {
                        Console.WriteLine("Exiting Depth Stream processing thread.");
                        ws.OnMessage -= eventHandler;
                        ws.Close(CloseStatusCode.Normal, "Closing C# Sample Application");
                        Console.WriteLine("Done with Depth Stream.");
                    }
                    catch(Exception exception)
                    {
                        Console.WriteLine(exception.ToString());
                    }
                }
            }
        }

        static void Main(string[] args)
        {
            DisplayStream displayStream = new DisplayStream();

            Thread displayThread = new Thread(DisplayStreamProc);
            Thread colorStreamThread = new Thread(ColorStreamProc);
            Thread depthStreamThread = new Thread(DepthStreamProc);

            displayThread.Start(displayStream);

            // This is important from performance point of view to raise priority of depth stream processing. Depth stream is 
            // approximately 700 KB per frame and is bigger than color camera. In order to empty network buffers as fast as possible
            // for color camera not to have glitches, we have to increase thread priority.
            depthStreamThread.Priority = ThreadPriority.AboveNormal;
            lock (_lock)
            {
                colorStreamThread.Start(displayStream);
                depthStreamThread.Start(displayStream);
                displayThread.Join();
            }

            terminate = true;
            colorStreamThread.Join();
            depthStreamThread.Join();
        }
    }
}
