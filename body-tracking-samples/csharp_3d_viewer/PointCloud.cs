using Microsoft.Azure.Kinect.Sensor;
using System;
using System.Collections.Generic;
using System.Numerics;
using System.Runtime.InteropServices;

namespace Csharp_3d_viewer
{
    public static class PointCloud
    {
        private static Vector3[,] pointCloudCache;

        public static void ComputePointCloudCache(Calibration calibration)
        {
            using (var transformation = calibration.CreateTransformation())
            using (var fakeDepth = new Image(ImageFormat.Depth16, calibration.DepthCameraCalibration.ResolutionWidth, calibration.DepthCameraCalibration.ResolutionHeight))
            {
                // compute 3D points at z = 1000mm distance from the camera.
                MemoryMarshal.Cast<byte, ushort>(fakeDepth.Memory.Span).Fill(1000);
                using (var pointCloudImage = transformation.DepthImageToPointCloud(fakeDepth))
                {
                    var pointCloudBuffer = MemoryMarshal.Cast<byte, short>(pointCloudImage.Memory.Span);

                    pointCloudCache = new Vector3[calibration.DepthCameraCalibration.ResolutionHeight, calibration.DepthCameraCalibration.ResolutionWidth];
                    for (int k = 0, v = 0; v < calibration.DepthCameraCalibration.ResolutionHeight; ++v)
                    {
                        for (int u = 0; u < calibration.DepthCameraCalibration.ResolutionWidth; ++u, k += 3)
                        {
                            // Divide by 1e6 to store points position per each 1 millimeter of z-distance.
                            var point = new Vector3(pointCloudBuffer[k], pointCloudBuffer[k + 1], pointCloudBuffer[k + 2]) / 1000000;
                            pointCloudCache[v, u] = point;
                        }
                    }
                }
            }
        }

        public static void ComputePointCloud(Image depth, ref List<Vertex> pointCloud)
        {
            if (pointCloudCache == null)
            {
                throw new Exception("Must compute the point cloud cache before calling ComputePointCloud.");
            }

            if (pointCloud == null)
            {
                pointCloud = new List<Vertex>(depth.HeightPixels * depth.WidthPixels);
            }

            pointCloud.Clear();

            var depthPixels = depth.GetPixels<ushort>().ToArray();

            for (int v = 0, pixelIndex = 0; v < depth.HeightPixels; ++v)
            {
                for (int u = 0; u < depth.WidthPixels; ++u, ++pixelIndex)
                {
                    var depthInMillimeters = depthPixels[pixelIndex];

                    var positionPerMillimeterDepth = pointCloudCache[v, u];
                    if (depthInMillimeters > 0)
                    {
                        pointCloud.Add(new Vertex
                        {
                            Position = positionPerMillimeterDepth * depthInMillimeters,

                            // Computing normal on the CPU is expensive. Keeping it simple in this sample instead of making another Shader.
                            Normal = Vector3.UnitZ,
                        });
                    }
                }
            }
        }
    }
}
