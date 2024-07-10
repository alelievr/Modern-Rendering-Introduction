using System.Collections;
using System.Collections.Generic;
using System.Linq;
using Unity.VisualScripting;
using UnityEngine;

[ExecuteAlways]
public class Matrix00 : MonoBehaviour
{
    public int squareMatrixSize;
    public float lineWidth;
    public float spacing = 0.2f;
    
    List<Line> lines = new();
    List<AnchoredText> textCells = new();
    
    void OnEnable()
    {
        lines = GetComponentsInChildren<Line>().ToList();
        textCells = GetComponentsInChildren<AnchoredText>().ToList();

        var sq = squareMatrixSize + 1;
        while (lines.Count < sq * 2)
        {
            var l = Instantiate(Resources.Load("Line"), transform).GetComponent<Line>();
            l.dotted = false;
            lines.Add(l);
        }

        while (textCells.Count < squareMatrixSize * squareMatrixSize)
        {
            var t = Instantiate(Resources.Load("AnchoredText"), transform).GetComponent<AnchoredText>();
            textCells.Add(t);
        }

        int i = 0;
        foreach (var t in textCells)
        {
            t.text = Random.value.ToString("F1");
            t.transform.localPosition = new Vector3(i % squareMatrixSize + 0.5f, i / squareMatrixSize + 0.5f, 0) * spacing;
            t.transform.localScale = Vector3.one * 0.4f;
            i++;
        }
    }

    void Update()
    {
        float halfLineWidth = lineWidth / 3.0f;
        for (int x = 0; x <= squareMatrixSize; x++)
        {
            lines[x].UpdateLine(new Vector3(-halfLineWidth, x, 0) * spacing, new Vector3(squareMatrixSize + halfLineWidth, x, 0) * spacing, false);
            lines[x].width = lineWidth;
        }
        for (int y = 0; y <= squareMatrixSize; y++)
        {
            lines[y + squareMatrixSize + 1].UpdateLine(new Vector3(y, -halfLineWidth, 0) * spacing, new Vector3(y, squareMatrixSize + halfLineWidth, 0) * spacing, false);
            lines[y + squareMatrixSize + 1].width = lineWidth;
        }
    }
}
