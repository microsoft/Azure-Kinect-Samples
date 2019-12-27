using Microsoft.Azure.Kinect.BodyTracking;
using System.Collections.Generic;

namespace Csharp_3d_viewer
{
    public static class JointConnections
    {
        public static readonly IReadOnlyDictionary<JointId, JointId> JointParent = new Dictionary<JointId, JointId>
        {
            { JointId.SpineNavel, JointId.Pelvis },
            { JointId.SpineChest, JointId.SpineNavel },
            { JointId.Neck, JointId.SpineChest },
            { JointId.ClavicleLeft, JointId.SpineChest },
            { JointId.ShoulderLeft, JointId.ClavicleLeft },
            { JointId.ElbowLeft, JointId.ShoulderLeft },
            { JointId.WristLeft, JointId.ElbowLeft },
            { JointId.HandLeft, JointId.WristLeft },
            { JointId.HandTipLeft, JointId.HandLeft },
            { JointId.ThumbLeft, JointId.WristLeft },
            { JointId.ClavicleRight, JointId.SpineChest },
            { JointId.ShoulderRight, JointId.ClavicleRight },
            { JointId.ElbowRight, JointId.ShoulderRight },
            { JointId.WristRight, JointId.ElbowRight },
            { JointId.HandRight, JointId.WristRight },
            { JointId.HandTipRight, JointId.HandRight },
            { JointId.ThumbRight, JointId.WristRight },
            { JointId.HipLeft, JointId.Pelvis },
            { JointId.KneeLeft, JointId.HipLeft },
            { JointId.AnkleLeft, JointId.KneeLeft },
            { JointId.FootLeft, JointId.AnkleLeft },
            { JointId.HipRight, JointId.Pelvis },
            { JointId.KneeRight, JointId.HipRight },
            { JointId.AnkleRight, JointId.KneeRight },
            { JointId.FootRight, JointId.AnkleRight },
            { JointId.Head, JointId.Neck },
            { JointId.Nose, JointId.Head },
            { JointId.EyeLeft, JointId.Nose },
            { JointId.EarLeft, JointId.EyeLeft },
            { JointId.EyeRight, JointId.Nose },
            { JointId.EarRight, JointId.EyeRight },
        };
    }
}
