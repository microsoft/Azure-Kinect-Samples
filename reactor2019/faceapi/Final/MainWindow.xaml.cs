using Microsoft.Azure.CognitiveServices.Vision.Face;
using Microsoft.Azure.CognitiveServices.Vision.Face.Models;
using Microsoft.Azure.Kinect.Sensor;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Reflection.Metadata.Ecma335;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace AzureKinectFaceApi
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        bool running = true;
        Device device;
        Transformation transformation;
        IList<DetectedFace> detectedFaces = null;
        Presence lastPresence = Presence.NotInView;
        IFaceClient client;

        public MainWindow()
        {
            InitializeComponent();
        }
                
        private async Task ProcessCapture(Capture capture)
        {
            // Get a bitmap source for the original image
            BitmapSource bitmapSource = BitmapHelper.GetBitmapSource(capture);
            

            int centerDistance = capture.Depth.GetPixel<ushort>(
                capture.Depth.HeightPixels / 2,
                capture.Depth.WidthPixels / 2);

            
            Presence presence = PresenceMethods.GetPresenceForDepth(centerDistance);

            // Call the face API when someone enters the right spot
            if (presence == Presence.JustRight && this.lastPresence != Presence.JustRight)
            {
                this.detectedFaces = await GetFaces(bitmapSource);
            }
            // Clear the faces when someone leaves
            else if (presence != Presence.JustRight)
            {
                this.detectedFaces = null;
            }

            // Remember the presence to detect changes in the next image
            this.lastPresence = presence;

            // Show the current status
            this.lblStatus.Content = $"{presence} ({centerDistance})";

            // Build a list of rectangles to draw
            List<StatusRectangle> rectangles = new List<StatusRectangle>();
            if (detectedFaces != null)
            {
                // Map the depth pixels to the coordinate system of the color camera
                using var depthMap = transformation.DepthImageToColorCamera(capture);

                foreach (DetectedFace face in detectedFaces)
                {
                    // Find the current depth where the face was detected
                    ushort depth = depthMap.GetPixel<ushort>(
                        (int)face.FaceLandmarks.NoseTip.Y,
                        (int)face.FaceLandmarks.NoseTip.X);

                    StringBuilder textForFace = new StringBuilder();
                    textForFace.AppendLine($"Smile: { face.FaceAttributes.Smile }");
                    textForFace.AppendLine($"Emotion: { EmotionToString(face.FaceAttributes.Emotion) }");
                    textForFace.AppendLine($"Depth: { depth }");

                    rectangles.Add(new StatusRectangle(face.FaceRectangle, textForFace.ToString()));
                }
            }

            // Draw the image
            this.imgPreview.Source = BitmapBuilder.GetRenderedBitmap(bitmapSource, rectangles);
        }

        public static IFaceClient Authenticate(string endpoint, string key)
        {
            return new FaceClient(new ApiKeyServiceClientCredentials(key)) { Endpoint = endpoint };
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            string SUBSCRIPTION_KEY = Environment.GetEnvironmentVariable("FACE_SUBSCRIPTION_KEY");
            string ENDPOINT = Environment.GetEnvironmentVariable("FACE_ENDPOINT");

            // Prepare for using the Face API
            this.client = Authenticate(ENDPOINT, SUBSCRIPTION_KEY);

            // Open and start the Azure Kinect
            this.device = Device.Open();
            DeviceConfiguration configration = new DeviceConfiguration()
            {
                CameraFPS = FPS.FPS15,
                ColorResolution = ColorResolution.R720p,
                ColorFormat = ImageFormat.ColorBGRA32,
                DepthMode = DepthMode.NFOV_2x2Binned,
                SynchronizedImagesOnly = true
            };
            this.device.StartCameras(configration);

            // Get a transformation object to perform depth transforms
            this.transformation = new Transformation(this.device.GetCalibration());

            while (running)
            {
                // Wait for a new capture to be available
                using (Capture capture = await GetCaptureAsync())
                {
                    if (capture == null)
                        continue;

                    try
                    {
                        // Process the capture
                        await ProcessCapture(capture);

                    } catch (Exception ex)
                    {
                        this.lblStatus.Content = ex.Message;
                        await Task.Delay(1000);
                    }
                }
            }
        }
        
        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            lock (device)
            {
                running = false;
                device.StopCameras();
            }
        }
    }
}
