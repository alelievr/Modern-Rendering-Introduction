Shader "Hidden/LineRenderer/DottedLine"
{
	Properties
	{
		_MainTex ("Texture", 2D) = "white" {}
		_LineSpacing ("Space", Range (0.0,1.0)) = 0.25
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

		#include "UnityCG.cginc"

		struct appdata
		{
			float4 vertex : POSITION;
			float2 uv : TEXCOORD0;
			float4 color : COLOR;
		};

		struct v2f
		{
			float2 uv : TEXCOORD0;
			float4 vertex : SV_POSITION;
            // float4 worldPos : TEXCOORD1;
			float4 color : COLOR;
		};

		// sampler2D _MainTex;
		// float4 _MainTex_ST;
		float _LineSpacing;
        float _LineLength;


		v2f vert (appdata v)
		{
			v2f o;
			o.vertex = UnityObjectToClipPos(v.vertex);
			o.uv = v.uv * _LineLength;
			o.color = v.color;
            // o.worldPos = mul(unity_ObjectToWorld, v.vertex);
			return o;
		}

		fixed4 frag(v2f i) : SV_Target
		{
            // TODO: dotted lines
            if ((i.uv.x % (_LineSpacing * 2)) > _LineSpacing)
            {
                discard;
            }
			return i.color;
		}
		ENDCG
		}
	}
} 