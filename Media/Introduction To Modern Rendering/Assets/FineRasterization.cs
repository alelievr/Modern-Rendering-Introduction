using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using static Unity.Mathematics.math;

public class FineRasterization : MonoBehaviour
{
    const int fineTileSize = 8;
    
    public BackgroundGrid tileGrid;
    public Triangle triangle;
    public GameObject quad;
    
    public bool showTileGrid;
    public bool showFineGrid;
    public bool showGridCenters;
    public bool showTriangleOverlapTileGrid;
    public bool showTriangleOverlapFineGrid;
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

                if (IntersectsTriangle(center))
                {
                    SpawnQuad(tileOverlappingSquares, center, tileGrid.scale);

                    SpawnLinesInsideTile(center, tileGrid.scale);
                }

                // if (InsideTriangle(center))
                //     SpawnQuad(center);
            }
        }
    }

    void SpawnLinesInsideTile(Vector2 center, float tileGridScale)
    {
        var corner = center - new Vector2(tileGridScale / 2, tileGridScale / 2);

        // Y lines
        for (int x = 0; x < fineTileSize + 1; x++)
        {
             var line = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();
             line.dotted = false;
             var start = corner + new Vector2(x / (float)fineTileSize, 0) * tileGridScale;
             var end = corner + new Vector2(x / (float)fineTileSize, 1) * tileGridScale;
             line.UpdateLine(start, end);
             
             fineRasterLines.Add(line.gameObject);
        }
        
        // X lines
        for (int y = 0; y < fineTileSize + 1; y++)
        {
            var line = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();
            line.dotted = false;
            var start = corner + new Vector2(0, y / (float)fineTileSize) * tileGridScale;
            var end = corner + new Vector2(1, y / (float)fineTileSize) * tileGridScale;
            line.UpdateLine(start, end);
             
            fineRasterLines.Add(line.gameObject);
        }
        
        for (int x = 0; x < fineTileSize; x++)
        for (int y = 0; y < fineTileSize; y++)
        {
            Vector2 p = corner + new Vector2(x + 0.5f, y + 0.5f) / fineTileSize * tileGridScale;
            
            SpawnCenter(p);
            
            if (InsideTriangle(p))
                SpawnQuad(fineOverlappingSquares, p, tileGridScale / fineTileSize);
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

    bool IntersectsTriangle(Vector2 boxCenter)
    {
        Vector2 a = triangle.a;
        Vector2 b = triangle.b;
        Vector2 c = triangle.c;
        
        Vector2 halfExtents = new Vector2(tileGrid.scale / 2, tileGrid.scale / 2);

        // Calculate the box corners
        Vector2 boxMin = boxCenter - halfExtents;
        Vector2 boxMax = boxCenter + halfExtents;

        // Check overlap on all axes
        if (!OverlapOnAxis(a, b, c, boxMin, boxMax, new Vector2(1, 0))) return false;  // Box X-axis
        if (!OverlapOnAxis(a, b, c, boxMin, boxMax, new Vector2(0, 1))) return false;  // Box Y-axis
        
        // Triangle edges' normals
        if (!OverlapOnAxis(a, b, c, boxMin, boxMax, EdgeNormal(a, b))) return false;
        if (!OverlapOnAxis(a, b, c, boxMin, boxMax, EdgeNormal(b, c))) return false;
        if (!OverlapOnAxis(a, b, c, boxMin, boxMax, EdgeNormal(c, a))) return false;

        return true;  // All axes overlap
    }

    Vector2 EdgeNormal(Vector2 point1, Vector2 point2)
    {
        Vector2 edge = point2 - point1;
        return new Vector2(-edge.y, edge.x).normalized;
    }

    bool OverlapOnAxis(Vector2 a, Vector2 b, Vector2 c, Vector2 boxMin, Vector2 boxMax, Vector2 axis)
    {
        // Project triangle points onto the axis
        float minTriangle = Vector2.Dot(axis, a);
        float maxTriangle = minTriangle;
        float projection = Vector2.Dot(axis, b);
        minTriangle = Mathf.Min(minTriangle, projection);
        maxTriangle = Mathf.Max(maxTriangle, projection);
        projection = Vector2.Dot(axis, c);
        minTriangle = Mathf.Min(minTriangle, projection);
        maxTriangle = Mathf.Max(maxTriangle, projection);

        // Project the box onto the axis
        Vector2[] boxCorners = {
            new Vector2(boxMin.x, boxMin.y),
            new Vector2(boxMax.x, boxMin.y),
            new Vector2(boxMin.x, boxMax.y),
            new Vector2(boxMax.x, boxMax.y)
        };

        float minBox = Vector2.Dot(axis, boxCorners[0]);
        float maxBox = minBox;
        for (int i = 1; i < boxCorners.Length; i++)
        {
            projection = Vector2.Dot(axis, boxCorners[i]);
            minBox = Mathf.Min(minBox, projection);
            maxBox = Mathf.Max(maxBox, projection);
        }

        // Check for overlap
        return !(minTriangle > maxBox || maxTriangle < minBox);
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

    void SpawnQuad(List<GameObject> output, Vector2 center, float scale)
    {
        var q = Instantiate(quad, transform);

        q.transform.position = center;
        q.transform.localScale = new Vector3(scale, scale, 1);

        output.Add(q);
    }

    void Update()
    {
        tileGrid.gameObject.SetActive(showTileGrid);

        foreach (var c in fineGridCenters)
            c.SetActive(showGridCenters);

        foreach (var o in tileOverlappingSquares)
            o.SetActive(showTriangleOverlapTileGrid);
        
        foreach (var o in fineOverlappingSquares)
            o.SetActive(showTriangleOverlapFineGrid);

        foreach (var o in fineRasterLines)
            o.SetActive(showFineGrid);
        
        triangle.gameObject.SetActive(showTriangle);
    }
}
