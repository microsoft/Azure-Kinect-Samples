using UnityEngine;

public class CameraFlipper : MonoBehaviour
{
    Camera cam;

    [Tooltip("Flip by x axis")]
    public bool flipByX;

    void Start()
    {
        cam = GetComponent<Camera>();
    }

    // Flip fron camera to be aligned in directions with depth image on scene.
    void OnPreCull()
    {
        if (flipByX)
        {
            cam.ResetWorldToCameraMatrix();
            cam.ResetProjectionMatrix();
            Vector3 scale = new Vector3(-1, 1, 1);
            cam.projectionMatrix = cam.projectionMatrix * Matrix4x4.Scale(scale);
        }
    }

    void OnPreRender()
    {
        if (flipByX)
        {
            GL.invertCulling = true;
        }
    }

    void OnPostRender()
    {
        if (flipByX)
        {
            GL.invertCulling = false;
        }
    }
}
