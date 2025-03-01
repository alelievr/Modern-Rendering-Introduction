#pragma once

struct OBB
{
    float3 right;
    float extentRight;
    float3 up;
    float extentUp;
    float3 center;
    float extentForward;
};

struct AABB
{
    float3 min;
    float3 max;
};

struct FrustumPlane
{
    float3 normal;
    float dist;
};

struct Frustum
{
    float3 normal0;
    float dist0;
    float3 normal1;
    float dist1;
    float3 normal2;
    float dist2;
    float3 normal3;
    float dist3;
    float3 normal4;
    float dist4;
    float3 normal5;
    float dist5;
    float4 corner0;
    float4 corner1;
    float4 corner2;
    float4 corner3;
    float4 corner4;
    float4 corner5;
    float4 corner6;
    float4 corner7;
};

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

float DistanceToPlane(float3 planeNormal, float planeDistance, float3 p)
{
    return dot(float4(p, 1.0), float4(planeNormal, planeDistance));
}
 
// Returns > 0 if the sphere overlaps the frustum, <= 0 otherwise
float SphereFrustumIntersection(Frustum frustum, float3 center, float radius)
{
    float dist01 = min(DistanceToPlane(frustum.normal0, frustum.dist0, center), DistanceToPlane(frustum.normal1, frustum.dist1, center));
    float dist23 = min(DistanceToPlane(frustum.normal2, frustum.dist2, center), DistanceToPlane(frustum.normal3, frustum.dist3, center));
    float dist45 = min(DistanceToPlane(frustum.normal4, frustum.dist4, center), DistanceToPlane(frustum.normal5, frustum.dist5, center));
 
    return min(min(dist01, dist23), dist45) + radius;
}

bool CheckOverlap(OBB obb, float3 planeNormal, float planeDistance)
{
    // Max projection of the half-diagonal onto the normal (always positive).
    float maxHalfDiagProj = obb.extentRight * abs(dot(planeNormal, obb.right))
        + obb.extentUp * abs(dot(planeNormal, obb.up))
        + obb.extentForward * abs(dot(planeNormal, cross(obb.up, obb.right)));

    // Positive distance -> center in front of the plane.
    // Negative distance -> center behind the plane (outside).
    float centerToPlaneDist = dot(planeNormal, obb.center) + planeDistance;

    // outside = maxHalfDiagProj < -centerToPlaneDist
    // outside = maxHalfDiagProj + centerToPlaneDist < 0
    // overlap = overlap && !outside
    return (maxHalfDiagProj + centerToPlaneDist >= 0);
}

bool FrustumOBBIntersection(OBB obb, Frustum frustum)
{
    // Test the OBB against frustum planes. Frustum planes are inward-facing.
    // The OBB is outside if it's entirely behind one of the frustum planes.
    // See "Real-Time Rendering", 3rd Edition, 16.10.2.
    bool overlap = CheckOverlap(obb, frustum.normal0, frustum.dist0);
    overlap = overlap && CheckOverlap(obb, frustum.normal1, frustum.dist1);
    overlap = overlap && CheckOverlap(obb, frustum.normal2, frustum.dist2);
    overlap = overlap && CheckOverlap(obb, frustum.normal3, frustum.dist3);
    overlap = overlap && CheckOverlap(obb, frustum.normal4, frustum.dist4);
    overlap = overlap && CheckOverlap(obb, frustum.normal5, frustum.dist5);
    
    // Test the frustum corners against OBB planes. The OBB planes are outward-facing.
    // The frustum is outside if all of its corners are entirely in front of one of the OBB planes.
    // See "Correct Frustum Culling" by Inigo Quilez.
    // We can exploit the symmetry of the box by only testing against 3 planes rather than 6.
    FrustumPlane planes[3];
    planes[0].normal = obb.right;
    planes[0].dist = obb.extentRight;
    planes[1].normal = obb.up;
    planes[1].dist = obb.extentUp;
    planes[2].normal = cross(obb.up, obb.right);
    planes[2].dist = obb.extentForward;

    for (int i = 0; overlap && i < 3; i++)
    {
        // We need a separate counter for the "box fully inside frustum" case.
        bool outsidePos = true; // Positive normal
        bool outsideNeg = true; // Reversed normal
        float proj = 0.0;

        // Merge 2 loops. Continue as long as all points are outside either plane.
        // Corner 0
        proj = dot(planes[i].normal, frustum.corner0.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Corner 1
        proj = dot(planes[i].normal, frustum.corner1.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Corner 2
        proj = dot(planes[i].normal, frustum.corner2.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Corner 3
        proj = dot(planes[i].normal, frustum.corner3.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Corner 4
        proj = dot(planes[i].normal, frustum.corner4.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Corner 5
        proj = dot(planes[i].normal, frustum.corner5.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Corner 6
        proj = dot(planes[i].normal, frustum.corner6.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Corner 7
        proj = dot(planes[i].normal, frustum.corner7.xyz - obb.center);
        outsidePos = outsidePos && (proj > planes[i].dist);
        outsideNeg = outsideNeg && (-proj > planes[i].dist);

        // Combine data of the previous plane
        overlap = overlap && !(outsidePos || outsideNeg);
    }

    return overlap;
}
