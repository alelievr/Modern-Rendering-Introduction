using System.Collections.Generic;
using UnityEngine;

public class GgxVndfVisualizer : MonoBehaviour
{
    [Range(0f, 1f)] public float roughness = 0.5f;
    public int linesPerFrame = 10;
    public Vector3 wo = Vector3.up;

    public bool reflect = true;

    public Color woColor = Color.red;
    public Color vColor = Color.cyan;

    float previousRoughness;
    Vector3 previousWo;
    readonly List<GameObject> lines = new();

    void Start()
    {
        previousRoughness = roughness;
        CreateLine(transform.position, transform.position + wo.normalized * 10, woColor);
    }

    void Update()
    {
        bool change = false;

        // Detect roughness change
        if (!Mathf.Approximately(roughness, previousRoughness))
            change = true;

        // Detect wo change
        if (wo != previousWo)
            change = true;
        
        if (change)
        {
            previousRoughness = roughness;
            previousWo = wo;
            ClearLines();
            CreateLine(transform.position, transform.position + wo.normalized * 10, woColor);
        }

        // Add lines every frame
        for (int i = 0; i < linesPerFrame; i++)
        {
            float u1 = Random.value;
            float u2 = Random.value;

            Vector3 dir = GgxVndf(wo.normalized, roughness, u1, u2);
            dir = reflect ? Vector3.Reflect(dir, wo.normalized) : dir;
            CreateLine(transform.position, transform.position + dir, vColor);
        }
    }

    void CreateLine(Vector3 start, Vector3 end, Color color)
    {
        var lineObj = Instantiate(Resources.Load("Line"), transform) as GameObject;
        Line line = lineObj.GetComponent<Line>();
        line.dotted = false;
        line.color = color;
        line.width = 0.03f;
        line.UpdateLine(start, end);
        lines.Add(lineObj);
    }

    void ClearLines()
    {
        foreach (var lineObj in lines)
        {
            Destroy(lineObj);
        }
        lines.Clear();
    }

    Vector3 GgxVndf(Vector3 wo, float roughness, float u1, float u2)
    {
        Vector3 v = new Vector3(wo.x * roughness, wo.y, wo.z * roughness).normalized;

        Vector3 t1 = (v.y < 0.999f) ? Vector3.Normalize(Vector3.Cross(v, Vector3.up)) : Vector3.right;
        Vector3 t2 = Vector3.Cross(t1, v);

        float a = 1.0f / (1.0f + v.y);
        float r = Mathf.Sqrt(u1);
        float phi = (u2 < a) ? (u2 / a) * Mathf.PI : Mathf.PI + (u2 - a) / (1.0f - a) * Mathf.PI;
        float p1 = r * Mathf.Cos(phi);
        float p2 = r * Mathf.Sin(phi) * ((u2 < a) ? 1.0f : v.y);

        Vector3 n = p1 * t1 + p2 * t2 + Mathf.Sqrt(Mathf.Max(0.0f, 1.0f - p1 * p1 - p2 * p2)) * v;

        Vector3 result = new Vector3(roughness * n.x, Mathf.Max(0.0f, n.y), roughness * n.z);
        return result.normalized;
    }
}
