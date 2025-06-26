Shader "Unlit/VisualizeHeightmap"
{
    Properties
    {
        _HeightMap ("Texture", 2D) = "white" {}
        [Enum(HeightMap, 0, POM, 1)] _ViewMode ("View Mode", Float) = 0
        _POMNumSteps ("POM Steps", Range(1, 4096)) = 10
        _ColorRemap ("Remap", Vector) = (-5, 5, 0, 0)
    }

    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Cull Front
        ZTest LEqual
        ZWrite On

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
                float3 viewDirWS : TEXCOORD1;
                float3 positionWS : TEXCOORD2;
                float4 vertex : SV_POSITION;
            };

            struct f2o
            {
                float4 color : COLOR;
                float depth : SV_Depth;
            };

            sampler2D _HeightMap;
            float4 _HeightMap_TexelSize;
            float _ViewMode;
            float _POMNumSteps;
            float4 _ColorRemap;

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);

                float3 positionWS = mul(unity_ObjectToWorld, v.vertex).xyz;
                float3 viewDirWS = positionWS - _WorldSpaceCameraPos;
                o.viewDirWS = mul((float3x3)unity_WorldToObject, viewDirWS);
                o.viewDirWS = viewDirWS;
                o.uv = positionWS.xz + 0.5;
                o.positionWS = positionWS;

                return o;
            }

            float RemapHeight(float h)
            {
                return h / (_ColorRemap.y - _ColorRemap.x) + 0.5;
            }

            float3 HeightColor(float h)
            {
                return lerp(lerp(float3(0,0,1), float3(0,1,1), h), lerp(float3(1,1,0), float3(1,0,0), h), h);
            }

            float SampleHeight(float2 uv)
            {
                float h = tex2D(_HeightMap, uv).r;
                return h;
            }

            bool ParallaxOcclusionMapping(float2 uv, float3 viewDirWS, inout float3 positionWS)
            {
                // In the heightfield, each pixel represents 1 unit, so to scale the model to world space, we divide by the texture size.
                float localToWorldScale = _HeightMap_TexelSize.x;
                float t1 = (5.0 * localToWorldScale - positionWS.y) / viewDirWS.y;
                float t2 = (-5.0 * localToWorldScale - positionWS.y) / viewDirWS.y;

                float totalVolumeDistance = abs(t1 - t2);
                float stepSize = totalVolumeDistance / _POMNumSteps;

                [loop]
                for (int i = 0; i < (int)_POMNumSteps; i++)
                {
                    float sampled = SampleHeight(positionWS.xz + 0.5) * localToWorldScale;
                    if (positionWS.y < sampled && positionWS.y > sampled - 2.0 * localToWorldScale) // 2 unit of thickness
                        return true;

                    positionWS += viewDirWS * stepSize;
                }

                return false;
            }

            float3 IntersectAABBProxy(float3 positionWS, float3 viewDirWS)
            {
                float localToWorldScale = _HeightMap_TexelSize.x;

                // Intersect AABB representing the cube to get a starting point before the ray marching
                float planePosition = 5 * localToWorldScale; // the values in the height field are always less than 5.
                float3 rayOrigin = positionWS - viewDirWS * 100;
                float3 rayDir = viewDirWS;
                float3 boxMin = float3(-0.5, -planePosition, -0.5);
                float3 boxMax = float3( 0.5,  planePosition,  0.5);

                // Ray-AABB intersection using slab method
                float3 invDir = rcp(rayDir);
                float3 t0 = (boxMin - rayOrigin) * invDir;
                float3 t1 = (boxMax - rayOrigin) * invDir;

                float3 tMin = min(t0, t1);
                float3 tMax = max(t0, t1);

                float tEnter = max(max(tMin.x, tMin.y), tMin.z);
                float tExit  = min(min(tMax.x, tMax.y), tMax.z);

                // Valid intersection check
                if (tExit >= max(tEnter, 0.0))
                {
                    positionWS = rayOrigin + tEnter * rayDir;
                }

                return positionWS;
            }

            f2o frag (v2f i)
            {
                f2o o;

                i.viewDirWS = normalize(i.viewDirWS);

                i.positionWS = IntersectAABBProxy(i.positionWS, i.viewDirWS);

                if (_ViewMode >= 1.0)
                {
                    bool hit = ParallaxOcclusionMapping(i.uv, i.viewDirWS, i.positionWS);
                    if (!hit)
                        clip(-1);
                }

                float2 uv = i.positionWS.xz + 0.5;

                if (any(uv < -0.0001) || any(uv > 1.0001))
                    clip(-1);

                o.color = RemapHeight(tex2D(_HeightMap, uv));

                // border color
                if (any(uv < 0.0001) || any(uv > 0.9999))
                    o.color = float4(0, 0, 0, 1);

                float4 positionCS = mul(UNITY_MATRIX_VP, float4(i.positionWS, 1.0));
                o.depth = positionCS.z / positionCS.w; // NDC depth

                return o;
            }
            ENDCG
        }
    }
}
