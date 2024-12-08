using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

public class RasterizationQuads : MonoBehaviour
{
    public BackgroundGrid grid;
    public Triangle triangle;
    public GameObject quad;
    
    public bool showGrid;
    public bool showQuads;
    public bool showGridCenters;
    public bool showColoredQuads;
    public bool showTriangle;

    public Color centerColor;
    public Color insideQuadColor;
    public Color outsideQuadColor;
    public float centerSize = 0.01f;

    List<GameObject> gridCenters = new();
    List<GameObject> overlappingSquares = new();
    List<GameObject> overlappingColoredSquares = new();
    
    void Start()
    {
        grid.enabled = false;
        
        // Spawn grid centers:
        for (int x = -grid.lineCount; x < grid.lineCount; x += 2)
        {
            for (int y = -grid.lineCount; y < grid.lineCount; y += 2)
            {
                var quadCenter = new Vector2(x | 1, y | 1) + (Vector2)grid.transform.position;
                // if (x % 2 == 0 && y % 2 == 0)
                // {
                //     var quadCenter = new Vector2(x, y) + (Vector2)grid.transform.position;
                //     checkCenter *= grid.scale;
                //     // SpawnCenter(checkCenter);

                //     if (IntersectsTriangle(checkCenter))
                //         SpawnQuad(quadCenter, grid.scale * 2);
                // }

                bool spawnQuad = false;
                for (int ix = 0; ix < 2; ix++)
                    for (int iy = 0; iy < 2; iy++)
                    {
                        var pixelCenter = (new Vector2(x + ix + 0.5f, y + iy + 0.5f) + (Vector2)grid.transform.position) * grid.scale;
                        spawnQuad |= InsideTriangle(pixelCenter);
                    }

                var center = new Vector2(x + 0.5f, y + 0.5f) + (Vector2)grid.transform.position;
                center *= grid.scale;

                if (spawnQuad)
                {
                    for (int ix = 0; ix < 2; ix++)
                        for (int iy = 0; iy < 2; iy++)
                        {
                            var pixelCenter = (new Vector2(x + ix + 0.5f, y + iy + 0.5f) + (Vector2)grid.transform.position) * grid.scale;
                            SpawnCenter(pixelCenter);
                            SpawnQuad(overlappingSquares, pixelCenter, grid.scale, insideQuadColor);
                            if (InsideTriangle(pixelCenter))
                                SpawnQuad(overlappingColoredSquares, pixelCenter, grid.scale, insideQuadColor);
                            else
                                SpawnQuad(overlappingColoredSquares, pixelCenter, grid.scale, outsideQuadColor);
                        }
                }

                // if (IntersectsTriangle(quadCenter, grid.scale * 2))
                // {
                //     SpawnCenter(center);
                // }
                //     SpawnQuad(center, grid.scale);
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
        gridCenters.Add(dot.gameObject);
    }

    void SpawnQuad(List<GameObject> gameObjects, Vector2 center, float scale, Color color)
    {
        var q = Instantiate(quad, transform);

        q.GetComponent<MeshRenderer>().material.color = color;
        q.transform.position = center;
        q.transform.localScale = new Vector3(scale, scale, 1);

        gameObjects.Add(q);
    }
    
    bool IntersectsTriangle(Vector2 boxCenter, float scale)
    {
        Vector2 a = triangle.a;
        Vector2 b = triangle.b;
        Vector2 c = triangle.c;
        
        Vector2 halfExtents = new Vector2(scale / 2, scale / 2);

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

    void Update()
    {
        grid.gameObject.SetActive(showGrid);

        foreach (var c in gridCenters)
            c.SetActive(showGridCenters);

        foreach (var o in overlappingSquares)
            o.SetActive(showQuads);
        
        foreach (var o in overlappingColoredSquares)
            o.SetActive(showColoredQuads);

        triangle.gameObject.SetActive(showTriangle);
    }
}
