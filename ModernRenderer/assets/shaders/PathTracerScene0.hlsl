#include "Common.hlsl"
#include "GeometryUtils.hlsl"

RWTexture2D<float4> _Output : register(u0);

struct Sphere
{
    float3 position;
    float radius;
    float3 color;
};

struct Plane
{
    float3 position;
    float3 normal;
    float3 color;
};

struct Triangle
{
    float3 v0;
    float3 v1;
    float3 v2;
    float3 color;
};

void IntersectSphere(Sphere sphere, float3 rayDir, float3 rayOrigin, inout float currentDistance, inout float3 currentColor)
{
    float2 intersections;
    if (IntersectRaySphere(rayOrigin - sphere.position, rayDir, sphere.radius, intersections) && intersections.x > 0)
    {
        if (intersections.x < currentDistance)
        {
            currentDistance = intersections.x;
            currentColor = sphere.color;
        }
    }
}

void IntersectPlane(Plane plane, float3 rayDir, float3 rayOrigin, inout float currentDistance, inout float3 currentColor)
{
    float intersection;
    if (IntersectRayPlane(plane.position - rayOrigin, rayDir, plane.normal, intersection) && intersection > 0)
    {
        if (intersection < currentDistance)
        {
            currentDistance = intersection;
            currentColor = plane.color;
        }
    }
}

void IntersectTriangle(Triangle tri, float3 rayDir, float3 rayOrigin, inout float currentDistance, inout float3 currentColor)
{
    float3 uvw;
    if (IntersectRayTraiangle(rayOrigin, rayDir, tri.v0 - rayOrigin, tri.v1 - rayOrigin, tri.v2 - rayOrigin, uvw))
    {
        float3 pos = tri.v0 * uvw.x + tri.v1 * uvw.y + tri.v2 * uvw.z + rayOrigin;
        float distance = length(pos);
        if (distance < currentDistance)
        {
            currentDistance = distance;
            currentColor = tri.color;
        }
    }
}


void IntersectsScene(float3 rayDir, float3 rayOrigin, out float3 color)
{
    color = 0;
    float distance = 1e20;
    
    // Scene
    Sphere sphere = { float3(-2, 0, 0), 1, float3(0.9, 0.2, 0.2) };
    Sphere sphere2 = { float3(2, 2, 0), 1, float3(0.5, 0.2, 0.2) };
    Plane plane = { float3(0, -2, 0), float3(0, 1, 0), float3(0.2, 0.9, 0.2) };
    Triangle tri = { float3(-1, 0, 0), float3(1, 0, 0), float3(0, 1, 0), float3(0.2, 0.2, 0.9) };
    
    IntersectSphere(sphere, rayDir, rayOrigin, distance, color);
    IntersectSphere(sphere2, rayDir, rayOrigin, distance, color);
    IntersectPlane(plane, rayDir, rayOrigin, distance, color);
    IntersectTriangle(tri, rayDir, rayOrigin, distance, color);
}

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    uint2 positionSS = threadID.xy;
    float2 outputSize;
    _Output.GetDimensions(outputSize.x, outputSize.y);
    
    float3 positionNDC = float3((positionSS / outputSize) * 2 - 1, 1);
    positionNDC.y = -positionNDC.y; // TODO: investigate why this is needed

    float3 viewDirWS = TransformNDCToWorldDir(positionNDC);

    float3 rayDir = viewDirWS;
    float3 origin = cameraPosition.xyz;
    
    float3 color;
    IntersectsScene(rayDir, origin, color);
    _Output[positionSS] = float4(color, 1);
}