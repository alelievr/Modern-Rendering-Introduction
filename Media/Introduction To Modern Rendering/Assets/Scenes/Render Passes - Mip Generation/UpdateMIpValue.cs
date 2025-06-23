using TMPro;
using UnityEngine;

public class UpdateMIpValue : MonoBehaviour
{
    public float value;
    public Material material;
    public TMP_Text text;

    void Start()
    {
        
    }

    void Update()
    {
        material.SetFloat("_MipMapLevel", value);
        text.text = "Mip Level: " + value.ToString("F2");
    }
}
