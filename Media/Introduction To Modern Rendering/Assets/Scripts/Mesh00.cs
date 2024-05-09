using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;

public class Mesh00 : MonoBehaviour
{
    public MeshFilter mf;
    public float length = 0.05f;
    public float scale = 0.04f;
    public float width = 0.1f;

    void Start()
    {
        for (int vertexIndex = 0; vertexIndex < mf.mesh.vertexCount; vertexIndex++)
        {
            Vector3 vertex = mf.mesh.vertices[vertexIndex];
            Vector3 normal = mf.mesh.normals[vertexIndex];
            vertex = mf.transform.TransformPoint(vertex);

            var arrow = Instantiate(Resources.Load("Arrow")).GetComponent<Arrow>();
            arrow.transform.SetParent(transform);
            arrow.color = NormalToColor(normal);
            arrow.width = width;
            arrow.scale = scale;
            arrow.Initialize(vertex, vertex + normal * length);
        }
    }

    Color NormalToColor(Vector3 normal)
    {
        normal = normal / 2 + new Vector3(0.5f, 0.5f, 0.5f);
        return new Color(normal.x, normal.y, normal.z);
    }
}
