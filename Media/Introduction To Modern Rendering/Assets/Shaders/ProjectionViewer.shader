Shader "Custom/ProjectionViewer"
{
    Properties
    {
        _Color ("Color", Color) = (1,1,1,1)
        _MainTex ("Albedo (RGB)", 2D) = "white" {}
        _Glossiness ("Smoothness", Range(0,1)) = 0.5
        _Metallic ("Metallic", Range(0,1)) = 0.0
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        CGPROGRAM
        #pragma surface surf Standard fullforwardshadows vertex:vert addshadow

        sampler2D _MainTex;
        
        sampler2D _NormalMap;
 
        struct Input
        {
            float2 uv_MainTex;
            float2 uv_NormalMap;
        };
 
        half _Glossiness;
        half _Metallic;
        fixed4 _Color;
        float _ProjectionViewSlider;

        float4x4 _InverseViewMatrix;
        float4x4 _ViewProjectionMatrix;
        float _NearPlane;
        float _NearPlaneSize;
 
        void vert(inout appdata_full v)
        {
            // float4 projectedPosition = mul(_ViewProjectionMatrix, v.vertex);
            // v.vertex.xyz = lerp(v.vertex, projectedPosition, _ProjectionViewSlider).xyz;
            // Transform vertex position to world space
            float4 worldPos = mul(unity_ObjectToWorld, v.vertex);

            // Transform world position to clip space using the custom view proj matrix
            float4 clipPos = mul(_ViewProjectionMatrix, worldPos);

            // Perspective divide
            clipPos.xyz /= clipPos.w;

            // positions in HCLIP are between -1 and 1, we need to adjust that so it matches the near and far planes of the camera for the visualization
            clipPos.w = 1;
            clipPos.z = -_NearPlane - 0.000001; // offset to avoid z clip
            clipPos.xy *= abs(_NearPlaneSize / 2);

            // Now that the objects are transformed in HCLIP space of the camera, we transform them back to world space
            clipPos = mul(_InverseViewMatrix, clipPos);

            // Lerp between the original clip position and the projected clip position
            clipPos = lerp(worldPos, clipPos, _ProjectionViewSlider);
            // Transform clip space back to object space

            v.vertex = mul(unity_WorldToObject, clipPos);
        }
 
        void surf (Input In, inout SurfaceOutputStandard o)
        {
            fixed4 c = tex2D (_MainTex, In.uv_MainTex) * _Color;
            o.Albedo = c.rgb;
            o.Normal = UnpackNormal (tex2D (_NormalMap, In.uv_NormalMap));
            o.Metallic = _Metallic;
            o.Smoothness = _Glossiness;
            o.Alpha = c.a;   
        }
 
        ENDCG
    }
    FallBack "Diffuse"
}
