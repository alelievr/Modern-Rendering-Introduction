Shader "Unlit/DirectionalDiffuseCustomBRDF"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
        _Roughness ("Roughness", Range(0, 1)) = 0.5
        _DiffuseBRDFLUT ("Diffuse BRDF LUT", 3D) = "" {}
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
            #pragma multi_compile_fog

            #include "UnityCG.cginc"
            #include "Lighting.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float3 worldNormal : TEXCOORD1;
                float3 worldPos : TEXCOORD2;
                float4 vertex : SV_POSITION;
            };

            sampler2D _MainTex;
            float4 _MainTex_ST;
            float _Roughness;
            sampler3D _DiffuseBRDFLUT;

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.uv = TRANSFORM_TEX(v.uv, _MainTex);
                o.worldNormal = UnityObjectToWorldNormal(v.normal);
                o.worldPos = mul(unity_ObjectToWorld, v.vertex).xyz;
                return o;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                fixed3 normal = normalize(i.worldNormal);
                fixed3 lightDir = normalize(_WorldSpaceLightPos0.xyz);
                fixed3 viewDir = normalize(_WorldSpaceCameraPos - i.worldPos);

                // Custom BRDF: example - lambert * (N·L)^0.5
                float NdotL = saturate(dot(normal, lightDir));
                float LdotV = saturate(dot(lightDir, viewDir) * 0.5 + 0.5);
                float brdf = tex3D(_DiffuseBRDFLUT, float3(NdotL, LdotV, _Roughness)).r;

                fixed3 lightColor = _LightColor0.rgb;
                fixed3 texColor = tex2D(_MainTex, i.uv).rgb;

                fixed3 finalColor = texColor * lightColor * brdf * NdotL;

                fixed4 col = fixed4(finalColor, 1.0);
                return col;
            }
            ENDCG
        }
    }
}
