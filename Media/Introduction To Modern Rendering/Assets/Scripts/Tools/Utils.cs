using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

[InitializeOnLoad]
public static class Utils 
{
    static BackgroundAxises axises;
    static BackgroundGrid grid;

    static Utils() => Init();
    
    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
    static void Init()
    {
        axises = Object.FindObjectOfType<BackgroundAxises>(true);
        grid = Object.FindObjectOfType<BackgroundGrid>(true);
    }

    public static Color GetXAxisColor()
    {
        if (axises != null)
            return axises.xAxisColor;
        if (grid != null)
            return grid.xAxisColor;
        return Color.white;
    }

    public static Color GetYAxisColor()
    {
        if (axises != null)
            return axises.yAxisColor;
        if (grid != null)
            return grid.yAxisColor;
        return Color.white;
    }

    public static Color GetZAxisColor()
    {
        if (axises != null)
            return axises.zAxisColor;
        if (grid != null)
            return grid.zAxisColor;
        return Color.white;
    }
}
