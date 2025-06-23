Shader "Unlit/DebugLines"
{
    Properties
    {
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
            // make fog work
            #pragma multi_compile_fog

            #pragma enable_d3d11_debug_symbols

            #include "UnityCG.cginc"

            #define READ_DEBUG
            #include "DebugLines.hlsl"

            struct appdata
            {
            };

            struct v2f
            {
                float3 color : COLOR;
                float4 vertex : SV_POSITION;
            };

            v2f vert(appdata v, uint instanceID : SV_InstanceID, uint vertexID : SV_VertexID)
            {
                v2f o;

                DebugLine dLine = GetDebugLine(instanceID);

                float3 vertexPosition;
                if (vertexID == 0)
                    vertexPosition = dLine.start;
                else
                    vertexPosition = dLine.end;

                o.vertex = UnityObjectToClipPos(vertexPosition);
                o.color = dLine.color;
                return o;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                return float4(i.color, 1.0f);
            }
            ENDCG
        }
    }
}
