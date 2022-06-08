using Microsoft.Azure.CognitiveServices.Vision.Face;
using Microsoft.Azure.CognitiveServices.Vision.Face.Models;
using Microsoft.Azure.Kinect.Sensor;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Security.Principal;
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
        Device device;
        bool running = true;
        Presence lastPresence = Presence.NotInView;
        List<StatusRectangle> faces = new List<StatusRectangle>();
        IFaceClient client;
        Transformation transformation;

        public MainWindow()
        {
            InitializeComponent();
        }

        private async Task ProcessCapture(Capture capture)
        {
            BitmapSource originalBitmap = KinectHelper.GetBitmapSource(capture);

            int centerDepth = capture.Depth.GetPixel<ushort>(
                capture.Depth.HeightPixels / 2,
                capture.Depth.WidthPixels / 2);

            Presence currentPresence = KinectHelper.GetPresenceForDepth(centerDepth);
            lblStatus.Content = currentPresence + " " + centerDepth;

            if (currentPresence == Presence.JustRight && lastPresence != Presence.JustRight)
            {
                lblStatus.Content = "Loading...";
                using var depthMap = transformation.DepthImageToColorCamera(capture);

                Stream jpeg = BitmapHelper.GetJpegStream(originalBitmap);

                foreach (DetectedFace face in await FaceHelper.GetFaces(this.client, jpeg))
                {
                    StringBuilder text = new StringBuilder();

                    text.AppendLine($"Smile: {face.FaceAttributes.Smile}");
                    text.AppendLine($"Emotion: {FaceHelper.EmotionToString(face.FaceAttributes.Emotion)}");

                    int depth = depthMap.GetPixel<ushort>(
                        (int)face.FaceLandmarks.NoseTip.Y,
                        (int)face.FaceLandmarks.NoseTip.X);

                    text.AppendLine($"Depth: {depth}");

                    faces.Add(new StatusRectangle(face.FaceRectangle, text.ToString()));
                }
            }

            if (currentPresence != Presence.JustRight)
            {
                this.faces.Clear();
            }

            this.lastPresence = currentPresence;
            this.imgPreview.Source = BitmapHelper.GetRenderedBitmap(originalBitmap, this.faces);
#if false
            BitmapSource originalBitmap = KinectHelper.GetBitmapSource(capture);
         
            int centerDepth = capture.Depth.GetPixel<ushort>(
                capture.Depth.HeightPixels / 2,
                capture.Depth.WidthPixels / 2);

            Presence currentPresence = KinectHelper.GetPresenceForDepth(centerDepth);
            lblStatus.Content = currentPresence + " " + centerDepth;

            if (currentPresence == Presence.JustRight && lastPresence != Presence.JustRight)
            {
                lblStatus.Content = "Loading...";
                using var depthMap = transformation.DepthImageToColorCamera(capture);

                Stream jpeg = BitmapHelper.GetJpegStream(originalBitmap);

                foreach (DetectedFace face in await FaceHelper.GetFaces(this.client, jpeg))
                {
                    StringBuilder text = new StringBuilder();

                    text.AppendLine($"Smile: {face.FaceAttributes.Smile}");
                    text.AppendLine($"Emotion: {FaceHelper.EmotionToString(face.FaceAttributes.Emotion)}");

                    int depth = depthMap.GetPixel<ushort>(
                        (int)face.FaceLandmarks.NoseTip.Y, 
                        (int)face.FaceLandmarks.NoseTip.X);

                    text.AppendLine($"Depth: {depth}");

                    faces.Add(new StatusRectangle(face.FaceRectangle, text.ToString()));
                }
            }

            if (currentPresence != Presence.JustRight)
            {
                this.faces.Clear();
            }

            this.lastPresence = currentPresence;
            // this.imgPreview.Source = originalBitmap;
            this.imgPreview.Source = BitmapHelper.GetRenderedBitmap(originalBitmap, this.faces);
#endif
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            lblStatus.Content = "Loaded";

            this.device = Device.Open();

            DeviceConfiguration deviceConfiguration = new DeviceConfiguration()
            {
                CameraFPS = FPS.FPS15,
                ColorFormat = ImageFormat.ColorBGRA32,
                ColorResolution = ColorResolution.R720p,
                DepthMode = DepthMode.NFOV_2x2Binned,
                SynchronizedImagesOnly = true,
            };

            device.StartCameras(deviceConfiguration);

            this.transformation = device.GetCalibration().CreateTransformation();

            this.client = FaceHelper.Authenticate();

            while (running)
            {
                using (Capture capture = await Task.Run(() =>
                {
                    return device.GetCapture();
                }))
                {
                    await ProcessCapture(capture);
                }
            }

#if false
            this.device = Device.Open();
            
            DeviceConfiguration deviceConfiguration = new DeviceConfiguration()
            {
                CameraFPS = FPS.FPS15,
                ColorFormat = ImageFormat.ColorBGRA32,
                ColorResolution = ColorResolution.R720p,
                DepthMode = DepthMode.NFOV_2x2Binned,
                SynchronizedImagesOnly = true,
            };

            device.StartCameras(deviceConfiguration);

            this.transformation = device.GetCalibration().CreateTransformation();

            this.client = FaceHelper.Authenticate();

            while (running)
            {
                using (Capture capture = await Task.Run(() =>
                {
                    return device.GetCapture();
                }))
                {
                    await ProcessCapture(capture);
                }
            }
#endif
        }


        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            this.running = false;
            this.device.StopCameras();
#if false
            this.running = false;
            this.device.StopCameras();
#endif
        }
    }
}
