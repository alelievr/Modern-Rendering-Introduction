float4 vert(uint vertexId : SV_VertexID) : SV_POSITION
{
    // Determine the position based on vertex ID
    float2 v = float2(
        vertexId == 2 ? 3.0f : -1.0f,
        vertexId == 1 ? 3.0f : -1.0f
    );

    // Set the output position
    return float4(v, 0.1f, 1.0f);

    // return output;
}
