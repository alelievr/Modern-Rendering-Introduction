Shader "Unlit/TransparentGrid"
{
    Properties
    {
        _GridColor ("Grid Color", Color) = (1,1,1,0.3)
        _BackgroundColor ("Background Color", Color) = (0,0,0,0.1)
        _GridSpacing ("Grid Spacing", Float) = 10.0
        _LineWidth ("Line Width", Float) = 0.05
        _Right ("Right Axis (world)", Vector) = (1,0,0,0)
        _Up ("Up Axis (world)", Vector) = (0,0,1,0)
    }
    SubShader
    {
        Tags { "Queue"="Transparent" "RenderType"="Transparent" }
        LOD 100

        Blend SrcAlpha OneMinusSrcAlpha
        ZWrite Off
        Cull Off

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #include "UnityCG.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float2 uv : TEXCOORD0;
            };

            float4 _GridColor;
            float4 _BackgroundColor;
            float _GridSpacing;
            float _LineWidth;
            float4 _Right; // xyz direction
            float4 _Up;    // xyz direction

            v2f vert (appdata data)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(data.vertex);

                float3 R = (_Right.xyz);
                float3 U = (_Up.xyz);
                // R = float3(1, 0, 0);
                // U = float3(0, 1, 0);

                // build UV: project worldPos onto R and U,
                // and divide by spacing here to keep frag simple
                o.uv.x = dot(data.vertex.xyz, R) / _GridSpacing;
                o.uv.y = dot(data.vertex.xyz, U) / _GridSpacing;

                return o;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                float2 coord = abs(frac(i.uv / _GridSpacing) - 0.5);
                float l = step(coord.x, _LineWidth) + step(coord.y, _LineWidth);
                l = saturate(l);

                return lerp(_BackgroundColor, _GridColor, l);
            }
            ENDCG
        }
    }
}
