Shader "Unlit/Visualize3DTextureUInt"
{
    Properties
    {
        _Slice ("Depth Slice", Range(0, 1)) = 0.0
        _Tex3D ("3D Texture", 3D) = "" {}
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

            uint GetMaxDimension(Texture3D<uint> tex)
            {
                uint3 size;
                tex.GetDimensions(size.x, size.y, size.z);
                return size.z;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                uint3 size;
                _Tex3D.GetDimensions(size.x, size.y, size.z);

                // Compute 3D texel coordinate
                uint3 coord;
                coord.xy = uint2(i.uv * size.xy);
                coord.z = uint(_Slice * (size.z - 1) + 0.5);

                uint hitCount = _Tex3D.Load(uint4(coord, 0)).r;

                _TotalSampleCount = 1024 * 1024 * 4; // probably?

                // Normalize for visualization (optional: assumes 0..255)
                float norm = hitCount / (float)_TotalSampleCount;
                return fixed4(norm, norm, norm, 1.0);
            }
            ENDHLSL
        }
    }
}
