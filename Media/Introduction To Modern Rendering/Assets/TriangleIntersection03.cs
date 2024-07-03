using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class TriangleIntersection03 : MonoBehaviour
{
    [Header("Settings")]
    public Vector3 lineDir;

    [Header("Objects")]
    public Arrow crossArrow;
    public Triangle triangle;
    public Line line;
    public Arrow lineDirection;
    public Dot p;
    public Arrow pa;
    public Arrow pb;
    public Arrow pc;

    public Arrow crossPC;
    public Arrow crossPA;
    public Arrow crossPB;

    // TODO: class to show colored transparent Parallelepiped

    // public Dot A;
    // public Dot B;
    // public Dot C;

    void Start()
    {
        
    }

    void Update()
    {
        if (line != null)
            line.UpdateLine(p.transform.position - lineDir * 100, p.transform.position + lineDir * 100);
        // var ab = B.transform.position - A.transform.position;
        // var ac = C.transform.position - A.transform.position;

        // var cross = Vector3.Cross(ac, ab);
        // crossArrow.start = A.transform.position;
        // crossArrow.end = crossArrow.start + cross;

        var lineDirection = line.GetDirection();

        if (crossPA != null)
            crossPA.Initialize(p.transform.position, p.transform.position + Vector3.Cross(lineDirection, pa.GetVector()));
    }
}
