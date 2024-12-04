using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Matrix05 : MonoBehaviour
{
    public Camera filmDeformationCam;

    Matrix04FrustumToBox matrix04FrustumToBox;
    // bool init = false;

    // Start is called before the first frame update
    void Start()
    {
        matrix04FrustumToBox = GetComponent<Matrix04FrustumToBox>();
    }

    // Update is called once per frame
    void Update()
    {
        var proj = GL.GetGPUProjectionMatrix(matrix04FrustumToBox.cam.projectionMatrix, false);
        float nearPlaneSize = Mathf.Abs(matrix04FrustumToBox.nearCorners[0].x - matrix04FrustumToBox.nearCorners[2].x);
        float farPlaneSize = Mathf.Abs(matrix04FrustumToBox.intermediateFarCorners[0].x - matrix04FrustumToBox.intermediateFarCorners[2].x);

        Shader.SetGlobalFloat("_ProjectionViewSlider", matrix04FrustumToBox.t);
        Shader.SetGlobalMatrix("_ViewProjectionMatrix", proj * matrix04FrustumToBox.cam.worldToCameraMatrix);
        Shader.SetGlobalMatrix("_InverseViewMatrix", matrix04FrustumToBox.cam.worldToCameraMatrix.inverse);
        Shader.SetGlobalFloat("_NearPlane", matrix04FrustumToBox.cam.nearClipPlane);
        Shader.SetGlobalFloat("_NearPlaneSize", nearPlaneSize);
        filmDeformationCam.orthographicSize = farPlaneSize / 2;
        filmDeformationCam.cullingMatrix = matrix04FrustumToBox.cam.projectionMatrix * matrix04FrustumToBox.cam.worldToCameraMatrix;

        // if (!init)
        // {
        //     matrix04FrustumToBox.cam.enabled = true;
        //     matrix04FrustumToBox.cam.targetTexture = output;
        //     matrix04FrustumToBox.cam.Render();
        //     matrix04FrustumToBox.cam.enabled = false;
        //     init = true;
        // }

    //     foreach (GameObject obj in objects)
    //     {
    //         var mr = obj.GetComponent<MeshRenderer>();
    //         var mf = obj.GetComponent<MeshFilter>();


    //         mr.material.SetF
    //     }
    }
}
