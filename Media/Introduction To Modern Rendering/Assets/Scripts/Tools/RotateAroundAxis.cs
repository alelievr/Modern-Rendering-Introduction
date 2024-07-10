using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RotateAroundAxis : MonoBehaviour
{
    public float rotationTime;
    public Vector3 axis;
    public float angle;

    float currentAngle;
    
    void Update()
    {
        float a = Time.deltaTime * angle / rotationTime;
        if (currentAngle + a >= angle)
            a = angle - currentAngle;
        if (currentAngle >= angle)
            return;
        transform.RotateAround(transform.position, axis, a);
        currentAngle += a;
    }
}
