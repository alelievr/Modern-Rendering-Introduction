Shader "Unlit/PrimitiveID"
{
	Properties
	{
        _Color ("Albedo", Color) = (1,1,1,1)
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
            #pragma target 5.0

			#include "UnityCG.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };
            
            struct v2f
            {
                float4 vertex : SV_POSITION;
                
                UNITY_VERTEX_OUTPUT_STEREO
            };
            
            float4 _Color;

            v2f vert (appdata v)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
                o.vertex = UnityObjectToClipPos(v.vertex);
                return o;
            }

            // https://www.shadertoy.com/view/llGSzw
            float3 hash13( uint n ) 
            {
                // integer hash copied from Hugo Elias
                n = (n << 13U) ^ n;
                n = n * (n * n * 15731U + 789221U) + 1376312589U;
                uint3 k = n * uint3(n,n*16807U,n*48271U);
                return float3( k & 0x7fffffffU) / (float)0x7fffffff;
            }

            float4 frag(v2f i, uint primitiveID : SV_PrimitiveID) : SV_Target
            {
                return float4(hash13(primitiveID), 1);
            }
			ENDCG
		}
	}
}