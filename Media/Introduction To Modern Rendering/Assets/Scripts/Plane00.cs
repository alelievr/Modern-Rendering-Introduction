using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Unity.VisualScripting;

public class Plane00 : MonoBehaviour
{
    public Dot dot;
    public Transform target;
    public Arrow arrow;
    public float time = 1;

    void Start()
    {
    }

    void Update()
    {
        Vector3 target = Vector3.Lerp(dot.transform.position, this.target.position, time);

        arrow.Initialize(dot.transform.position, target, Color.white);

        var y = Instantiate(Resources.Load("Line")).GetComponent<Line>();
    }
}
