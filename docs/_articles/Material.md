---
title: "Material"
order: 20
author: Antoine Lelievre
category: Authoring 
layout: post
---

In real-time computer graphics, a **Material** is an object that contains the various properties needed to render a surface. These properties can include textures, colors, numbers, etc. They usually map directly to the data used in shaders to render the objects, which is why most materials also reference a shader capable of reading the material data. Most 3D engines now provide the ability to create new shaders and use materials to store values for these shaders to render a particular object.

For example, if we take an opaque object with a simple PBR shader that takes albedo, normals, metallic, and roughness as inputs, the material could store values for those inputs as follows:
- Albedo: Solid color or a 2D Texture
- Normals: 2D Texture
- Metallic: Floating point value or a 2D Texture
- Roughness: Floating point value or a 2D Texture

Materials are closely linked to the shaders they use, and this relationship is not typically present in low-level APIs. Because of this, the way material systems are implemented can vary greatly depending on the software. In some systems, materials provide the ability to create or modify parts of the shader itself, offering a higher level of control over the final effect and allowing for more customization and flexibility in how materials are rendered.

## Material Types

Materials can be categorized into three distinct parts. Let's take a look at them.

### Unlit Materials

| Unity | Blender |
| --- | --- |
| ![](/assets/Images/HDRP_Unlit.png) | ![](/assets/Images/Blender_Unlit.png) |

> Example of an unlit material in Unity HDRP and Blender.

These materials are a bit special because their shader doesn't implement lighting code, which makes them very fast to render. They can either be opaque or transparent and are usually used to display text or UI elements, or to mimic the bright parts of an emissive object. This is an optimization often used because the emissive part is brighter than the lighting it receives, so it doesn't need to take it into account.

Another interesting use case for unlit materials is when using pre-computed lighting. In this case, the lighting is computed "offline" (during the game development process), and then a simple unlit material is sufficient to display the resulting lighting.

### Lit Materials

| Unity | Blender |
| --- | --- |
| ![](/assets/Images/HDRP_Lit.png) | ![](/assets/Images/Blender_Lit.png) |

> Example of a standard lit material in Unity HDRP and Blender.

Lit materials are the most commonly used type of materials in games. Almost every object in a scene responds to lighting in some way. There are multiple types of lit materials, often categorized by the parametrization they use. The parametrization of the material describes how the surface reacts to lighting and is defined by the lighting algorithm used in the shader of the lit material. Among the most commonly used ones, we can find:

- **Standard**: This material type can be used to render metals, plastics, wood, etc.
- **Subsurface scattering**: Used for skin, snow, leaves, wax, etc.
- **Transparent / Translucent**: Used for glass.
- **Hair**: Hairs use a different shading model to account for the complex light scattering that happens in hair due to their shape.

In addition, most materials support a coating system that adds an extra lighting response on top of the base material. This is typically used to represent oil coatings or varnish.

Lit materials and their parametrization is a complex topic that we will explore in more detail when discussing [BSDFs](https://en.wikipedia.org/wiki/Bidirectional_scattering_distribution_function) (Bidirectional Scattering Distribution Functions) that model lighting interactions with surfaces.

### Volume Materials

| Unity | Blender |
| --- | --- |
| ![](/assets/Images/HDRP_Volume.png) | ![](/assets/Images/Blender_Volume.png) |

> Example of a volume material in Unity HDRP and Blender.

These materials are used to model participating media or "fog." They are evaluated throughout a volume and create an optical density that resembles fog. These materials are often used to represent fog, clouds, particles in the air, etc.

To achieve this volumetric effect, volume materials need to be evaluated multiple times throughout the depth, which makes them particularly expensive. That's why, in real-time rendering, they are typically used for specialized effects. In contrast, regular fog or atmospheric scattering uses an implementation that is not parameterized through materials, allowing for further optimizations.

## Material Authoring

Now that we've seen the how materials are defined from their interaction to lighting, let's take a look at the more artistic part of the creation of the material.

The purpose of the material is still unchanged, i.e. provide the correct parameters for the underlying lighting model to shade the surface. What we'll see in the material authoring process is how these parameters can be computed from algorithms, procedural functions and exposed in other ways by re-parametrization.

Depending on the use case, even some gameplay logic or reaction to environment changes can be baked into the material.

### Material Graph

| Unity | Blender |
| --- | --- |
| ![](/assets/Images/Unity_ShaderGraph.png) | ![](/assets/Images/Blender_ShaderEditor.png) |

> Example of a shader/material editors in Unity HDRP and Blender.

A **Material Graph** is a nodal editor used to create and manipulate materials in a more visual and intuitive way. Instead of manually coding shader functions, artists and developers can use a node-based interface to build the material's properties by connecting different nodes that represent various operators or properties. These graphs abstract the underlying shader code, making it easier to design and test materials without needing deep programming knowledge.

The material graph typically operates in a **data-flow** style, where each node processes inputs (like textures or numerical values) and outputs them in a way that eventually determines the material's visual appearance. The benefit of this system is that it enables rapid prototyping, debugging, and tweaking of material properties without needing to rewrite code.

It is worth noting that the logic of a material graph is often decoupled from the inputs of the lighting algorithm, which are typically represented by a monolithic node. This separation ensures that the logic within the graph can be adjusted safely without compromising the integrity of the lighting calculations.

### Material Layering

[![](/assets/Images/MaterialLayering.png)](https://unity3d.com/files/solutions/photogrammetry/Unity-Photogrammetry-Workflow-Layered-Shader_v2.pdf)

> Example of a layered material in Unity HDRP with 3 different surfaces.

### Material Layering

Material Layering is a technique that allows you to combine different materials on a single surface. Instead of creating a single monolithic material that defines all of an object’s characteristics, layering enables you to stack multiple materials on top of one another and blend them. Each layer can represent a different surface, such as grass, dirt, rocks, etc. The interpolation can then be achieved through a procedural function, a texture created by artists, or a height-based blend that uses the heightmap of each layer to determine which one to display.

This system provides a non-destructive approach to material creation, allowing artists to make changes to specific layers without affecting others. In essence, material layering blends the material properties just before they are sent to the lighting algorithm.

Material layering is a technique commonly used for terrain rendering, but it can also be applied to create complex materials like chipped paint or rusty metals.

### Procedural Materials

[![](https://cdn.80.lv/api/upload/content/09/images/6303dc9595c76/widen_920x0.jpeg)](https://80.lv/articles/creating-a-procedural-sci-fi-material-generator-in-substance-3d-designer-toolbag/)

> Image from Creating a Procedural Sci-Fi Material Generator in Substance 3D Designer & Toolbag at [80.lv](80.lv).

Procedural materials are generated algorithmically rather than being based on static textures or manually painted data. These materials are defined by mathematical functions or rules that describe their appearance, which means they can be created and modified without relying on traditional image-based textures. Procedural materials are extremely flexible, allowing for nearly infinite variation based on parameters, noise patterns, and other inputs.

A major advantage of procedural materials is that they are resolution-independent, meaning they can be scaled to almost any size without losing quality. They are also dynamic and adaptable, often responding to changes in the environment, such as weather or lighting conditions.

Additionally, procedural techniques allow for easy iteration and variation, as small changes in input values can produce drastically different results. This makes them ideal for applications where unique materials need to be created at runtime, such as generating random textures, terrain types, or effects dependent on game state or player interactions.

Nevertheless, procedural materials often need to be evaluated every frame, making them expensive in real-time and limiting the complexity of procedural effects achievable in such scenarios. A common approach to mitigate these costs is to bake the procedural material output and re-evaluate it only when necessary, effectively turning the procedural material into a regular one.

## Other Uses

Since materials are often used to represent surfaces, they can also store the physical properties of those surfaces required for gameplay. For example, the footstep sound of a character needs to match the type of surface the player is walking on. Similarly, the friction coefficient or bounciness can be stored for the game's physics system. 

Although this course focuses solely on properties related to rendering, it is still interesting to note that the same object can be utilized by several systems.

## Conclusion

Materials play a fundamental role in real-time rendering, defining how surfaces interact with light and other environmental factors. From the simplicity of unlit materials, used to render objects without lighting influence, to the complexity of lit and volume materials, which simulate phenomena like subsurface scattering or fog, each material type serves a distinct purpose in achieving diverse visual effects. While the material entity itself may seem abstract, as it primarily exposes inputs for underlying algorithms, understanding its use cases and types provides valuable insight into their functionality.

By exploring material authoring, we see how the creation process shifts from purely technical parameters to a more artistic approach. Material authoring demonstrates how parameters for lighting models can be derived from algorithms, procedural functions, and re-parameterization. Through tools like material graphs, artists and developers gain an intuitive interface to design and manipulate materials, allowing for rapid prototyping and iteration. Moreover, techniques such as material layering and procedural generation enable the creation of highly detailed and adaptable materials, whether through blending layers for terrains or generating textures algorithmically for unique effects.

Understanding these concepts equips you with the knowledge to leverage materials effectively in rendering. It lays the groundwork for exploring more abstract and advanced topics, ensuring you can navigate the interplay between artistic vision and technical implementation.

## References

- 📄 [OpenPBR Documentation - Academy Software Foundation](https://academysoftwarefoundation.github.io/OpenPBR/index.html)
- 📄 [Filament Documentation - Google](https://google.github.io/filament/Filament.html)
- 📄 [Lit Shader Documentation - Unity](https://docs.unity3d.com/Packages/com.unity.render-pipelines.high-definition@latest?subfolder=/manual/lit-material.html)
- 📄 [Principled BSDF Shader - Blender Manual](https://docs.blender.org/manual/fr/dev/render/shader_nodes/shader/principled.html)
- 📄 [Procedural texture - Wikipedia](https://en.wikipedia.org/wiki/Procedural_texture)
