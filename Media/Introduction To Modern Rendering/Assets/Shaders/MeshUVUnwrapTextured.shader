Shader "Unlit/MeshUVUnwrapTextured"
{
    Properties
    {
        [Enum(Position, 0, Normal, 1, UV, 2, Tangent, 3, BiTangent, 4)]_Mode ("Mode", Float) = 0
        _Progress("Progress", Range(0, 1)) = 0
        _Pivot ("Pivot", Vector) = (0.5, 0.5, 0.5, 0.5)
        _Scale("Scale", Float) = 1
        _Albedo("Albedo", 2D) = "white" {}
        _NormalMap("Normal Map", 2D) = "bump" {}
    }

    SubShader
    {
        Tags { "RenderType"="Opaque" }

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc"
            #include "UnityLightingCommon.cginc"

            float _Mode;
            float _Progress;
            float4 _Pivot;
            float _Scale;
            sampler2D _Albedo;
            float4 _Albedo_ST;
            sampler2D _NormalMap;
            float4 _NormalMap_ST;

            struct appdata
            {
                float4 vertex : POSITION;
                float4 uv : TEXCOORD0;
                float3 normal : NORMAL;
                float4 tangent : TANGENT;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float2 uv : TEXCOORD0;
                float3 normal : TEXCOORD1;
                float3 tangent : TEXCOORD2;
                float3 bitangent : TEXCOORD3;
                float3 worldPos : TEXCOORD4;
                UNITY_VERTEX_OUTPUT_STEREO
            };

            v2f vert (appdata v)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(o);
                float3 uvPosition = float3(v.uv.x, _Pivot.y, v.uv.y) * 2 - 1;
                uvPosition *= _Scale;
                uvPosition = -uvPosition;
                o.vertex = UnityObjectToClipPos(lerp(v.vertex, uvPosition, _Progress));
                v.uv.xy = v.uv.xy * _Albedo_ST.xy + _Albedo_ST.zw;
                o.uv = v.uv.xy;
                o.normal = UnityObjectToWorldNormal(v.normal);
                o.normal = lerp(o.normal, float3(0, 1, 0), _Progress);
                o.tangent = UnityObjectToWorldDir(v.tangent.xyz);
                o.bitangent = cross(o.normal, o.tangent) * v.tangent.w;
                o.worldPos = mul(unity_ObjectToWorld, v.vertex).xyz;
                return o;
            }

            float4 frag(v2f i) : SV_Target
            {
                i.normal = normalize(i.normal);
                float3 albedo = tex2D(_Albedo, i.uv).rgb;
                float3 normalMap = UnpackNormal(tex2D(_NormalMap, i.uv));
                float3 normal = normalize(i.tangent * normalMap.x + i.bitangent * normalMap.y + i.normal * normalMap.z);
                float3 lightDir = normalize(_WorldSpaceLightPos0.xyz);
                float lambert = max(dot(normal, lightDir), 0.0);
                float3 diffuse = lambert * _LightColor0.rgb;
                return float4(albedo * diffuse, 1);
            }
            ENDCG
        }
    }
}