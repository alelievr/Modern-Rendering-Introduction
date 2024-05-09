Shader "Unlit/MeshUVUnwrap"
{
	Properties
	{
		[Enum(Position, 0, Normal, 1, UV, 2, Tangent, 3, BiTangent, 4)]_Mode ("Mode", Float) = 0
        _Progress("Progress", Range(0, 1)) = 0
        _Pivot ("Pivot", Vector) = (0.5, 0.5, 0.5, 0.5)
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
            float _Progress;
            float4 _Pivot;

            struct appdata
            {
                float4 vertex : POSITION;
                float4 uv : TEXCOORD0;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };
            
            struct v2f
            {
                float4 vertex : SV_POSITION;
                float4 uv : TEXCOORD0;
                UNITY_VERTEX_OUTPUT_STEREO
            };

            v2f vert (appdata v)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
                float3 uvPosition = float3(v.uv.x, _Pivot.y, v.uv.y) * 2 - 1; // remap UV between -1 and 1 in object space to do better visualization
                o.vertex = UnityObjectToClipPos(lerp(v.vertex, uvPosition, _Progress));
                o.uv = v.uv;
                return o;
            }

            float4 frag(v2f i) : SV_Target
            {
                return i.uv;
            }
			ENDCG
		}
	}
}