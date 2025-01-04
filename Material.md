# Material

In real-time computer graphics, a **Material** is an object that contains the various properties needed to render a surface. These properties can include textures, colors, numbers, etc. They usually map directly to the data used in shaders to render the objects, which is why most materials also reference a shader capable of reading the material data. Most 3D engines now provide the ability to create new shaders and use materials to store values for these shaders to render a particular object.

For example, if we take an opaque object with a simple PBR shader that takes albedo, normals, metallic, and roughness as inputs, the material could store values for those inputs as follows:
- Albedo: Solid color or a 2D Texture
- Normals: 2D Texture
- Metallic: Floating point value or a 2D Texture
- Roughness: Floating point value or a 2D Texture

Materials are usually categorized into three distinct parts. Let's take a look at them.

## Unlit Materials

These materials are a bit special as their shader doesn't implement lighting code, which makes them very fast to render. They can either be opaque or transparent and are usually used to display text or UI elements, or mimic the bright parts of an emissive object, this is an optimization that we often see as the emissive part is brighter than the lighting it recieves, it doesn't need to take it into account.

## Lit Materials

Lit materials are the kind of materials that is the most commonly used in games, almost every objects in a scene respond to lighting in some way. There are multiple types of Lit material, often categorized by the parametrization they are using. The parametrization of the material describes how the surface reacts to the lighting and is defined by the lighting algorithm used in the shader of the lit material. Among the most used ones, we can see

- Standard: This is a material type that can be used to render metals, plastics, wood, etc.
- Subsurface scattering: Used for skin, snow, leaves, wax, etc.
- Transparent / Translucent: Used for glass
- Hair: Hairs use a different shading model to account for the complex light scattering happening in hairs due to their shape.

On top of that, most material support a coating system that adds an extra lighting response on top of the base material, this is typically used to represent oil coatings or varnish.

Materials can also be blended between each other as long as the lighting model used is similar. This is interesting to make the transition between two objects less obvious for example. This is a technique that is used a lot in terrain rendering to transition from on type of soil to another.

Lit materials and their parametrization is a complex topic that we'll look at closely when talking about the BSDFs that models lighting interactions with the surface.

## Volume Materials

These materials are used to model participating media or "fog", they are evaluated throughout the a volume and create an optical density that looks like fog. These material are often used to represent fog, clouds, particles in air, etc.

To give this volumetric feeling, volume materials need to be evaluated multiple times throughout the depth which makes them particularly expensive, that's why in realtime they are used for specialized effects whereas the regular fog or atmospheric scattering uses an implementation that is not parametrized through materials hence allowing further optimizations.

## Conclusion

## References

https://academysoftwarefoundation.github.io/OpenPBR/index.html

https://google.github.io/filament/Filament.html
