using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class OrbitalAnimation : MonoBehaviour
{
    public enum Mode
    {
        YRotation,
        None,
    }

    public Transform pivot;
    public Mode mode;
    public float timeToCompleteRotation = 5.0f;

    void Start()
    {
        
    }

    void Update()
    {
        switch (mode)
        {
            case Mode.YRotation:
                transform.RotateAround(pivot.position, Vector3.up, 360 / timeToCompleteRotation * Time.deltaTime);
                break;
        }
    }
}
