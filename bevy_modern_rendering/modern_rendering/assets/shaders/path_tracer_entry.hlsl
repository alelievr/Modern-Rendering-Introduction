RWTexture2D<float4> _Output : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    _Output[id.xy] = float4(1.0, 0.0, 0.0, 1.0);
}
