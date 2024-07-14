using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;
using Unity.Mathematics;

public class Mesh07TangentBitangent : MonoBehaviour
{
    public Transform controlPoint;
    public MeshFilter sphere;
    public float scale = 0.05f;
    public float length = 0.1f;

    Arrow normal;
    Arrow tangent;
    Arrow bitangent;

    Vector2 OnMinusY(Vector2 v)
    {
        return new Vector2(v.x, 1 - v.y);
    }

    void Start()
    {
        var m = sphere.sharedMesh;
        var positions = m.vertices;
        int vertexIndex = 0;
        Vector3 pos = controlPoint.position;
        foreach (var p in positions)
        {
            var worldPos = sphere.transform.TransformPoint(p);

            controlPoint.position = worldPos;
            var normalVector = m.normals[vertexIndex];
            var tangentVector = m.tangents[vertexIndex];
            // Unity stores the direction of the bitangent in the w component of the tangent
            var bitangentVector = Vector3.Cross(normalVector, tangentVector);

            pos = p;
            normal = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>(); 
            tangent = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>(); 
            bitangent = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>(); 

            normal.Initialize(pos, pos + normalVector * length, Utils.GetYAxisColor());
            tangent.Initialize(pos, pos + new Vector3(tangentVector.x, tangentVector.y, tangentVector.z) * length, Utils.GetXAxisColor());
            bitangent.Initialize(pos, pos + bitangentVector * length, Utils.GetZAxisColor());
            normal.scale = tangent.scale = bitangent.scale = scale;

            var dot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
            dot.transform.position = pos;
            dot.size = scale * 0.3f;
            vertexIndex++;
        }

        // var indices = m.GetIndices(0);
        // for (int i = 0; i < indices.Length; i += 3)
        // {
        //     var v0 = positions[indices[i]];
        //     var v1 = positions[indices[i + 1]];
        //     var v2 = positions[indices[i + 2]];

        //     // Unity uses the OpenGL conventions for UV for it's primitives
        //     var uv0 = OnMinusY(m.uv[indices[i]]);
        //     var uv1 = OnMinusY(m.uv[indices[i + 1]]);
        //     var uv2 = OnMinusY(m.uv[indices[i + 2]]);

        //     var normal0 = m.normals[indices[i]];
        //     var normal1 = m.normals[indices[i + 1]];
        //     var normal2 = m.normals[indices[i + 2]];

        //     Vector3 ab = v1 - v0;
        //     Vector3 ac = v2 - v0;

        //     float2 uv_ab = uv1 - uv0;
        //     float2 uv_ac = uv2 - uv0;

        //     float det = uv_ab.x * uv_ac.y - uv_ab.y * uv_ac.x;

        //     Vector3 tangentVector = (uv_ac.y * ab - uv_ab.y * ac);
        //     Vector3 bitangentVector = (uv_ab.x * ac - uv_ac.x * ab);

        //     tangentVector -= normal0 * Vector3.Dot(normal0, tangentVector);
        //     tangentVector.Normalize();
            
        //     bitangentVector = Vector3.Cross(normal0, tangentVector);

        //     Vector3 pos = v0;
        //     normal = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>(); 
        //     tangent = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>(); 
        //     bitangent = Instantiate(Resources.Load("Arrow"), transform).GetComponent<Arrow>(); 

        //     normal.Initialize(pos, pos + normal0 * length, Utils.GetYAxisColor());
        //     tangent.Initialize(pos, pos + tangentVector * length, Utils.GetXAxisColor());
        //     bitangent.Initialize(pos, pos + bitangentVector * length, Utils.GetZAxisColor());
        //     normal.scale = tangent.scale = bitangent.scale = scale;

        //     var dot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
        //     dot.transform.position = pos;
        //     dot.size = scale * 0.3f;
        // }
    }
}
