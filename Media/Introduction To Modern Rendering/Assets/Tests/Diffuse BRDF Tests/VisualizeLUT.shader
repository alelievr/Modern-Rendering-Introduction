Shader "Unlit/Visualize3DTextureUInt"
{
    Properties
    {
        _Slice("Depth Slice", Range(0, 1)) = 0.0
        _Tex3D("3D Texture", 3D) = "" {}
        [Enum(Point, 0, Bilinear, 1, Trilinear, 2)] _FilterMode("Filter Mode", Float) = 0
        _SamplePerPixels("Sample Per Pixel", Float) = 4 // Set this to the total number of samples used for normalization
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
            };

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.uv = v.uv;
                return o;
            }

            // Texture and uniforms
            Texture3D<uint> _Tex3D;
            SamplerState sampler_Tex3D;
            float _Slice; // 0..1, normalized depth
            float _TotalSampleCount;
            float _FilterMode;
            float _SamplePerPixels;

            float LoadNormalizedHitCount(uint3 coord)
            {
                uint3 size;
                _Tex3D.GetDimensions(size.x, size.y, size.z);

                coord = clamp(coord, uint3(0, 0, 0), size - 1);
                uint hitCount = _Tex3D.Load(uint4(coord, 0)).r;

                // Assuming _TotalSampleCount is set to the total number of samples
                // This should be set before rendering, e.g., in a compute shader
                uint totalSampleCount = _TotalSampleCount;

                if (totalSampleCount == 0)
                    return 0.0f;

                // Normalize hit count to [0, 1] range
                return hitCount / (float)totalSampleCount;
            }

            fixed4 frag(v2f i) : SV_Target
            {
                uint3 size;
                _Tex3D.GetDimensions(size.x, size.y, size.z);

                // Compute 3D texel coordinate
                uint3 coord;
                coord.xy = uint2(i.uv * size.xy);
                coord.z = uint(_Slice * (size.z - 1) + 0.5);
                
                _TotalSampleCount = 1024 * 1024; // TODO: send value from C#

                float throughput = 0.0f;
                switch ((uint)_FilterMode)
                {
                    case 0: // Point
                        throughput = LoadNormalizedHitCount(coord);
                        break;
                    case 1: // Bilinear
                        // Manual bilinear filtering
                        float t0 = LoadNormalizedHitCount(coord);
                        float t1 = LoadNormalizedHitCount(coord + uint3(1, 0, 0));
                        float t2 = LoadNormalizedHitCount(coord + uint3(0, 1, 0));
                        float t3 = LoadNormalizedHitCount(coord + uint3(1, 1, 0));
                        // uv fraction to weight the bilinear interpolation
                        float2 uvFraction = i.uv * size.xy - float2(coord.xy);
                        throughput = (t0 * (1.0f - uvFraction.x) * (1.0f - uvFraction.y) +
                                      t1 * uvFraction.x * (1.0f - uvFraction.y) +
                                      t2 * (1.0f - uvFraction.x) * uvFraction.y +
                                      t3 * uvFraction.x * uvFraction.y);
                        break;
                    case 2: // Trilinear
                        float y0 = LoadNormalizedHitCount(coord);
                        float y1 = LoadNormalizedHitCount(coord + uint3(1, 0, 0));
                        float y2 = LoadNormalizedHitCount(coord + uint3(0, 1, 0));
                        float y3 = LoadNormalizedHitCount(coord + uint3(1, 1, 0));
                        float y4 = LoadNormalizedHitCount(coord + uint3(0, 0, 1));
                        float y5 = LoadNormalizedHitCount(coord + uint3(1, 0, 1));
                        float y6 = LoadNormalizedHitCount(coord + uint3(0, 1, 1));
                        float y7 = LoadNormalizedHitCount(coord + uint3(1, 1, 1));
                        // 3D uv + slice fraction to weight the trilinear interpolation
                        float3 uvFraction2 = float3(i.uv * size.xy, _Slice * size.z) - float3(coord) + 0.5;
                        throughput = (y0 * (1.0f - uvFraction2.x) * (1.0f - uvFraction2.y) * (1.0f - uvFraction2.z) +
                                      y1 * uvFraction2.x * (1.0f - uvFraction2.y) * (1.0f - uvFraction2.z) +
                                      y2 * (1.0f - uvFraction2.x) * uvFraction2.y * (1.0f - uvFraction2.z) +
                                      y3 * uvFraction2.x * uvFraction2.y * (1.0f - uvFraction2.z) +
                                      y4 * (1.0f - uvFraction2.x) * (1.0f - uvFraction2.y) * uvFraction2.z +
                                      y5 * uvFraction2.x * (1.0f - uvFraction2.y) * uvFraction2.z +
                                      y6 * (1.0f - uvFraction2.x) * uvFraction2.y * uvFraction2.z +
                                      y7 * uvFraction2.x * uvFraction2.y * uvFraction2.z);
                        break;
                }

                throughput /= _SamplePerPixels;

                return fixed4(throughput, throughput, throughput, 1.0);
            }
            ENDHLSL
        }
    }
}
