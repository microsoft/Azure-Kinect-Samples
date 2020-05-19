using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Microsoft.Azure.Kinect.BodyTracking;

public class PuppetAvatar : MonoBehaviour
{
    public TrackerHandler KinectDevice;
    Dictionary<JointId, Quaternion> absoluteOffsetMap;
    Animator PuppetAnimator;
    public GameObject RootPosition;
    public float OffsetY;
    public float OffsetZ;
    private static HumanBodyBones MapKinectJoint(JointId joint)
    {
        // https://docs.microsoft.com/en-us/azure/Kinect-dk/body-joints
        switch (joint)
        {
            case JointId.Pelvis: return HumanBodyBones.Hips;
            case JointId.SpineNavel: return HumanBodyBones.Spine;
            case JointId.SpineChest: return HumanBodyBones.Chest;
            case JointId.Neck: return HumanBodyBones.Neck;
            case JointId.Head: return HumanBodyBones.Head;
            case JointId.HipLeft: return HumanBodyBones.LeftUpperLeg;
            case JointId.KneeLeft: return HumanBodyBones.LeftLowerLeg;
            case JointId.AnkleLeft: return HumanBodyBones.LeftFoot;
            case JointId.FootLeft: return HumanBodyBones.LeftToes;
            case JointId.HipRight: return HumanBodyBones.RightUpperLeg;
            case JointId.KneeRight: return HumanBodyBones.RightLowerLeg;
            case JointId.AnkleRight: return HumanBodyBones.RightFoot;
            case JointId.FootRight: return HumanBodyBones.RightToes;
            case JointId.ClavicleLeft: return HumanBodyBones.LeftShoulder;
            case JointId.ShoulderLeft: return HumanBodyBones.LeftUpperArm;
            case JointId.ElbowLeft: return HumanBodyBones.LeftLowerArm;
            case JointId.WristLeft: return HumanBodyBones.LeftHand;
            case JointId.ClavicleRight: return HumanBodyBones.RightShoulder;
            case JointId.ShoulderRight: return HumanBodyBones.RightUpperArm;
            case JointId.ElbowRight: return HumanBodyBones.RightLowerArm;
            case JointId.WristRight: return HumanBodyBones.RightHand;
            default: return HumanBodyBones.LastBone;
        }
    }
    private void Start()
    {
        PuppetAnimator = GetComponent<Animator>();
        Transform _rootJointTransform = PuppetAnimator.GetBoneTransform(HumanBodyBones.Hips);

        absoluteOffsetMap = new Dictionary<JointId, Quaternion>();
        for (int i = 0; i < (int)JointId.Count; i++)
        {
            HumanBodyBones hbb = MapKinectJoint((JointId)i);
            if (hbb != HumanBodyBones.LastBone)
            {
                Transform transform = PuppetAnimator.GetBoneTransform(hbb);
                Quaternion absOffset = GetSkeletonBone(PuppetAnimator, transform.name).rotation;
                // find the absolute offset for the tpose
                while (!ReferenceEquals(transform, _rootJointTransform))
                {
                    transform = transform.parent;
                    absOffset = GetSkeletonBone(PuppetAnimator, transform.name).rotation * absOffset;
                }
                absoluteOffsetMap[(JointId)i] = absOffset;
            }
        }
    }

    private static SkeletonBone GetSkeletonBone(Animator animator, string boneName)
    {
        int count = 0;
        foreach (SkeletonBone sb in animator.avatar.humanDescription.skeleton)
        {
            if (sb.name == boneName)
            {
                return animator.avatar.humanDescription.skeleton[count];
            }
            count++;
        }
        return new SkeletonBone();
    }

    // Update is called once per frame
    private void LateUpdate()
    {
        for (int j = 0; j < (int)JointId.Count; j++)
        {
            if (MapKinectJoint((JointId)j) != HumanBodyBones.LastBone && absoluteOffsetMap.ContainsKey((JointId)j))
            {
                // get the absolute offset
                Quaternion absOffset = absoluteOffsetMap[(JointId)j];
                Transform finalJoint = PuppetAnimator.GetBoneTransform(MapKinectJoint((JointId)j));
                finalJoint.rotation = absOffset * Quaternion.Inverse(absOffset) * KinectDevice.absoluteJointRotations[j] * absOffset;
                if (j == 0)
                {
                    finalJoint.localPosition = new Vector3(RootPosition.transform.localPosition.x, RootPosition.transform.localPosition.y + OffsetY, RootPosition.transform.localPosition.z - OffsetZ);
                }
            }
        }
    }

}
