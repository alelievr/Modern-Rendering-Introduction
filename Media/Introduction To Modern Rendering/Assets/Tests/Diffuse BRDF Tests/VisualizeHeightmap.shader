Shader "Unlit/VisualizeHeightmap"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
        _HeightScale ("Height Scale", Range(0, 0.1)) = 0.05
        [Enum(HeightMap, 0, POM, 1)] _ViewMode ("View Mode", Float) = 0
        _POMNumSteps ("POM Steps", Range(1, 1024)) = 10
        _Remap ("Remap", Vector) = (-5, 5, 0, 0)
    }

    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma multi_compile_fog

            #include "UnityCG.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float4 tangent : TANGENT;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float3 viewDirTS : TEXCOORD1;
                UNITY_FOG_COORDS(2)
                float4 vertex : SV_POSITION;
            };

            sampler2D _MainTex;
            float4 _MainTex_ST;
            float _HeightScale;
            float _ViewMode;
            float _POMNumSteps;
            float4 _Remap;

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.uv = TRANSFORM_TEX(v.uv, _MainTex);

                float3 viewDirWS = _WorldSpaceCameraPos - mul(unity_ObjectToWorld, v.vertex).xyz;
                float3 t = normalize(mul((float3x3)unity_ObjectToWorld, v.tangent.xyz));
                float3 n = normalize(mul((float3x3)unity_ObjectToWorld, v.normal));
                float3 b = cross(n, t) * v.tangent.w;
                float3x3 TBN = float3x3(t, b, n);
                o.viewDirTS = mul(TBN, viewDirWS);

                UNITY_TRANSFER_FOG(o, o.vertex);
                return o;
            }

            float RemapHeight(float h)
            {
                return h / (_Remap.y - _Remap.x) + 0.5;
            }

            float3 HeightColor(float h)
            {
                return lerp(lerp(float3(0,0,1), float3(0,1,1), h), lerp(float3(1,1,0), float3(1,0,0), h), h);
            }

            float2 ParallaxOcclusionMapping(float2 uv, float3 viewDirTS)
            {
                float2 dir = normalize(viewDirTS.xy) * _HeightScale / viewDirTS.z;
                float2 delta = -dir / _POMNumSteps;
                float2 curUV = uv;
                float curHeight = 0.0;

                [loop]
                for (int i = 0; i < (int)_POMNumSteps; i++)
                {
                    float sampled = RemapHeight(tex2D(_MainTex, curUV).r);
                    if (sampled < curHeight)
                        break;
                    curUV += delta;
                    curHeight += 1.0 / _POMNumSteps;
                }

                return curUV;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                float2 uv = i.uv;
                if (_ViewMode >= 1.0)
                    uv = ParallaxOcclusionMapping(i.uv, normalize(i.viewDirTS));

                fixed4 col = 1 - RemapHeight(tex2D(_MainTex, uv));
                UNITY_APPLY_FOG(i.fogCoord, col);
                return col;
            }
            ENDCG
        }
    }
}
