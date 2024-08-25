RWTexture2D<float4> _Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float2 uv = (threadID.xy + 0.5) / 256;
    _Output[threadID.xy] = float4(uv.x%1, uv.y%1, 0, 1);
}