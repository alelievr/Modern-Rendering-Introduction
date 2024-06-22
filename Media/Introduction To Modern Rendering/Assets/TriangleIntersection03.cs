using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteAlways]
public class TriangleIntersection03 : MonoBehaviour
{
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
        // var ab = B.transform.position - A.transform.position;
        // var ac = C.transform.position - A.transform.position;

        // var cross = Vector3.Cross(ac, ab);
        // crossArrow.start = A.transform.position;
        // crossArrow.end = crossArrow.start + cross;
    }
}
