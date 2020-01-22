using System.Collections.Generic;
using UnityEngine;
using Microsoft.Azure.Kinect.BodyTracking;


public class TrackerHandler : MonoBehaviour
{
    Dictionary<JointId, int> parentJointMap;
    public bool drawSkeletons = true;

    // Start is called before the first frame update
    void Awake()
    {
        parentJointMap = new Dictionary<JointId, int>();
        parentJointMap[JointId.Pelvis] = -1;
        parentJointMap[JointId.SpineNavel] = 0;
        parentJointMap[JointId.SpineChest] = 1;
        parentJointMap[JointId.Neck] = 2;
        parentJointMap[JointId.ClavicleLeft] = 2;
        parentJointMap[JointId.ShoulderLeft] = 4;
        parentJointMap[JointId.ElbowLeft] = 5;
        parentJointMap[JointId.WristLeft] = 6;
        parentJointMap[JointId.HandLeft] = 7;
        parentJointMap[JointId.HandTipLeft] = 8;
        parentJointMap[JointId.ThumbLeft] = 8;
        parentJointMap[JointId.ClavicleRight] = 2;
        parentJointMap[JointId.ShoulderRight] = 11;
        parentJointMap[JointId.ElbowRight] = 12;
        parentJointMap[JointId.WristRight] = 13;
        parentJointMap[JointId.HandRight] = 14;
        parentJointMap[JointId.HandTipRight] = 15;
        parentJointMap[JointId.ThumbRight] = 15;
        parentJointMap[JointId.HipLeft] = 1;
        parentJointMap[JointId.KneeLeft] = 18;
        parentJointMap[JointId.AnkleLeft] = 19;
        parentJointMap[JointId.FootLeft] = 20;
        parentJointMap[JointId.HipRight] = 1;
        parentJointMap[JointId.KneeRight] = 22;
        parentJointMap[JointId.AnkleRight] = 23;
        parentJointMap[JointId.FootRight] = 24;
        parentJointMap[JointId.Head] = 0;
        parentJointMap[JointId.Nose] = -1;
        parentJointMap[JointId.EyeLeft] = -1;
        parentJointMap[JointId.EarLeft] = -1;
        parentJointMap[JointId.EyeRight] = -1;
        parentJointMap[JointId.EarRight] = -1;
    }

    public void updateTracker(BackgroundData trackerFrameData)
    {
        //this is an array in case you want to get the n closest bodies
        int[] closestBodies = new int[1] { 0 };
        findClosestTrackedBody(trackerFrameData, closestBodies);

        // render the closest body
        Body skeleton = trackerFrameData.Bodies[closestBodies[0]];
        renderSkeleton(skeleton, 0);
    }

    int findIndexFromId(BackgroundData frameData, int id)
    {
        int retIndex = -1;
        for (int i = 0; i < (int)frameData.NumOfBodies; i++)
        {
            if ((int)frameData.Bodies[i].Id == id)
            {
                retIndex = i;
                break;
            }
        }
        return retIndex;
    }

    private void findClosestTrackedBody(BackgroundData trackerFrameData, int[] bodies)
    {
        const float MAX_DISTANCE = 5000.0f;
        float[] minDistanceFromKinect = { MAX_DISTANCE };
        for (int i = 0; i < (int)trackerFrameData.NumOfBodies; i++)
        {
            var pelvisPosition = trackerFrameData.Bodies[i].JointPositions3D[(int)JointId.Pelvis];
            Vector3 pelvisPos = new Vector3((float)pelvisPosition.X, (float)pelvisPosition.Y, (float)pelvisPosition.Z);
            if (pelvisPos.magnitude < minDistanceFromKinect[0])
            {
                bodies[0] = i;
                minDistanceFromKinect[0] = pelvisPos.magnitude;
            }
        }
    }

    public void turnOnOffSkeletons()
    {
        drawSkeletons = !drawSkeletons;
        const int bodyRenderedNum = 0;
        for (int jointNum = 0; jointNum < (int)JointId.Count; jointNum++)
        {
            transform.GetChild(bodyRenderedNum).GetChild(jointNum).gameObject.GetComponent<MeshRenderer>().enabled = drawSkeletons;
            transform.GetChild(bodyRenderedNum).GetChild(jointNum).GetChild(0).GetComponent<MeshRenderer>().enabled = drawSkeletons;
        }
    }

    public void renderSkeleton(Body skeleton, int skeletonNumber)
    {
        for (int jointNum = 0; jointNum < (int)JointId.Count; jointNum++)
        {
            Vector3 jointPos = new Vector3(skeleton.JointPositions3D[jointNum].X, -skeleton.JointPositions3D[jointNum].Y, skeleton.JointPositions3D[jointNum].Z);
            Vector3 offsetPosition = transform.rotation * jointPos;
            Vector3 positionInTrackerRootSpace = transform.position + offsetPosition;
            Quaternion jointRot = new Quaternion(skeleton.JointRotations[jointNum].X, skeleton.JointRotations[jointNum].Y, skeleton.JointRotations[jointNum].Z, skeleton.JointRotations[jointNum].W);
            transform.GetChild(skeletonNumber).GetChild(jointNum).localPosition = jointPos;
            transform.GetChild(skeletonNumber).GetChild(jointNum).localRotation = jointRot;

            const int boneChildNum = 0;
            if (parentJointMap[(JointId)jointNum] != -1)
            {
                Vector3 parentTrackerSpacePosition = new Vector3(skeleton.JointPositions3D[parentJointMap[(JointId)jointNum]].X, -skeleton.JointPositions3D[parentJointMap[(JointId)jointNum]].Y, skeleton.JointPositions3D[parentJointMap[(JointId)jointNum]].Z);
                Vector3 boneDirectionTrackerSpace = jointPos - parentTrackerSpacePosition;
                Vector3 boneDirectionWorldSpace = transform.rotation * boneDirectionTrackerSpace;
                Vector3 boneDirectionLocalSpace = Quaternion.Inverse(transform.GetChild(skeletonNumber).GetChild(jointNum).rotation) * Vector3.Normalize(boneDirectionWorldSpace);
                transform.GetChild(skeletonNumber).GetChild(jointNum).GetChild(boneChildNum).localScale = new Vector3(1, 20.0f * 0.5f * boneDirectionWorldSpace.magnitude, 1);
                transform.GetChild(skeletonNumber).GetChild(jointNum).GetChild(boneChildNum).localRotation = Quaternion.FromToRotation(Vector3.up, boneDirectionLocalSpace);
                transform.GetChild(skeletonNumber).GetChild(jointNum).GetChild(boneChildNum).position = transform.GetChild(skeletonNumber).GetChild(jointNum).position - 0.5f * boneDirectionWorldSpace;
            }
            else
            {
                transform.GetChild(skeletonNumber).GetChild(jointNum).GetChild(boneChildNum).gameObject.SetActive(false);
            }
        }
    }

}
