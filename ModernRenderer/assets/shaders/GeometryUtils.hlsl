#pragma once

// Utility functions
bool SolveQuadraticEquation(float a, float b, float c, out float2 roots)
{
    float det = b * b - 4.0 * a * c;

    float sqrtDet = sqrt(det);
    roots.x = (-b - sign(a) * sqrtDet) / (2.0 * a);
    roots.y = (-b + sign(a) * sqrtDet) / (2.0 * a);

    return (det >= 0.0);
}

float ScalarTriple(float3 a, float3 b, float3 c)
{
    return dot(cross(a, b), c);
}

bool SameSign(float a, float b)
{
    return (a < 0 && b < 0) || (a > 0 && b > 0);
}

// Assume Sphere is at the origin (i.e start = position - spherePosition)
bool IntersectRaySphere(float3 start, float3 dir, float radius, out float2 intersections)
{
    float a = dot(dir, dir);
    float b = dot(dir, start) * 2.0;
    float c = dot(start, start) - radius * radius;

    return SolveQuadraticEquation(a, b, c, intersections);
}

// Assume that the plane is a the origin (i.e start = position - planePosition)
bool IntersectRayPlane(float3 rayOrigin, float3 rayDir, float3 planeNormal, out float distance)
{
    float lDotN = dot(planeNormal, rayDir);
    if (abs(lDotN) > 1e-6)
    {
        float3 p0l0 = rayOrigin;
        distance = dot(p0l0, planeNormal) / lDotN;
        return distance >= 0;
    }

    distance = 0;
    return false;
}


bool IntersectRayTraiangle(float3 rayOrigin, float3 rayDir, float3 v0, float3 v1, float3 v2, out float3 uvw)
{
    // Create vectors from p to triangle vertices
    float3 pa = v0 - rayOrigin;
    float3 pb = v1 - rayOrigin;
    float3 pc = v2 - rayOrigin;
    
    // Test if the line is inside the edges bc, ca and ab. Done by testing
    // that the signed parallelepiped volumes, computed using scalar triple
    // products, are all positive
    //uvw.x = ScalarTriple(rayDir, pc, pb);
    //if (uvw.x < 0.0f)
    //    return false;
    //uvw.y = ScalarTriple(rayDir, pa, pc);
    //if (uvw.y < 0.0f)
    //    return false;
    //uvw.z = ScalarTriple(rayDir, pb, pa);
    //if (uvw.z < 0.0f)
    //    return false;
    
    float3 m = cross(rayDir, pc);
    uvw.x = dot(pb, m);
    uvw.y = -dot(pa, m);
    if (!SameSign(uvw.x, uvw.y))
        return false;
    uvw.z = ScalarTriple(rayDir, pb, pa);
    if (!SameSign(uvw.x, uvw.z))
        return false;
    
    // Compute the barycentric coordinates (u, v, w) by normalizing the parallelepiped volumes.
    uvw *= 1.0f / (uvw.x + uvw.y + uvw.z);
    return true;
}

float DistanceToPlane(float4 plane, float3 p)
{
    return dot(float4(p, 1.0), plane);
}
 
// Returns > 0 if the sphere overlaps the frustum, <= 0 otherwise
float SphereFrustumTest(float4 planes[6], float3 center, float radius)
{
    float dist01 = min(DistanceToPlane(planes[0], center), DistanceToPlane(planes[1], center));
    float dist23 = min(DistanceToPlane(planes[2], center), DistanceToPlane(planes[3], center));
    float dist45 = min(DistanceToPlane(planes[4], center), DistanceToPlane(planes[5], center));
 
    return min(min(dist01, dist23), dist45) + radius;
}
