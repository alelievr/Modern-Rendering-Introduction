Shader "Custom/Color CullMode"
{
    Properties
    {
        _FrontColor ("Front Color", Color) = (1,1,1,1)
        _BackColor ("Back Color", Color) = (1,1,1,1)
        _CullMode ("Cull Mode", Int) = 2
    }
    SubShader
    {
        Tags { "RenderType" = "Opaque" }
        Cull [_CullMode]

        CGPROGRAM
        
        float4 _FrontColor;
        float4 _BackColor;
        
        #pragma surface surf Lambert
        #pragma target 3.0
        struct Input
        {
            float face : VFACE;
        };
        
        void surf (Input IN, inout SurfaceOutput o)
        {
            float4 c = IN.face > 0 ? _FrontColor : _BackColor;
            if (c.a < 0.5)
                discard;
            o.Albedo = c.rgb;
        }
        ENDCG
    }
    Fallback "Diffuse"
}
