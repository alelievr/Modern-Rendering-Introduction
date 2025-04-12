using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GGXVNDFDirections : MonoBehaviour
{
    public Transform basisTransform;
    public float roughness = 0.5f;
    public Vector3 outgoingDirectionWS_Red = Vector3.forward;

    void Update()
    {
        if (basisTransform == null) return;

        // Tangent frame setup
        Vector3 N = basisTransform.up.normalized;
        Vector3 T = basisTransform.right.normalized;
        Vector3 B = basisTransform.forward.normalized;

        Matrix4x4 tangentToWorld = new Matrix4x4(T, N, B, new Vector4(0, 0, 0, 1));

        // tangentToWorld = Matrix4x4.identity;

        Matrix4x4 worldToTangent = tangentToWorld.transpose;

        // Outgoing direction to tangent space
        Vector3 outgoingTS = worldToTangent.MultiplyVector(outgoingDirectionWS_Red.normalized);

        float u1 = Random.value;
        float u2 = Random.value;

        // Sample microfacet normal
        Vector3 microfacetNormalTS = GetRandomDirectionGGX_VNDF(outgoingTS, roughness, u1, u2);

        // Reflect
        Vector3 incomingTS = Vector3.Reflect(-outgoingTS, microfacetNormalTS);
        Vector3 incomingWS = tangentToWorld.MultiplyVector(incomingTS).normalized;

        Vector3 origin = basisTransform.position;

        // Intermediate tangent space vectors
        Debug.DrawLine(origin, origin + outgoingTS.normalized + Vector3.one * 0.01f, Color.cyan);
        Debug.DrawLine(origin, origin + microfacetNormalTS.normalized, Color.yellow);
        Debug.DrawLine(origin, origin + incomingTS.normalized + Vector3.one * 0.01f, Color.magenta);

        // Outgoing vector (towards the camera)
        Debug.DrawLine(origin, origin + outgoingDirectionWS_Red.normalized, Color.red);
        
        // Incoming vector (coming from the light)
        Debug.DrawLine(origin, origin + incomingWS, Color.green);
    }

    Vector3 GetRandomDirectionGGX_VNDF(Vector3 wo, float roughness, float u1, float u2)
    {
        Vector3 v = new Vector3(wo.x * roughness, wo.y, wo.z * roughness).normalized;

        Vector3 t1 = (v.y < 0.999f) ? Vector3.Normalize(Vector3.Cross(v, Vector3.up)) : Vector3.forward;
        Vector3 t2 = Vector3.Cross(t1, v);

        float a = 1.0f / (1.0f + v.y);
        float r = Mathf.Sqrt(u1);
        float phi = (u2 < a) ? (u2 / a) * Mathf.PI : Mathf.PI + (u2 - a) / (1.0f - a) * Mathf.PI;
        float p1 = r * Mathf.Cos(phi);
        float p2 = r * Mathf.Sin(phi) * ((u2 < a) ? 1.0f : v.y);

        Vector3 n = p1 * t1 + p2 * t2 + Mathf.Sqrt(Mathf.Max(0.0f, 1.0f - p1 * p1 - p2 * p2)) * v;
        return new Vector3(roughness * n.x, Mathf.Max(0.0f, n.y), roughness * n.z).normalized;
    }
}
