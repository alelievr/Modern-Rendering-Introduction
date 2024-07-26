// Texture2D<float> _Input;
RWTexture2D<float> _Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    _Output[id.xy] = float4(1, 1, 0, 1);
}