Shader "Unlit/MeshData"
{
	Properties
	{
		[Enum(Position, 0, Normal, 1, UV, 2, Tangent, 3, BiTangent, 4)]_Mode ("Mode", Float) = 0
        _Pivot ("Pivot", Vector) = (0, 0, 0, 0) 
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

            float _Mode = 25.0;
            float4 _Pivot;

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
                float3 normal : NORMAL;
                float4 uv : TEXCOORD0;
                float3 position : TEXCOORD1;
                float3 tangent : TEXCOORD2;
                UNITY_VERTEX_OUTPUT_STEREO
            };

            v2f vert (appdata v)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.normal = v.normal;
                o.uv = v.uv;
                o.position = v.vertex - _Pivot.xyz;
                o.tangent = v.tangent;
                return o;
            }

            float4 frag(v2f i) : SV_Target
            {
                switch (_Mode)
                {
                    default:
                    case 0:
                        return float4(i.position, 1);
                    case 1:
                        return float4(i.normal * 0.5 + 0.5, 1);
                    case 2:
                        return i.uv;
                    case 3:
                        return float4(i.tangent * 0.5 + 0.5, 1);
                    case 4:
                        return float4(normalize(cross(i.tangent, i.normal)), 1);
                }
            }
			ENDCG
		}
	}
}