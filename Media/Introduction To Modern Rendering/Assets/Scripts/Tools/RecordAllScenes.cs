using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

public class RecordAllScenes
{
    [MenuItem("Modern Rendering/Record All Scenes")]
    public static void RecordAll()
    {
        foreach (var assetGUID in AssetDatabase.FindAssets("t:Scene", new []{ "Assets/Scenes" }))
        {
            var path = AssetDatabase.GUIDToAssetPath(assetGUID);
        }
    }
}
