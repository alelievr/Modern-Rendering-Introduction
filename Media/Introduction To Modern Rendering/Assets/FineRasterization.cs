using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class FineRasterization : MonoBehaviour
{
    public BackgroundGrid tileGrid;
    public Triangle triangle;
    public GameObject quad;
    
    public bool showGrid;
    public bool showGridCenters;
    public bool showTriangleOverlapGrid;
    public bool showTriangle;

    public Color centerColor;
    public float centerSize = 0.01f;

    List<GameObject> tileOverlappingSquares = new();
    List<GameObject> fineRasterLines = new();
    List<GameObject> fineGridCenters = new();
    List<GameObject> fineOverlappingSquares = new();
    
    void Start()
    {
        tileGrid.enabled = false;
        
        // Spawn tileGrid centers:
        for (int x = -tileGrid.lineCount; x < tileGrid.lineCount; x++)
        {
            for (int y = -tileGrid.lineCount; y < tileGrid.lineCount; y++)
            {
                var center = new Vector2(x + 0.5f, y + 0.5f) + (Vector2)tileGrid.transform.position;
                center *= tileGrid.scale;
                SpawnCenter(center);

                if (InsideTriangle(center))
                    SpawnQuad(center);
            }
        }
    }
    
    float Cross2D(Vector2 u, Vector2 v)
    {
        return u.y * v.x - u.x * v.y;
    }

    // We can ignore the Z dimension
    bool InsideTriangle(Vector2 p)
    {
        if (Cross2D(p - (Vector2)triangle.a, triangle.b - triangle.a) < 0.0f) return false;
        if (Cross2D(p - (Vector2)triangle.b, triangle.c - triangle.b) < 0.0f) return false;
        if (Cross2D(p - (Vector2)triangle.c, triangle.a - triangle.c) < 0.0f) return false;
        return true;
    }

    void SpawnCenter(Vector3 center)
    {
        var dot = Instantiate(Resources.Load("Dot"), transform).GetComponent<Dot>();
        dot.color = centerColor;
        dot.size = centerSize;
        dot.transform.position = center;
        dot.gameObject.SetActive(false);
        fineGridCenters.Add(dot.gameObject);
    }

    void SpawnQuad(Vector2 center)
    {
        var q = Instantiate(quad, transform);

        q.transform.position = center;
        q.transform.localScale = new Vector3(grid.scale, grid.scale, 1);

        overlappingSquares.Add(q);
    }

    void Update()
    {
        grid.gameObject.SetActive(showGrid);

        foreach (var c in fineGridCenters)
            c.SetActive(showGridCenters);

        foreach (var o in overlappingSquares)
            o.SetActive(showTriangleOverlapGrid);
        
        triangle.gameObject.SetActive(showTriangle);
    }
}
