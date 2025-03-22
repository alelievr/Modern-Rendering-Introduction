#include "Common.hlsl"
#include "MeshUtils.hlsl"

Texture2D<uint> _VisibilityTexture : register(t1, space2);

struct ForwardMeshToFragment
{
    float4 positionCS : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ForwardMeshToFragment GetFullscreenTriangleVertex(uint id)
{
    ForwardMeshToFragment v;
    
    v.positionCS = GetFullScreenTriangleVertexPosition(id);
    v.uv = GetFullScreenTriangleTexCoord(id);

    return v;
}

struct BarycentricDeriv
{
    float3 m_lambda;
    float3 m_ddx;
    float3 m_ddy;
};

// http://filmicworlds.com/blog/visibility-buffer-rendering-with-material-graphs/
BarycentricDeriv CalcFullBary(float4 pt0, float4 pt1, float4 pt2, float2 pixelNdc, float2 two_over_windowsize)
{
    BarycentricDeriv ret;
    float3 invW = rcp(float3(pt0.w, pt1.w, pt2.w));
	//Project points on screen to calculate post projection positions in 2D
    float2 ndc0 = pt0.xy * invW.x;
    float2 ndc1 = pt1.xy * invW.y;
    float2 ndc2 = pt2.xy * invW.z;

	// Computing partial derivatives and prospective correct attribute interpolation with barycentric coordinates
	// Equation for calculation taken from Appendix A of DAIS paper:
	// https://cg.ivd.kit.edu/publications/2015/dais/DAIS.pdf

	// Calculating inverse of determinant(rcp of area of triangle).
    float invDet = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));

	//determining the partial derivatives
	// ddx[i] = (y[i+1] - y[i-1])/Determinant
    ret.m_ddx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
    ret.m_ddy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;
	// sum of partial derivatives.
    float ddxSum = dot(ret.m_ddx, float3(1, 1, 1));
    float ddySum = dot(ret.m_ddy, float3(1, 1, 1));
	
	// Delta vector from pixel's screen position to vertex 0 of the triangle.
    float2 deltaVec = pixelNdc - ndc0;

	// Calculating interpolated W at point.
    float interpInvW = invW.x + deltaVec.x * ddxSum + deltaVec.y * ddySum;
    float interpW = rcp(interpInvW);
	// The barycentric co-ordinate (m_lambda) is determined by perspective-correct interpolation. 
	// Equation taken from DAIS paper.
    ret.m_lambda.x = interpW * (invW[0] + deltaVec.x * ret.m_ddx.x + deltaVec.y * ret.m_ddy.x);
    ret.m_lambda.y = interpW * (0.0f + deltaVec.x * ret.m_ddx.y + deltaVec.y * ret.m_ddy.y);
    ret.m_lambda.z = interpW * (0.0f + deltaVec.x * ret.m_ddx.z + deltaVec.y * ret.m_ddy.z);

	//Scaling from NDC to pixel units
    ret.m_ddx *= two_over_windowsize.x;
    ret.m_ddy *= two_over_windowsize.y;
    ddxSum *= two_over_windowsize.x;
    ddySum *= two_over_windowsize.y;

    ret.m_ddy *= -1.0f;
    ddySum *= -1.0f;

	// This part fixes the derivatives error happening for the projected triangles.
	// Instead of calculating the derivatives constantly across the 2D triangle we use a projected version
	// of the gradients, this is more accurate and closely matches GPU raster behavior.
	// Final gradient equation: ddx = (((lambda/w) + ddx) / (w+|ddx|)) - lambda

	// Calculating interpW at partial derivatives position sum.
    float interpW_ddx = 1.0f / (interpInvW + ddxSum);
    float interpW_ddy = 1.0f / (interpInvW + ddySum);

	// Calculating perspective projected derivatives.
    ret.m_ddx = interpW_ddx * (ret.m_lambda * interpInvW + ret.m_ddx) - ret.m_lambda;
    ret.m_ddy = interpW_ddy * (ret.m_lambda * interpInvW + ret.m_ddy) - ret.m_lambda;

    return ret;
}

float3 InterpolateWithDeriv(BarycentricDeriv deriv, float v0, float v1, float v2)
{
    float3 mergedV = float3(v0, v1, v2);
    float3 ret;
    ret.x = dot(mergedV, deriv.m_lambda);
    ret.y = dot(mergedV, deriv.m_ddx);
    ret.z = dot(mergedV, deriv.m_ddy);
    return ret;
}

void InterpolateWithDeriv(BarycentricDeriv bary, float3 v0, float3 v1, float3 v2, out float3 v, out float3 v_ddx, out float3 v_ddy)
{
    float3 tvX = InterpolateWithDeriv(bary, v0.x, v1.x, v2.x);
    float3 tvY = InterpolateWithDeriv(bary, v0.y, v1.y, v2.y);
    float3 tvZ = InterpolateWithDeriv(bary, v0.z, v1.z, v2.z);

    v.x = tvX.x;
    v.y = tvY.x;
    v.z = tvZ.x;
    
    v_ddx.x = tvX.y;
    v_ddx.y = tvY.y;
    v_ddx.z = tvZ.y;
    
    v_ddy.x = tvX.z;
    v_ddy.y = tvY.z;
    v_ddy.z = tvZ.z;
}

void InterpolateWithDeriv(BarycentricDeriv bary, float2 v0, float2 v1, float2 v2, out float2 v, out float2 v_ddx, out float2 v_ddy)
{
    float3 tvX = InterpolateWithDeriv(bary, v0.x, v1.x, v2.x);
    float3 tvY = InterpolateWithDeriv(bary, v0.y, v1.y, v2.y);

    v.x = tvX.x;
    v.y = tvY.x;
    
    v_ddx.x = tvX.y;
    v_ddx.y = tvY.y;
    
    v_ddy.x = tvX.z;
    v_ddy.y = tvY.z;
}

[NumThreads(1, 1, 1)]
[OutputTopology("triangle")]
void mesh(
    out indices uint3 triangles[1],
    out vertices ForwardMeshToFragment vertices[3])
{
    SetMeshOutputCounts(3, 1);
    
    triangles[0] = uint3(0, 1, 2);
    vertices[0] = GetFullscreenTriangleVertex(0);
    vertices[1] = GetFullscreenTriangleVertex(1);
    vertices[2] = GetFullscreenTriangleVertex(2);
}

float4 fragment(ForwardMeshToFragment input) : SV_TARGET0
{
    uint visibilityData = _VisibilityTexture.Load(uint3(input.positionCS.xy, 0));
    
    uint visibleMeshetID, triangleID;
    DecodeVisibility(visibilityData, visibleMeshetID, triangleID);
    
    VisibleMeshlet visibleMeshlet = visibleMeshlets1[visibleMeshetID];
    Meshlet meshlet = meshlets[visibleMeshlet.meshletIndex];
    uint3 prim = LoadPrimitive(meshlet.triangleOffset, triangleID);
    
    InstanceData instance = LoadInstance(visibleMeshlet.instanceIndex);
    
    uint3 index = prim + meshlet.vertexOffset;
    uint index0 = meshletIndices[index.x];
    uint index1 = meshletIndices[index.y];
    uint index2 = meshletIndices[index.z];
    
    // Load And Transform vertices
    TransformedVertex attrib0 = LoadVertexAttributes(visibleMeshlet.meshletIndex, index0, visibleMeshlet.instanceIndex);
    TransformedVertex attrib1 = LoadVertexAttributes(visibleMeshlet.meshletIndex, index1, visibleMeshlet.instanceIndex);
    TransformedVertex attrib2 = LoadVertexAttributes(visibleMeshlet.meshletIndex, index2, visibleMeshlet.instanceIndex);
    
    float2 pixelNDC = input.positionCS.xy * cameraResolution.zw * 2 - 1;
    pixelNDC.y = -pixelNDC.y;
    BarycentricDeriv bary = CalcFullBary(attrib0.positionCS, attrib1.positionCS, attrib2.positionCS, pixelNDC, cameraResolution.zw * 2);
    
    // Interpolate
    float3 normal, normalDDX, normalDDY;
    InterpolateWithDeriv(bary, attrib0.normal, attrib1.normal, attrib2.normal, normal, normalDDX, normalDDY);
    float2 uv, uvDDX, uvDDY;
    InterpolateWithDeriv(bary, attrib0.uv, attrib1.uv, attrib2.uv, uv, uvDDX, uvDDY);
    
    // Shade material surface with lighting
    MaterialData material = materialBuffer.Load(instance.materialIndex);
    
    // Apply fog
    
    //return float4(GetRandomColor(visibleMeshetID), 1);
    //return float4(GetRandomColor(triangleID), 1);
    return float4(normal * 0.5 + 0.5, 1);
    //return float4(material.diffuseRoughness.xxx, 1);
}
