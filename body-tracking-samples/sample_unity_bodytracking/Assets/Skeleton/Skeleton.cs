using Microsoft.Azure.Kinect.BodyTracking;
using System.Collections.Generic;
using UnityEngine;

public class Skeleton : MonoBehaviour
{
    #region Enumerations

    public enum Visibility
    {
        InFocus,
        OutOfFocus,
        Disappeared
    }

    // Holds list of body parts which exist in model but are not all followed by sensor.
    // Additional transformations should be applied after joints are moved (scale, align).
    public enum BodyParts
    {
        Head,
        Chest,
        RightBicep,
        LeftBicep,
        RightForeArm,
        LeftForeArm,
        Hips,
        RightThigh,
        LeftThigh,
        RightShin,
        LeftShin,
        RightHand,
        LeftHand
    }

    // BoneTransformation class represents single transformation which is going to be applied on body part.
    // On single body part can be applied chain of transformations.
    private class BoneTransformation
    {
        #region Body transformations
        public enum Transformations
        {
            // In case body part is attached to joint and move together with joint (head on neck).
            AttachToJoint,

            // In case body part is attached to position between joint and middle of other two joints(hips on model).
            AttachToPositionJointAndMiddleJoints,

            // In case body part should be scaled between two joints (bicep between elbow and shoulder).
            ScaleBetweenJoints,

            // In case body part should be scaled between joint and middle of two joints.
            ScaleBetweenJointAndMiddleJoints,

            //Align with direction between two joints.
            AlignJointToJoint,

            //Align with direction between two joint to middle between two joints.
            AlignJointToMiddleJoints,

            //Align with direction between middle to joints with between two joints.
            AlignMiddleJointsToMiddleJoints
        }
        #endregion

        // Reference to game object which is going to be deformed.
        public Transform bodyPart;

        // Actions which are going to be applied to body part.
        public Transformations action;

        // List of joints for given body part.
        public List<JointId> joints = new List<JointId>();

        // Axis between joints, calculate scale factor.
        public string aligningAxis;

        // Original bone size used to scale after reative joint positions changed.
        public float originalSize;

        // Original local scale in imported 3D model.
        public Vector3 originalLocalScale;

        public BoneTransformation(
            Transform bp,
            Transformations act,
            List<JointId> betweenJoints,
            string axis,
            float bs)
        {
            bodyPart = bp;
            action = act;
            joints = betweenJoints;
            aligningAxis = axis;
            originalSize = bs;
            originalLocalScale = bp.localScale;
        }
    }

    // Holds reference to transform object and original scale of joint.
    private class JointTransformation
    {
        public JointTransformation(Transform trans)
        {
            transformation = trans;
            originalLocalScale = trans.localScale;
        }

        public Transform transformation;
        public Vector3 originalLocalScale;
    }
    #endregion

    #region skin materials
    public Material inFocusMaterial;
    public Material outOfFocusMaterial;
    public List<Renderer> skin = new List<Renderer>();
    #endregion

    [Tooltip("True in case you want to hide bones on scene")]
    public bool hideBones;

    // Holds references to joints in model.
    private Dictionary<JointId, JointTransformation> jointCache = new Dictionary<JointId, JointTransformation>();

    // Holds original scale of joints.
    private Dictionary<JointId, Vector3> jointOriginalLocalScale = new Dictionary<JointId, Vector3>();

    // Holds references to transformations and body parts in model.
    private Dictionary<BodyParts, List<BoneTransformation>> bodyPartCache = new Dictionary<BodyParts, List<BoneTransformation>>();

    // Defines list of joints which are represented in 3D model.
    private List<JointId> jointsMapper = new List<JointId>()
        {
            JointId.Pelvis,
            JointId.EarRight,
            JointId.EarLeft,
            JointId.Head,
            JointId.Neck,
            JointId.ShoulderRight,
            JointId.ShoulderLeft,
            JointId.ElbowRight,
            JointId.ElbowLeft,
            JointId.WristRight,
            JointId.WristLeft,
            JointId.SpineNavel,
            JointId.HipRight,
            JointId.HipLeft,
            JointId.KneeRight,
            JointId.KneeLeft,
            JointId.AnkleRight,
            JointId.AnkleLeft
        };

    public List<JointId> JointsMapper => jointsMapper;

    public enum TrackerBody { closestBodyTracker1, closestBodyTracker2 };

    // Holds value of initial position. Used later to scale model in that way to preserve same ratio and layout.
    private Dictionary<JointId, Vector3> initialJointPositions = new Dictionary<JointId, Vector3>();

    // Holds value of the latest received position.
    private Dictionary<JointId, Vector3> currentJointPositions = new Dictionary<JointId, Vector3>();

    // Holds value of initial position.
    private Dictionary<BodyParts, Vector3> initialBodyPartPositions = new Dictionary<BodyParts, Vector3>();

    #region Unity callbacks
    // Start is called before the first frame update
    void Awake()
    {
        InitializeSkinParts();
        InitializeJoints();
        InitializeBodyParts();
        SaveInitialSkeletonPosition();
        ManageBonesVisibility();

        ChangeMaterial(outOfFocusMaterial);
    }
    #endregion

    // Cache all parts of body which are renderer.
    private void InitializeSkinParts()
    {
        foreach (Transform child in transform)
        {
            if (child.GetComponent<Renderer>() != null)
            {
                skin.Add(child.GetComponent<Renderer>());
            }
        }
    }

    // Changing material of all rendered data.
    private void ChangeMaterial(Material newMaterial)
    {
        foreach (Renderer renderer in skin)
        {
            renderer.material = newMaterial;
        }
    }

    // Invisible, holo and opaque.
    public void SetVisibility(Visibility visible)
    {
        if (visible == Visibility.InFocus)
        {
            gameObject.SetActive(true);
            ChangeMaterial(inFocusMaterial);
        }
        else if (visible == Visibility.OutOfFocus)
        {
            gameObject.SetActive(true);
            ChangeMaterial(outOfFocusMaterial);
        }
        else if (visible == Visibility.Disappeared)
        {
            gameObject.SetActive(false);
            ChangeMaterial(outOfFocusMaterial);
        }
    }

    #region Skeletal functions
    private void InitializeBodyParts()
    {
        bodyPartCache[BodyParts.RightHand] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("R_hand"),
            BoneTransformation.Transformations.AlignJointToJoint,
            new List<JointId> {
                JointId.ElbowRight,
                JointId.WristRight },
            "x",
            0.0f) };

        bodyPartCache[BodyParts.LeftHand] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("L_hand"),
            BoneTransformation.Transformations.AlignJointToJoint,
            new List<JointId> {
                JointId.ElbowLeft,
                JointId.WristLeft },
            "-x",
            0.0f) };

        bodyPartCache[BodyParts.Head] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("head"),
            BoneTransformation.Transformations.AttachToJoint,
            new List<JointId> { JointId.Neck },
            "x",
            0.0f),

            new BoneTransformation(
                transform.Find("head"),
                BoneTransformation.Transformations.AlignJointToMiddleJoints,
                new List<JointId> {
                    JointId.Neck,
                    JointId.EarRight,
                    JointId.EarLeft},
                "-y",
                0.0f)
        };

        bodyPartCache[BodyParts.Chest] = new List<BoneTransformation>(){
            new BoneTransformation(
                transform.Find("chest"),
                BoneTransformation.Transformations.ScaleBetweenJoints,
                new List<JointId> {
                    JointId.ShoulderLeft,
                    JointId.ShoulderRight },
                "xz",
                Vector3.Distance(
                    jointCache[JointId.ShoulderLeft].transformation.position,
                    jointCache[JointId.ShoulderRight].transformation.position)),

            new BoneTransformation(
                transform.Find("chest"),
                BoneTransformation.Transformations.ScaleBetweenJoints,
                new List<JointId> {
                    JointId.SpineNavel,
                    JointId.ShoulderRight},
                "y",
                Vector3.Distance(jointCache[JointId.ShoulderLeft].transformation.position,
                    jointCache[JointId.SpineNavel].transformation.position)),

            new BoneTransformation(
                transform.Find("chest"),
                BoneTransformation.Transformations.AttachToPositionJointAndMiddleJoints,
                new List<JointId> {
                    JointId.SpineNavel,
                    JointId.ShoulderLeft,
                    JointId.ShoulderRight },
                "x",
                0.0f),

            new BoneTransformation(
                transform.Find("chest"),
                BoneTransformation.Transformations.AlignJointToJoint,
                new List<JointId> {
                    JointId.ShoulderLeft,
                    JointId.ShoulderRight },
                "x",
                0.0f),
            new BoneTransformation(
                transform.Find("chest"),
                BoneTransformation.Transformations.AlignJointToJoint,
                new List<JointId> {
                    JointId.SpineNavel,
                    JointId.Pelvis },
                "y",
                0.0f)
        };

        bodyPartCache[BodyParts.Hips] = new List<BoneTransformation>(){

            new BoneTransformation(
                transform.Find("hips"),
                BoneTransformation.Transformations.ScaleBetweenJoints,
                new List<JointId> {
                    JointId.HipLeft,
                    JointId.HipRight },
                "x",
                Vector3.Distance(
                    jointCache[JointId.HipLeft].transformation.position,
                    jointCache[JointId.HipRight].transformation.position)),

           new BoneTransformation(
                transform.Find("hips"),
                BoneTransformation.Transformations.ScaleBetweenJoints,
                new List<JointId> {
                    JointId.SpineNavel,
                    JointId.HipRight },
                "yz",
                Vector3.Distance(
                    jointCache[JointId.SpineNavel].transformation.position,
                    jointCache[JointId.HipRight].transformation.position)),

            new BoneTransformation(
                transform.Find("hips"),
                BoneTransformation.Transformations.AlignJointToJoint,
                new List<JointId> {
                    JointId.HipLeft,
                    JointId.HipRight },
                "x",
                0.0f),
            new BoneTransformation(
                transform.Find("hips"),
                BoneTransformation.Transformations.AlignJointToMiddleJoints,
                new List<JointId> {
                    JointId.SpineNavel,
                    JointId.HipLeft,
                    JointId.HipRight },
                "y",
                0.0f),
            new BoneTransformation(
                transform.Find("hips"),
                BoneTransformation.Transformations.AttachToPositionJointAndMiddleJoints,
                new List<JointId> {
                    JointId.SpineNavel,
                    JointId.HipLeft,
                    JointId.HipRight },
                "x",
                0.0f)};

        bodyPartCache[BodyParts.RightBicep] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("R_bicep"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.ShoulderRight,
                JointId.ElbowRight },
            "x",
            Vector3.Distance(
                jointCache[JointId.ShoulderRight].transformation.position,
                jointCache[JointId.ElbowRight].transformation.position)) };

        bodyPartCache[BodyParts.RightForeArm] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("R_forearm"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.ElbowRight,
                JointId.WristRight },
            "x",
            Vector3.Distance(
                jointCache[JointId.ElbowRight].transformation.position,
                jointCache[JointId.WristRight].transformation.position)) };

        bodyPartCache[BodyParts.LeftBicep] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("L_bicep"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.ShoulderLeft,
                JointId.ElbowLeft },
            "-x",
            Vector3.Distance(jointCache[JointId.ShoulderLeft].transformation.position, jointCache[JointId.ElbowLeft].transformation.position)) };

        bodyPartCache[BodyParts.LeftForeArm] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("L_forearm"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.ElbowLeft,
                JointId.WristLeft},
            "-x",
            Vector3.Distance(jointCache[JointId.ElbowLeft].transformation.position, jointCache[JointId.WristLeft].transformation.position)) };

        bodyPartCache[BodyParts.RightThigh] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("R_thigh"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.HipRight,
                JointId.KneeRight },
            "y",
            Vector3.Distance(jointCache[JointId.HipRight].transformation.position, jointCache[JointId.KneeRight].transformation.position)) };

        bodyPartCache[BodyParts.LeftThigh] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("L_thigh"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.HipLeft,
                JointId.KneeLeft },
            "y",
            Vector3.Distance(jointCache[JointId.HipLeft].transformation.position, jointCache[JointId.KneeRight].transformation.position)) };

        bodyPartCache[BodyParts.RightShin] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("R_shin"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.KneeRight,
                JointId.AnkleRight },
            "y",
            Vector3.Distance(jointCache[JointId.KneeRight].transformation.position, jointCache[JointId.AnkleRight].transformation.position)) };

        bodyPartCache[BodyParts.LeftShin] = new List<BoneTransformation>(){
            new BoneTransformation(
            transform.Find("L_shin"),
            BoneTransformation.Transformations.ScaleBetweenJoints,
            new List<JointId> {
                JointId.KneeLeft,
                JointId.AnkleLeft},
            "y",
            Vector3.Distance(jointCache[JointId.KneeLeft].transformation.position, jointCache[JointId.AnkleLeft].transformation.position)) };
    }

    // Cache joints from model.
    // Model dependent.
    private void InitializeJoints()
    {
        jointCache[JointId.Pelvis] = new JointTransformation(transform.Find("pelvis"));
        jointCache[JointId.EarRight] = new JointTransformation(transform.Find("R_ear"));
        jointCache[JointId.EarLeft] = new JointTransformation(transform.Find("L_ear"));

        jointCache[JointId.Head] = new JointTransformation(transform.Find("head"));
        jointCache[JointId.Neck] = new JointTransformation(transform.Find("neck"));
        jointCache[JointId.ShoulderRight] = new JointTransformation(transform.Find("R_shoulder"));
        jointCache[JointId.ShoulderLeft] = new JointTransformation(transform.Find("L_shoulder"));
        jointCache[JointId.ElbowRight] = new JointTransformation(transform.Find("R_elbow"));
        jointCache[JointId.ElbowLeft] = new JointTransformation(transform.Find("L_elbow"));
        jointCache[JointId.WristRight] = new JointTransformation(transform.Find("R_hand"));
        jointCache[JointId.WristLeft] = new JointTransformation(transform.Find("L_hand"));
        jointCache[JointId.SpineNavel] = new JointTransformation(transform.Find("waist"));
        jointCache[JointId.HipRight] = new JointTransformation(transform.Find("R_hip"));
        jointCache[JointId.HipLeft] = new JointTransformation(transform.Find("L_hip"));
        jointCache[JointId.KneeRight] = new JointTransformation(transform.Find("R_knee"));
        jointCache[JointId.KneeLeft] = new JointTransformation(transform.Find("L_knee"));
        jointCache[JointId.AnkleRight] = new JointTransformation(transform.Find("R_ankle"));
        jointCache[JointId.AnkleLeft] = new JointTransformation(transform.Find("L_ankle"));

    }

    // Saves initial positon of skeleton before moving.
    private void SaveInitialSkeletonPosition()
    {
        foreach (KeyValuePair<JointId, JointTransformation> entry in jointCache)
        {
            initialJointPositions[entry.Key] = jointCache[entry.Key].transformation.position;
        }

        foreach (KeyValuePair<BodyParts, List<BoneTransformation>> entry in bodyPartCache)
        {
            initialBodyPartPositions[entry.Key] = bodyPartCache[entry.Key][0].bodyPart.position;
        }
    }

    // Move joints to new positions.
    private void MoveJoints()
    {
        foreach (KeyValuePair<JointId, JointTransformation> entry in jointCache)
        {
            entry.Value.transformation.position = ReadPositionOfJoint(entry.Key);
        }
    }

    // Move/scale/align body parts.
    // Move/scale/align body parts.
    private void MoveBones()
    {
        foreach (KeyValuePair<BodyParts, List<BoneTransformation>> entry in bodyPartCache)
        {
            foreach (BoneTransformation transformation in entry.Value)
            {
                BoneTransformation bodyPart = transformation;
                if (bodyPart.action == BoneTransformation.Transformations.AttachToJoint)
                {
                    AttachToJointTransformation(entry.Key, bodyPart);
                }
                else if (bodyPart.action == BoneTransformation.Transformations.AttachToPositionJointAndMiddleJoints)
                {
                    AttachToPositionJointAndMiddleJointsTransformation(entry.Key, bodyPart);
                }
                else if (bodyPart.action == BoneTransformation.Transformations.ScaleBetweenJoints)
                {
                    ScaleBetweenJointsTransformation(bodyPart);
                }
                else if (bodyPart.action == BoneTransformation.Transformations.ScaleBetweenJointAndMiddleJoints)
                {
                    ScaleBetweenJointAndMiddleJointsTransformation(bodyPart);
                }
                else if (bodyPart.action == BoneTransformation.Transformations.AlignJointToJoint)
                {
                    AlignTwoJointsTransformation(bodyPart);
                }
                else if (bodyPart.action == BoneTransformation.Transformations.AlignJointToMiddleJoints)
                {
                    AlignJointToMiddleJointsTransformation(bodyPart);
                }
            }
        }
    }

    // Finds average scale value of forearm and thigh and applies to joints.
    // All joints are uniformly scaled.
    private void RescaleJoints()
    {
        float scaleFactor = (FindBoneScaleFactor(BodyParts.LeftForeArm)
            + FindBoneScaleFactor(BodyParts.RightForeArm)
            + FindBoneScaleFactor(BodyParts.LeftThigh)
            + FindBoneScaleFactor(BodyParts.RightThigh)) / 4.0f;

        foreach (KeyValuePair<JointId, JointTransformation> entry in jointCache)
        {
            Vector3 ls = entry.Value.originalLocalScale;
            entry.Value.transformation.localScale = new Vector3(
                ls.x * scaleFactor,
                ls.y * scaleFactor,
                ls.z * scaleFactor);
        }
    }

    // Read new positions for joints and applies to appropriate joint new position.
    public void RepositionInWorld()
    {
        MoveJoints();
        MoveBones();
        RescaleJoints();
    }

    // Set position of bone to the middle of joints.
    private void SetBoneToPosition(BoneTransformation bodyPart, Vector3 start, Vector3 end)
    {
        bodyPart.bodyPart.position = (start + end) / 2;
    }

    // Align bone with direction.
    private void AlignWithAxis(
        BoneTransformation bodyPart,
        Vector3 start,
        Vector3 end,
        string axis)
    {
        Vector3 axisAlign = Vector3.zero;
        if (axis == "x")
        {
            axisAlign = bodyPart.bodyPart.right;
        }
        else if (axis == "-x")
        {
            axisAlign = -bodyPart.bodyPart.right;
        }
        else if (axis == "y")
        {
            axisAlign = -bodyPart.bodyPart.up;
        }
        else if (axis == "-y")
        {
            axisAlign = bodyPart.bodyPart.up;
        }

        // Allign bone with appropriate direction determined by joints.
        Vector3 v = end - start;
        var q = UnityEngine.Quaternion.FromToRotation(axisAlign, v);
        bodyPart.bodyPart.rotation = q * bodyPart.bodyPart.rotation;
    }

    // Scale bone to reflect new size between joints.
    private void ScaleBone(
        BoneTransformation bodyPart,
        Vector3 start,
        Vector3 end,
        string axis,
        float size)
    {
        MeshRenderer renderer = bodyPart.bodyPart.gameObject.GetComponent<MeshRenderer>();
        if (renderer != null)
        {
            float distance = Vector3.Distance(end, start);
            float scale = distance / size;

            float scaleX = 1.0f;
            float scaleY = 1.0f;
            float scaleZ = 1.0f;

            if (axis.Contains("x"))
            {
                scaleX = scale;
            }
            if (axis.Contains("y"))
            {
                scaleY = scale;
            }
            if (axis.Contains("z"))
            {
                scaleZ = scale;
            }
            Vector3 origScale = bodyPart.originalLocalScale;
            bodyPart.bodyPart.localScale = new Vector3(
                origScale.x * scaleX,
                origScale.y * scaleY,
                origScale.z * scaleZ);
        }
    }

    // Show or hide bones on skeleton.
    private void ManageBonesVisibility()
    {
        if (hideBones)
        {
            foreach (KeyValuePair<BodyParts, List<BoneTransformation>> entry in bodyPartCache)
            {
                entry.Value[0].bodyPart.gameObject.SetActive(false);
            }
        }
    }

    #region Transformation functions

    // Move object relativelly to attached.
    private void AttachToJointTransformation(
        BodyParts bodyPartKey,
        BoneTransformation bodyPart)
    {
        JointId attachedToJoint = bodyPart.joints[0];
        Vector3 delta = initialBodyPartPositions[bodyPartKey] - initialJointPositions[attachedToJoint];
        bodyPart.bodyPart.position = jointCache[attachedToJoint].transformation.position + delta;
    }

    private void AttachToPositionJointAndMiddleJointsTransformation(
        BodyParts bodyPartKey,
        BoneTransformation bodyPart)
    {
        JointId singleJoint = bodyPart.joints[0];
        JointId middleJoint1 = bodyPart.joints[1];
        JointId middleJoint2 = bodyPart.joints[2];
        Vector3 singleJointPosition = jointCache[singleJoint].transformation.position;
        Vector3 middleJointPosition = (jointCache[middleJoint1].transformation.position + jointCache[middleJoint2].transformation.position) / 2.0f;

        bodyPart.bodyPart.position = (singleJointPosition + middleJointPosition) / 2.0f;
    }

    // Rotate and scale gameobject by x axis between positions.
    private void ScaleBetweenJointsTransformation(BoneTransformation bodyPart)
    {
        JointId fromJoint = bodyPart.joints[0];
        JointId toJoint = bodyPart.joints[1];
        Vector3 start = jointCache[fromJoint].transformation.position;
        Vector3 end = jointCache[toJoint].transformation.position;
        string axis = bodyPart.aligningAxis;
        float size = bodyPart.originalSize;

        SetBoneToPosition(bodyPart, start, end);
        AlignWithAxis(bodyPart, start, end, axis);
        ScaleBone(bodyPart, start, end, axis, size);
    }

    // Rotate and scale gameobject by x axis between positions.
    private void ScaleBetweenJointAndMiddleJointsTransformation(BoneTransformation bodyPart)
    {
        JointId fromJoint = bodyPart.joints[0];
        JointId middleJoint1 = bodyPart.joints[1];
        JointId middleJoint2 = bodyPart.joints[2];
        Vector3 start = jointCache[fromJoint].transformation.position;
        Vector3 end = (jointCache[middleJoint1].transformation.position + jointCache[middleJoint2].transformation.position) / 2;
        string axis = bodyPart.aligningAxis;
        float size = bodyPart.originalSize;

        ScaleBone(bodyPart, start, end, axis, size);
    }

    // TODO refactor tobe one with AlignWithAxis.
    // Align object by given axis with another objects.
    private void AlignTwoJointsTransformation(BoneTransformation bodyPart)
    {
        JointId fromJoint = bodyPart.joints[0];
        JointId toJoint = bodyPart.joints[1];
        Vector3 start = jointCache[fromJoint].transformation.position;
        Vector3 end = jointCache[toJoint].transformation.position;
        string axis = bodyPart.aligningAxis;
        AlignWithAxis(bodyPart, start, end, axis);
    }

    private void AlignJointToMiddleJointsTransformation(BoneTransformation bodyPart)
    {
        JointId fromJoint = bodyPart.joints[0];
        JointId middleJoint1 = bodyPart.joints[1];
        JointId middleJoint2 = bodyPart.joints[2];
        Vector3 start = jointCache[fromJoint].transformation.position;
        Vector3 end = (jointCache[middleJoint1].transformation.position + jointCache[middleJoint2].transformation.position) / 2;
        string axis = bodyPart.aligningAxis;
        AlignWithAxis(bodyPart, start, end, axis);
    }

    // Rotate and scale gameobject by x axis between positions.
    private void ScaleTransformation(BoneTransformation bodyPart)
    {
        JointId fromJoint = bodyPart.joints[0];
        JointId toJoint = bodyPart.joints[1];
        Vector3 start = jointCache[fromJoint].transformation.position;
        Vector3 end = jointCache[toJoint].transformation.position;
        string axis = bodyPart.aligningAxis;
        float size = bodyPart.originalSize;

        SetBoneToPosition(bodyPart, start, end);
        AlignWithAxis(bodyPart, start, end, axis);
        ScaleBone(bodyPart, start, end, axis, size);
    }

    // TODO refactor tobe one with AlignWithAxis.
    // Align object by given axis with another objects.
    private void AlignTransformation(BoneTransformation bodyPart)
    {
        JointId fromJoint = bodyPart.joints[0];
        JointId toJoint = bodyPart.joints[1];
        Vector3 start = jointCache[fromJoint].transformation.position;
        Vector3 end = jointCache[toJoint].transformation.position;
        string axis = bodyPart.aligningAxis;
        AlignWithAxis(bodyPart, start, end, axis);
    }

    #endregion

    // Finds scale factor for given bone/part of body.
    private float FindBoneScaleFactor(BodyParts b)
    {
        if (bodyPartCache.ContainsKey(b))
        {
            BoneTransformation bodyPart = bodyPartCache[b].Find(item => item.action == BoneTransformation.Transformations.ScaleBetweenJoints || item.action == BoneTransformation.Transformations.ScaleBetweenJointAndMiddleJoints);
            if (bodyPart != null)
            {
                JointId fromJoint = bodyPart.joints[0];
                JointId toJoint = bodyPart.joints[1];
                Vector3 start = jointCache[fromJoint].transformation.position;
                Vector3 end = jointCache[toJoint].transformation.position;
                string axis = bodyPart.aligningAxis;
                float size = bodyPart.originalSize;

                MeshRenderer renderer = bodyPart.bodyPart.gameObject.GetComponent<MeshRenderer>();
                if (renderer != null)
                {
                    float distance = Vector3.Distance(end, start);
                    float scale = distance / size;
                    return scale;
                }
            }
        }

        return 0.0f;
    }

    // Read position of appropriate joint.
    private Vector3 ReadPositionOfJoint(JointId jointName)
    {
        if (currentJointPositions.ContainsKey(jointName))
        {
            return currentJointPositions[jointName];
        }

        return jointCache[jointName].transformation.position;
    }

    // TODO read this from config.
    // Center of body is lower part of belly.
    public Vector3 CenterOfMass()
    {
        return jointCache[JointId.Pelvis].transformation.position;
    }

    // Receive position of joints.
    public void ReceiveNewSensorData(SkeletonPosition newData)
    {
        currentJointPositions = newData.currentJointPositions;
    }
    #endregion
}