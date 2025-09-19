using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

[ExecuteAlways]
public class FrustumPlaneViewer : MonoBehaviour
{
    public Camera targetCamera;
    public Material planeMaterial;
    public GameObject arrowPrefab;
    [Range(0, 5)] public int visiblePlaneIndex = 0; // 0: Near, 1: Far, 2: Left, 3: Right, 4: Top, 5: Bottom

    GameObject[] planeObjects = new GameObject[6];
    Arrow[] normalArrows = new Arrow[6];
    Vector3[] corners = new Vector3[8];

    // corner layout: 0-3 near (BL, TL, TR, BR), 4-7 far (BL, TL, TR, BR)
    static readonly int[,] planeCornerIndices = new int[6, 4]
    {
        { 0, 3, 2, 1 }, // Near
        { 4, 5, 6, 7 }, // Far
        { 0, 1, 5, 4 }, // Left
        { 3, 7, 6, 2 }, // Right
        { 1, 2, 6, 5 }, // Top
        { 0, 4, 7, 3 }, // Bottom
    };

    void OnEnable()
    {
        CreatePlaneVisuals();
    }

    void OnDisable()
    {
        foreach (var obj in planeObjects)
            if (obj) DestroyImmediate(obj);
        foreach (var arrow in normalArrows)
            if (arrow) DestroyImmediate(arrow.gameObject);
    }

    void Update()
    {
        if (!targetCamera)
            return;

        UpdateFrustumCorners();

        for (int i = 0; i < 6; i++)
        {
            planeObjects[i].SetActive(i == visiblePlaneIndex);
            normalArrows[i].gameObject.SetActive(i == visiblePlaneIndex);
            UpdatePlane(planeObjects[i], normalArrows[i], i);
        }
    }

    void CreatePlaneVisuals()
    {
        for (int i = 0; i < 6; i++)
        {
            var plane = GameObject.CreatePrimitive(PrimitiveType.Quad);
            plane.name = "Plane_" + i;
            plane.transform.SetParent(transform);
            planeObjects[i] = plane;
            if (planeMaterial)
                plane.GetComponent<MeshRenderer>().sharedMaterial = planeMaterial;

            var arrowGO = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>();
            arrowGO.name = "NormalArrow_" + i;
            normalArrows[i] = arrowGO.GetComponent<Arrow>();
        }
    }

    void UpdateFrustumCorners()
    {
        var cam = targetCamera;
        var near = cam.nearClipPlane;
        var far = cam.farClipPlane;

        Vector3[] nearCorners = new Vector3[4];
        Vector3[] farCorners = new Vector3[4];
        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), near, Camera.MonoOrStereoscopicEye.Mono, nearCorners);
        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), far, Camera.MonoOrStereoscopicEye.Mono, farCorners);

        for (int i = 0; i < 4; i++)
        {
            corners[i] = cam.transform.TransformPoint(nearCorners[i]);     // 0-3
            corners[i + 4] = cam.transform.TransformPoint(farCorners[i]);  // 4-7
        }
    }

    void UpdatePlane(GameObject planeGO, Arrow arrow, int index)
    {
        Vector3 p0 = corners[planeCornerIndices[index, 0]];
        Vector3 p1 = corners[planeCornerIndices[index, 1]];
        Vector3 p2 = corners[planeCornerIndices[index, 2]];
        Vector3 p3 = corners[planeCornerIndices[index, 3]];

        Vector3 center = (p0 + p1 + p2 + p3) / 4f;
        Vector3 right = (p1 - p0).normalized;
        Vector3 up = (p3 - p0).normalized;
        Vector3 normal = Vector3.Cross(right, up).normalized;

        float width = Vector3.Distance(p0, p1);
        float height = Vector3.Distance(p0, p3);

        planeGO.transform.position = center;
        planeGO.transform.rotation = Quaternion.LookRotation(normal, up);
        planeGO.transform.localScale = new Vector3(width, height, 1f);

        // Set up arrow to show normal
        arrow.start = center;
        arrow.end = center + normal * 0.5f;
    }
}
