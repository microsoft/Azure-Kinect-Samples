using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class debugTransform : MonoBehaviour
{
    public GameObject tracker;
    public int jointNum;
    public GameObject Joint;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        //Debug.Log("rot" + tracker.GetComponent<TrackerHandler>().GetRelativeJointRotation(Microsoft.Azure.Kinect.BodyTracking.JointId.ShoulderRight));
        //Quaternion Z_180_FLIP = new Quaternion(0.0f, 0.0f, 1.0f, 0.0f);
        transform.localRotation = tracker.GetComponent<TrackerHandler>().GetRelativeJointRotation((Microsoft.Azure.Kinect.BodyTracking.JointId)jointNum);
        //transform.rotation = Joint.transform.rotation;
    }
}
