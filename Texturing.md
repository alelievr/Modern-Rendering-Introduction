# Texturing

Texturing or [Texture Mapping](https://en.wikipedia.org/wiki/Texture_mapping) is very important in rendering, it's so important that GPUs have dedicated hardware to read, filter and decompress those textures on the fly. A texture can contain any kind of data, while it is most used to store a standardized set of material attributes, it can also be used for anything else, even storing animations. Texture mapping is the process of taking a texture and applying on a mesh in a certain way. You can also see the texture mapping system as an indirection used to fetch the surface data of the object.

## UV Mapping

This is the most common way of applying textures to a mesh, like we've seen in the chapter about [meshes](Meshes.md#texture-coordinates-uv), **UV Mapping** uses pre-determined texture coordinates that acts as an indirection for a 2D texture to be applied on a 3D mesh. Because UV coordinates are embedded in the mesh, it means that the texture are authored for this specific mesh, unless other meshes have the same UV layout. This is the case for tileable textures for example where the texture is authored to be repeated multiple times and thus it can be used on meshes than plan for it.

// TODO: gif

### UDIM Mapping

UDIM is an extension of the UV mapping technique, it consist into having multiple UV maps for the mesh, split in squares. This workflow helps when having to handle very high resolution textures by allowing to map multiple textures for a single attribute (albedo for example).

## Ptex

## Planar And Tri-Planar Mapping

## Procedural Mapping

## Conclusion

## References

- 📄 [Texture mapping - Wikipedia](https://en.wikipedia.org/wiki/Texture_mapping)
- 📄 [UV mapping - Wikipedia](https://en.wikipedia.org/wiki/UV_mapping)
- 📄 [Texture Mapping Solutions - Autodesk](https://www.autodesk.com/solutions/texture-mapping)
- 📄 [UDIM Workflow - Modo | Foundry](https://learn.foundry.com/modo/content/help/pages/uving/udim_workflow.html)
- 📄 [Ptex: Per-face Texture Mapping for Production Rendering - Disney](https://media.disneyanimation.com/technology/opensource/ptex/ptex-slides.pdf)
- 📄 [HexTile Demo - GitHub](https://github.com/mmikk/hextile-demo)
