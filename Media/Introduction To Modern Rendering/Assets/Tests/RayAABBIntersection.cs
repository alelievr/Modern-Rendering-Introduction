using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.Mathematics;
using static Unity.Mathematics.math;

[ExecuteAlways]
public class RayAABBIntersection : MonoBehaviour
{
    public Vector3 origin;

    public Vector3 end;

    public Bounds aabb;
    
    // Start is called before the first frame update
    void Start()
    {
        
    }
    
    // Test if segment specified by points p0 and p1 intersects AABB b
    int TestSegmentAABB(float3 p0, float3 p1, Bounds b)
    {
        float3 e = b.max - b.min;
        float3 d = p1 - p0;
        float3 m = p0 + p1 - (float3)b.min - (float3)b.max;// Try world coordinate axes as separating axes
        float adx = abs(d.x);
        if (abs(m.x) > e.x + adx) return 0;
        float ady = abs(d.y);
        if (abs(m.y) > e.y + ady) return 0;
        float adz = abs(d.z);
        if (abs(m.z) > e.z + adz) return 0;
// Add in an epsilon term to counteract arithmetic errors when segment is
// (near) parallel to a coordinate axis (see text for detail)
        adx += EPSILON; ady += EPSILON; adz += EPSILON;
// Try cross products of segment direction vector with coordinate axes
        if (abs(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady) return 0;
        if (abs(m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx) return 0;
        if (abs(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx) return 0;
// No separating axis found; segment must be overlapping AABB
        return 1;
    }

    // Update is called once per frame
    void Update()
    { 
        int a = TestSegmentAABB(origin, end, aabb);
        
        Debug.Log("Hit: " + a);
        Debug.DrawLine(origin, end, Color.blue);
        DrawBounds(aabb, Color.red);
    }
    
    private void DrawBounds(Bounds bounds, Color color)
    {
        // Get the corner points of the Bounds
        Vector3 center = bounds.center;
        Vector3 extents = bounds.extents;

        Vector3 p1 = center + new Vector3(-extents.x, -extents.y, -extents.z);
        Vector3 p2 = center + new Vector3(extents.x, -extents.y, -extents.z);
        Vector3 p3 = center + new Vector3(extents.x, -extents.y, extents.z);
        Vector3 p4 = center + new Vector3(-extents.x, -extents.y, extents.z);

        Vector3 p5 = center + new Vector3(-extents.x, extents.y, -extents.z);
        Vector3 p6 = center + new Vector3(extents.x, extents.y, -extents.z);
        Vector3 p7 = center + new Vector3(extents.x, extents.y, extents.z);
        Vector3 p8 = center + new Vector3(-extents.x, extents.y, extents.z);

        // Draw the bottom face
        Debug.DrawLine(p1, p2, color);
        Debug.DrawLine(p2, p3, color);
        Debug.DrawLine(p3, p4, color);
        Debug.DrawLine(p4, p1, color);

        // Draw the top face
        Debug.DrawLine(p5, p6, color);
        Debug.DrawLine(p6, p7, color);
        Debug.DrawLine(p7, p8, color);
        Debug.DrawLine(p8, p5, color);

        // Draw the sides
        Debug.DrawLine(p1, p5, color);
        Debug.DrawLine(p2, p6, color);
        Debug.DrawLine(p3, p7, color);
        Debug.DrawLine(p4, p8, color);
    }
}
