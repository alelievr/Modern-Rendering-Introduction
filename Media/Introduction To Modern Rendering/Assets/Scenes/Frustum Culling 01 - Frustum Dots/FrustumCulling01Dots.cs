using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Unity.VisualScripting;
using UnityEngine;

public class FrustumCulling01Dots : MonoBehaviour
{
    public int dotCount = 50;
    public float dotSize = 0.05f;
    public Vector3 randomRange = new Vector3(1f, 1f, 1f);
    public Color dotOriginalColor = Color.white;
    public Color dotCulledColor = Color.red;

    FrustumPlaneViewer viewer;

    List<GameObject> dots = new();

    List<Dot> visibleDots = new();

    Vector3 previousPlaneNormal = Vector3.zero;

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        viewer = FindAnyObjectByType<FrustumPlaneViewer>(FindObjectsInactive.Include);

        for (int i = 0; i < dotCount; i++)
        {
            var pos = new Vector3(
                Random.Range(-randomRange.x, randomRange.x),
                Random.Range(-randomRange.y, randomRange.y),
                Random.Range(-randomRange.z, randomRange.z)
            );

            if (pos.z < -4)
                continue;

            var dot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
            dot.transform.position = pos;
            dot.size = dotSize;
            dots.Add(dot.gameObject);
        }

        visibleDots.AddRange(dots.ConvertAll(dot => dot.GetComponent<Dot>()));
        foreach (var dot in visibleDots)
            dot.color = dotOriginalColor;
    }

    void Update()
    {
        if (previousPlaneNormal != viewer.selectedPlaneNormal)
        {
            previousPlaneNormal = viewer.selectedPlaneNormal;
            StartCoroutine(CullDots());
        }
    }

    IEnumerator CullDots()
    {
        // delay to visually see the culling effect
        yield return new WaitForSeconds(0.5f);

        foreach (var dot in visibleDots.ToList())
        {
            var planeNormal = viewer.selectedPlaneNormal;
            var planeCenter = viewer.selectedPlaneCenter;
            Debug.DrawRay(planeCenter, planeNormal, Color.green);

            // Check if the dot is in front of the selected plane
            if (Vector3.Dot(planeNormal, dot.transform.position - planeCenter) < 0)
            {
                // Dot is culled
                dot.color = dotCulledColor;
                visibleDots.Remove(dot);
            }
        }
    }
}
