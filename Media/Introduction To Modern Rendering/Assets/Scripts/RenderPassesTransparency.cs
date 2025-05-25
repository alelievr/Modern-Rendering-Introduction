using UnityEngine;
using System.Collections.Generic;

public class RenderPassesTransparency : MonoBehaviour
{
    public GameObject transparentPrefab;
    public int numberOfObjects = 10;
    public float spawnDelay = 0.1f;


    void Start()
    {
        Random.InitState(42);
        StartCoroutine(SpawnTransparentObjects());
    }

    System.Collections.IEnumerator SpawnTransparentObjects()
    {
        var spheres = new List<GameObject>();
        for (int i = 0; i < numberOfObjects; i++)
        {
            Vector3 spawnPosition = new Vector3(Random.Range(-5f, 5f), Random.Range(-5f, 5f), Random.Range(-5f, 5f));
            var go = Instantiate(transparentPrefab, spawnPosition, Quaternion.identity);
            spheres.Add(go);
            var randomColor = new Color(Random.value, Random.value, Random.value, 0.2f); // Semi-transparent color
            go.GetComponent<Renderer>().material.color = randomColor;
            yield return new WaitForSeconds(spawnDelay);
        }

        // destroy them in the reverse order
        for (int i = spheres.Count - 1; i >= 0; i--)
        {
            Destroy(spheres[i]);
            yield return new WaitForSeconds(spawnDelay);
        }
    }
}
