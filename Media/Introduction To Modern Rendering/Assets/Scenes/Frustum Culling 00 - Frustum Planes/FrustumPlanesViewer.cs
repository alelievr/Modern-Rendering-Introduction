using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

[ExecuteAlways]
public class FrustumPlaneViewer : MonoBehaviour
{
    public Camera targetCamera;
    public Material planeMaterial;
    [Range(0, 5)] public int visiblePlaneIndex = 0;
    public float normalLength = 0.5f;
    public bool showAsPlanes = false; 
    public float planeScale = 2f;
    public bool show = true;

    // wireframe settings
    public Color wireColor = Color.white;
    public Color normalColor = Color.white;
    public float wireWidth = 0.025f;

    GameObject[]  planeGOs      = new GameObject[6];
    Mesh[]        planeMeshes   = new Mesh[6];
    Arrow[]       normalArrows  = new Arrow[6];
    List<Line>    wireLines     = new List<Line>();
    Vector3[]     corners       = new Vector3[8];

    Vector3 right;
    Vector3 up;

    // corner indices for each plane (BL, BR, TR, TL)
    static readonly int[,] planeCornerIdx = new int[6, 4]
    {
        {0,3,2,1}, // Near
        {4,5,6,7}, // Far
        {0,1,5,4}, // Left
        {3,7,6,2}, // Right
        {1,2,6,5}, // Top
        {0,4,7,3}, // Bottom
    };

    internal Vector3 selectedPlaneNormal
    {
        get
        {
            if (visiblePlaneIndex < 0 || visiblePlaneIndex >= normalArrows.Length)
                return Vector3.zero;
            return (normalArrows[visiblePlaneIndex].end - normalArrows[visiblePlaneIndex].start).normalized;
        }
    }
    
    internal Vector3 selectedPlaneCenter
    {
        get
        {
            if (visiblePlaneIndex < 0 || visiblePlaneIndex >= normalArrows.Length)
                return Vector3.zero;
            return normalArrows[visiblePlaneIndex].start;
        }
    }

    void OnEnable()
    {
        DestroyEverything();
        CreatePlanes();
        CreateWireframe();
    }

    void OnDisable()
    {
        DestroyEverything();
    }

    void DestroyEverything()
    {
        // destroy all children safely
        while (transform.childCount > 0)
        {
            var child = transform.GetChild(0).gameObject;
            child.SetActive(false);
            DestroyImmediate(child);
        }
        wireLines.Clear();
    }

    void Update()
    {
        if (targetCamera == null)
            return;

        UpdateFrustumCorners();

        for (int i = 0; i < 6; i++)
        {
            bool show = (i == visiblePlaneIndex);
            planeGOs[i].SetActive(show);
            normalArrows[i].gameObject.SetActive(show);
            UpdatePlaneMesh(i);
            UpdateNormalArrow(i);


            if (show)
            {
                planeMaterial.SetVector("_Right", right);
                planeMaterial.SetVector("_Up", up);
                Debug.DrawRay(Vector3.zero, right, Color.red);
                Debug.DrawRay(Vector3.zero, up, Color.blue);
            }
        }

        UpdateWireframe();

        if (!show)
        {
            foreach (var go in planeGOs)
                go.SetActive(false);
            foreach (var arr in normalArrows)
                arr.gameObject.SetActive(false);
        }
    }

    // ----- SETUP -----

    void CreatePlanes()
    {
        for (int i = 0; i < 6; i++)
        {
            var go = new GameObject($"Plane_{i}");
            go.transform.SetParent(transform, false);
            var mf = go.AddComponent<MeshFilter>();
            var mr = go.AddComponent<MeshRenderer>();
            if (planeMaterial)
                mr.sharedMaterial = planeMaterial;

            var mesh = new Mesh { name = $"FrustumPlaneMesh_{i}" };
            mf.sharedMesh = mesh;
            planeMeshes[i] = mesh;
            planeGOs[i]     = go;

            // arrow for normal, unparented so world coords are direct
            var arrowGO = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>();
            arrowGO.name = $"NormalArrow_{i}";
            normalArrows[i] = arrowGO.GetComponent<Arrow>();
        }
    }

    void CreateWireframe()
    {
        // 12 edges, instantiate unparented to avoid local offset
        for (int i = 0; i < 12; i++)
        {
            var lnGO = Instantiate(Resources.Load<Line>("Line"), transform);
            lnGO.name = $"WireLine_{i}";
            wireLines.Add(lnGO);
        }
    }

    // ----- FRUSTUM CORNERS -----

    void UpdateFrustumCorners()
    {
        var cam = targetCamera;
        Vector3[] nearC = new Vector3[4], farC = new Vector3[4];

        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), cam.nearClipPlane, Camera.MonoOrStereoscopicEye.Mono, nearC);
        cam.CalculateFrustumCorners(new Rect(0, 0, 1, 1), cam.farClipPlane,  Camera.MonoOrStereoscopicEye.Mono, farC);

        for (int i = 0; i < 4; i++)
        {
            corners[i]   = cam.transform.TransformPoint(nearC[i]);
            corners[i+4] = cam.transform.TransformPoint(farC[i]);
        }
    }

    // ----- PLANE MESH + NORMAL -----

    void UpdatePlaneMesh(int idx)
    {
        int i0 = planeCornerIdx[idx,0], i1 = planeCornerIdx[idx,1],
            i2 = planeCornerIdx[idx,2], i3 = planeCornerIdx[idx,3];

        var verts = new Vector3[] { corners[i0], corners[i1], corners[i2], corners[i3] };
        var tris  = new int[] { 0,1,2,  0,2,3 };
        var uvs = new Vector2[]
        {
            new Vector2(0, 0), // bottom-left
            new Vector2(1, 0), // bottom-right
            new Vector2(1, 1), // top-right
            new Vector2(0, 1)  // top-left
        };

        if (showAsPlanes)
        {
            right = (verts[1] - verts[0]).normalized;
            up = (verts[3] - verts[0]).normalized;

            Vector3 center = (verts[0] + verts[1] + verts[2] + verts[3]) * 0.25f;
            Vector3 diag0 = verts[0] - center;
            Vector3 diag1 = verts[1] - center;
            Vector3 diag2 = verts[2] - center;
            Vector3 diag3 = verts[3] - center;

            verts[0] += diag0 * planeScale;
            verts[1] += diag1 * planeScale;
            verts[2] += diag2 * planeScale;
            verts[3] += diag3 * planeScale;

            right = (verts[1] - verts[0]).normalized;
            up = (verts[3] - verts[0]).normalized;
        }

        var m = planeMeshes[idx];
        m.Clear();
        m.vertices  = verts;
        m.triangles = tris;
        m.uv        = uvs;
        m.RecalculateNormals();
        m.RecalculateBounds();

        // ensure the plane's transform doesn't offset the mesh
        planeGOs[idx].transform.localPosition = Vector3.zero;
        planeGOs[idx].transform.localRotation = Quaternion.identity;
        planeGOs[idx].transform.localScale    = Vector3.one;
    }

    void UpdateNormalArrow(int idx)
    {
        int i0 = planeCornerIdx[idx,0], i1 = planeCornerIdx[idx,1], i3 = planeCornerIdx[idx,3];

        Vector3 p0 = corners[i0];
        Vector3 p1 = corners[i1];
        Vector3 p3 = corners[i3];
        Vector3 center = (corners[planeCornerIdx[idx,0]] +
                          corners[planeCornerIdx[idx,1]] +
                          corners[planeCornerIdx[idx,2]] +
                          corners[planeCornerIdx[idx,3]]) * 0.25f;

        Vector3 right  = (p1 - p0).normalized;
        Vector3 up     = (p3 - p0).normalized;
        Vector3 normal = Vector3.Cross(right, up).normalized;

        var arr = normalArrows[idx];
        arr.start = center;
        arr.color = normalColor;
        arr.end   = center + normal * normalLength;
    }

    // ----- WIREFRAME -----

    void UpdateWireframe()
    {
        int[,] edges = new int[,]
        {
            {0,1},{1,2},{2,3},{3,0},  // near
            {4,5},{5,6},{6,7},{7,4},  // far
            {0,4},{1,5},{2,6},{3,7}   // sides
        };

        for (int i = 0; i < 12; i++)
        {
            var ln = wireLines[i];
            Vector3 a = corners[edges[i,0]];
            Vector3 b = corners[edges[i,1]];
            ln.dotted = false;
            ln.UpdateLine(a, b, false);
            ln.width = wireWidth;
            ln.color = wireColor;
        }
    }
}
