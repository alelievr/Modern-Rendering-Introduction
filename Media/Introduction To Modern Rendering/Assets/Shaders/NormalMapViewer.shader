Shader "Unlit/NormalMapViewer"
{
	Properties
	{
        [NoScaleOffset] _NormalMap("Normals", 2D) = "bump" {}
        [Toggle]_NormalMapToggle("Normal Map Toggle", Float) = 1
	}

	SubShader
	{
		Tags { 
			"RenderType"="Opaque"
		}

		Pass
		{
			// Wireframe shader based on the the following
			// http://developer.download.nvidia.com/SDK/10/direct3d/Source/SolidWireframe/Doc/SolidWireframe.pdf

			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag

			#include "UnityCG.cginc"

            sampler2D _NormalMap;
            float _NormalMapToggle;

            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float4 uv : TEXCOORD0;
                float4 tangent : TANGENT;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };
            
            struct v2f
            {
                float4 vertex : SV_POSITION;
                float4 uv : TEXCOORD0;
                float3 normalWS : TEXCOORD1;
                float4 tangentWS : TEXCOORD2;
                UNITY_VERTEX_OUTPUT_STEREO
            };

            v2f vert (appdata v)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.normalWS = UnityObjectToWorldNormal(v.normal.xyz);
                o.tangentWS = float4(UnityObjectToWorldDir(v.tangent.xyz), v.tangent.w);
                o.uv = v.uv;
                return o;
            }

            float GetOddNegativeScale()
            {
                return unity_WorldTransformParams.w >= 0.0 ? 1.0 : -1.0;
            }

            float3x3 CreateTangentToWorld(float3 normal, float3 tangent, float flipSign)
            {
                // For odd-negative scale transforms we need to flip the sign
                float sgn = flipSign * GetOddNegativeScale();
                float3 bitangent = cross(normal, tangent) * sgn;

                return float3x3(tangent, bitangent, normal);
            }

            float4 frag(v2f i) : SV_Target
            {
                float4 map = tex2D(_NormalMap, i.uv);
                float3 normalMapTS = UnpackNormal(map);
                float3x3 tangentToWorld = CreateTangentToWorld(normalize(i.normalWS), normalize(i.tangentWS.xyz), i.tangentWS.w);
                float3 normalWS = normalize(mul(normalMapTS, tangentToWorld));
                if (_NormalMapToggle > 0.9999)
                    return float4(normalWS * 0.5 + 0.5, 1);
                else
                    return float4(i.normalWS * 0.5 + 0.5, 1);
            }
			ENDCG
		}
	}
}