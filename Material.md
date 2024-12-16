# Material

In realtime compute graphics a **Material** is an object that contains the different properties needed to render a surface. There properties can be textures, colors, numbers, etc. These properties usually map directly to the data used in shaders to render the objects, this is why most material also references a shader that is able to read the material data. Most 3D engines now provide the possibility to create new shaders and use materials to store values to provide for this shader to render a particular object.

For example, if we take an opaque object with a simple PBR shader taking albedo, normals, metallic and roughness, the material could store values for those inputs like so:
- Albedo: Solid color or a 2D Texture
- Normals: 2D Texture
- Metallic: floating point value or a 2D Texture
- Roughness: floating point value or a 2D Texture

Usually materials are categorized in 3 distinct parts, let's take a look at them

## Unlit Materials

These materials are a bit special as their shader doesn't implement lighting code, which makes them very fast to render. They can either be opaque or transparent and are usually used to display text or UI elements, or mimic the bright parts of an emissive object, this is an optimization that we often see as the emissive part is brighter than the lighting it recieves, it doesn't need to take it into account.

## Lit Materials



## Volume Materials

These materials are used to model participating media or "fog", they are evaluated throughout the a volume and create an optical density that looks like fog. These material are often used to represent fog, clouds, particles in air, etc.

To give this volumetric feeling, volume materials need to be evaluated multiple times throughout the depth which makes them particularly expensive, that's why in realtime they are used for specialized effects whereas the regular fog or atmospheric scattering uses an implementation that is not parametrized through materials hence allowing further optimizations.

