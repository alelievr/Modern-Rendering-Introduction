using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class Matrix02 : MonoBehaviour
{
    public GameObject finalRotation;
    public GameObject rotationX;
    public GameObject rotationY;
    public GameObject rotationZ;
    public TMP_Text uiX;
    public TMP_Text uiY;
    public TMP_Text uiZ;
    public float timeScale = 4;

    Vector3 startPosition;
    
    void Start()
    {
        // startPosition = go.transform.position;
    }

    void Update()
    {
        float angleDegree = Time.timeSinceLevelLoad / timeScale * 360f;
        uiX.text = $"Rotation X Axis: {angleDegree:F0}°";
        uiY.text = $"Rotation Y Axis: {angleDegree:F0}°";
        uiZ.text = $"Rotation Z Axis: {angleDegree:F0}°";
        rotationX.transform.rotation = Quaternion.Euler(angleDegree, 0,0);
        rotationY.transform.rotation = Quaternion.Euler(0, angleDegree,0);
        rotationZ.transform.rotation = Quaternion.Euler(0, 0,angleDegree);

        finalRotation.transform.rotation = rotationX.transform.rotation * rotationY.transform.rotation * rotationZ.transform.rotation;
    }
}