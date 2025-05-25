using UnityEngine;

public class MaterialGrid : MonoBehaviour
{
    public int gridWidth = 10;
    public int gridHeight = 10;
    public float spacing = 1.5f;
    public Material baseMaterial;
    public bool disabledByDefault = false;

    void Start()
    {
        if (baseMaterial == null)
        {
            Debug.LogError("Base material is not assigned.");
            return;
        }

        CreateMaterialGrid();
    }

    void CreateMaterialGrid()
    {
        float a = 1;
        float hw = spacing * a * (float)(gridWidth - 1) * 0.5f;
        float hh = spacing * (float)(gridHeight - 1) * 0.5f;

        for (int x = 0; x < gridWidth; x++)
        {
            for (int y = 0; y < gridHeight; y++)
            {
                float metallic = (float)x / (gridWidth - 1);
                float roughness = (float)y / (gridHeight - 1);

                GameObject sphere = GameObject.CreatePrimitive(PrimitiveType.Sphere);

                if (disabledByDefault)
                    sphere.SetActive(false);

                Material sphereMaterial = new Material(baseMaterial);
                sphereMaterial.SetFloat("_Metallic", metallic);
                sphereMaterial.SetFloat("_Glossiness", 1.0f - roughness);
                sphereMaterial.SetFloat("_Smoothness", 1.0f - roughness);

                sphere.GetComponent<Renderer>().material = sphereMaterial;
                sphere.transform.parent = transform;
                sphere.transform.localPosition = new Vector3(x * spacing * a - hw, y * spacing - hh, 0);
            }
        }
    }
}