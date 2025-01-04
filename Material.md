# Material

In real-time computer graphics, a **Material** is an object that contains the various properties needed to render a surface. These properties can include textures, colors, numbers, etc. They usually map directly to the data used in shaders to render the objects, which is why most materials also reference a shader capable of reading the material data. Most 3D engines now provide the ability to create new shaders and use materials to store values for these shaders to render a particular object.

For example, if we take an opaque object with a simple PBR shader that takes albedo, normals, metallic, and roughness as inputs, the material could store values for those inputs as follows:
- Albedo: Solid color or a 2D Texture
- Normals: 2D Texture
- Metallic: Floating point value or a 2D Texture
- Roughness: Floating point value or a 2D Texture

Materials are usually categorized into three distinct parts. Let's take a look at them.

## Unlit Materials

These materials are a bit special because their shader doesn't implement lighting code, which makes them very fast to render. They can either be opaque or transparent and are usually used to display text or UI elements, or to mimic the bright parts of an emissive object. This is an optimization often used because the emissive part is brighter than the lighting it receives, so it doesn't need to take the lighting into account.

Another interesting use case for unlit materials is when using pre-computed lighting. In this case, the lighting is computed "offline" (during the game development process), and then a simple unlit material is sufficient to display the resulting lighting.

## Lit Materials

Lit materials are the most commonly used type of materials in games. Almost every object in a scene responds to lighting in some way. There are multiple types of lit materials, often categorized by the parametrization they use. The parametrization of the material describes how the surface reacts to lighting and is defined by the lighting algorithm used in the shader of the lit material. Among the most commonly used ones, we can find:

- **Standard**: This material type can be used to render metals, plastics, wood, etc.
- **Subsurface scattering**: Used for skin, snow, leaves, wax, etc.
- **Transparent / Translucent**: Used for glass.
- **Hair**: Hairs use a different shading model to account for the complex light scattering that happens in hair due to their shape.

In addition, most materials support a coating system that adds an extra lighting response on top of the base material. This is typically used to represent oil coatings or varnish.

Materials can also be blended with each other as long as the lighting models used are similar. This is useful for making the transition between two objects less obvious, for example. This technique is commonly used in terrain rendering to transition from one type of soil to another.

Lit materials and their parametrization is a complex topic that we will explore in more detail when discussing [BSDFs](https://en.wikipedia.org/wiki/Bidirectional_scattering_distribution_function) (Bidirectional Scattering Distribution Functions) that model lighting interactions with surfaces.

## Volume Materials

These materials are used to model participating media or "fog." They are evaluated throughout a volume and create an optical density that resembles fog. These materials are often used to represent fog, clouds, particles in the air, etc.

To achieve this volumetric effect, volume materials need to be evaluated multiple times throughout the depth, which makes them particularly expensive. That's why, in real-time rendering, they are typically used for specialized effects. In contrast, regular fog or atmospheric scattering uses an implementation that is not parameterized through materials, allowing for further optimizations.

## Conclusion

Materials play a fundamental role in real-time rendering, as they define how surfaces interact with light and other environmental factors. From the simplicity of unlit materials, used to render objects with no lighting influence, to the complexity of lit and volume materials, which simulate phenomena like subsurface scattering or fog, each material type serves a distinct purpose in achieving different visual effects. While the material entity itself can remain somewhat mysterious, as it merely exposes the inputs for underlying algorithms, it's valuable to explore the various use cases and types. This will provide you with a clearer understanding of their function, which will be especially helpful when we delve into more abstract concepts later on. By familiarizing yourself with these material types, you'll gain insight into the practical applications of materials in rendering, laying the foundation for understanding more advanced topics as we move forward.

## References

https://academysoftwarefoundation.github.io/OpenPBR/index.html

https://google.github.io/filament/Filament.html
