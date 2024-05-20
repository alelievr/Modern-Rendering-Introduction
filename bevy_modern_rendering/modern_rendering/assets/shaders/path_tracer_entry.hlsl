[[vk::binding(0, 0)]]
Texture2D<float> _Input;
[[vk::binding(0, 1)]]
RWTexture2D<float> _Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    _Output[id.xy] = 0.0;
}
