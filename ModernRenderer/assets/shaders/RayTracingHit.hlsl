struct RayPayload
{
    float3 color;
};

[shader("closesthit")]
void closest_red(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    float2 bary = attribs.barycentrics;
    payload.color = float3(bary, 0);
}

[shader("closesthit")]
void closest_green(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    CallShader(0, payload);
}