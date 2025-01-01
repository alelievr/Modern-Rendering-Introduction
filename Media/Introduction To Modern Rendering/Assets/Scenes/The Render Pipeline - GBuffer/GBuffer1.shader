Shader "Unlit/GBuffer1"
{
	Properties
	{
        _Albedo ("Albedo", Color) = (1,1,1,1)
	}

	SubShader
	{
		Tags { 
			"RenderType"="Opaque"
		}

		Pass
		{
			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag

			#include "UnityCG.cginc"

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
                float3 tangent : TEXCOORD2;
                UNITY_VERTEX_OUTPUT_STEREO
            };
            
            float4 _Albedo;

            v2f vert (appdata v)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.normal = v.normal;
                o.uv = v.uv;
                o.tangent = v.tangent;
                return o;
            }

            float4 frag(v2f i) : SV_Target
            {
                return float4(i.normal, 1);
            }
			ENDCG
		}
	}
}