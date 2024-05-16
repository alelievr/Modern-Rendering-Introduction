Texture2D<float> _Input;
RWTexture2D<float> _Output : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    _Output[id.xy] = 0;
}

// [numthreads(8, 8, 1)]
// void update(uint3 id : SV_DispatchThreadID)
// {
//     _Output[id.xy] = 0;
// }
