using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RasterizationDepth : MonoBehaviour
{
    public float area = 5;
    public int instanceCount;
    public GameObject instance;
    public float time = 0.2f;

    void Start()
    {
        StartCoroutine(Generate());
    }

    IEnumerator Generate()
    {
        Random.InitState(0);
        int priority = 0;
        for (int i = 0; i < instanceCount; i++)
        {
            var x = Random.Range(-area, area);
            var y = Random.Range(-area, area);
            var z = Random.Range(-area, area);
            var go = Instantiate(instance, new Vector3(x, y, z) + transform.position, Quaternion.identity);
            var mr = go.GetComponent<MeshRenderer>();
            mr.rendererPriority = priority--;
            mr.material.color = Random.ColorHSV(0, 1, 0.6f, 0.8f, 0.5f, 1f);
            yield return new WaitForSeconds(time);
        }
    }
}
