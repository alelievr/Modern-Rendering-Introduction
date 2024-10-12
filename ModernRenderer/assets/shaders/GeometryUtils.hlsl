#pragma once

bool SolveQuadraticEquation(float a, float b, float c, out float2 roots)
{
    float det = b * b - 4.0 * a * c;

    float sqrtDet = sqrt(det);
    roots.x = (-b - sign(a) * sqrtDet) / (2.0 * a);
    roots.y = (-b + sign(a) * sqrtDet) / (2.0 * a);

    return (det >= 0.0);
}

// Assume Sphere is at the origin (i.e start = position - spherePosition)
bool IntersectRaySphere(float3 start, float3 dir, float radius, out float2 intersections)
{
    float a = dot(dir, dir);
    float b = dot(dir, start) * 2.0;
    float c = dot(start, start) - radius * radius;

    return SolveQuadraticEquation(a, b, c, intersections);
}
