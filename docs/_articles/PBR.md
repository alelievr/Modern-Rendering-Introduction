---
title: "Physically Based Rendering"
order: 40
author: Antoine Lelievre
category: Fundamentals 
layout: post
---

# Physically Based Rendering

[![Zero Day: https://developer.nvidia.com/orca/beeple-zero-day](Media/Images/ZeroDayMeasureOne_1.png)](https://developer.nvidia.com/orca/beeple-zero-day)

Physically based rendering (PBR) is a method for rendering virtual content using equations derived from real-world observations. It often targets photorealistic rendering but PBR can also be used for stylized rendering when more artistic control is given.

PBR has become the industry standard due to its numerous advantages for both artistic creation and engine development. It’s employed in most contemporary films, from matching real-world footage to completely replacing it, and even finds application in highly stylized works from studios like Pixar and Disney.

The benefit of having light behave "as expected in reality" within a virtual world is big. It allows artists to leverage their intuition rather than solely memorizing complex rules.

## Why use PBR?

PBR goes beyond simply rendering photorealistic visuals. It has real-world implications for creating virtual assets and establishes a standardized approach for content creation and rendering.

This standardization enables artists to reuse and share their knowledge across projects and even fosters asset libraries that are made compatible with various engines and renderers thanks to the common set of rules PBR implies. In essence, this standard content creation method functions as an interface. When implemented correctly, it ensures compatibility with any renderer supporting the PBR workflow.

Within PBR, lighting also becomes standardized. The reliance on real-world observation-derived equations necessitates the use of the corresponding units. Consequently, all lighting is quantified in [physical light units](https://en.wikipedia.org/wiki/Luminous_intensity) (e.g., lumens), allowing the use of real-world values within the virtual world.

PBR additionally opens doors to new workflows based on capturing real-world content, a process known as photogrammetry. The concept is straightforward: capture multiple photographs of an existing object and use these images to generate a 3D model. Since real-world data serves as the ultimate reference for PBR content, directly acquiring models from reality holds significant value.

## Separation of Materials and Lighting

A core concept of PBR is the distinct separation between how a surface is modeled and how it interacts with light.

Before the PBR workflow, creating a virtual asset involved building the 3D model in a chosen software, followed by applying textures to define color and light interaction. This texture set could contain lighting information, often baked into the object’s color. However, this presents a challenge for dynamic lighting scenarios. Baked-in lighting information within the model itself conflicts with the renderer’s light simulation, as the renderer interprets it solely as color data, and a lot of information about the light is lost when it’s baked into a model. This workflow placed a heavy burden on artists, requiring them to manage both surface creation and a part of the lighting.

PBR divides 3D content creation into two distinct aspects: materials and lighting. Materials are essentially a set of textures and parameters that define how a surface interacts with light (e.g., surface roughness, metallic properties, color, etc.).

On the lighting part, the renderer expects materials within the scene to provide this standardized set of inputs to ensure the proper functioning of its lighting algorithms. The list of parameters (textures, constants, functions, etc.) is determined by the lighting algorithm we choose. There are several of them depending on the type of objects we’re rendering; we’ll take a look at that in a future chapter.

This separation essentially means that the 3D object will work as expected in any lighting conditions which makes it highly reusable between environments. This is what made PBR so popular in asset libraries, the PBR workflow ensures that the asset you download is compatible with your lighting setup.

It’s important to note that some material information is actually derived from the geometry of the 3D model which is not linked to how the lighting interacts with the surface. For example, ambient occlusion or displacement is derived from the surface of the object and isn’t used during the lighting calculation of the surface.

Here’s an example showcasing how various textures combine to create a photorealistic rendering of a rock. Don’t worry about the specific textures yet; we’ll delve deeper into material creation later.

<table>
  <tr>
    <th>Final Render</th><th>Albedo</th><th>Ambient Occlusion</th>
  </tr>
  <tr>
    <td width=33.3%><img src="Media/Images/PBR Rock Sample/FinalRender.jpg"  alt="Final Render"></td>
    <td width=33.3%><img src="Media/Images/PBR Rock Sample/Albedo.jpg"  alt="Albedo"></td>
    <td width=33.3%><img src="Media/Images/PBR Rock Sample/AO.jpg"  alt="Ambient Occlusion"></td>
  </tr>
</table>

<table>
  <tr>
    <th>Displacement</th><th>Normal</th><th>Roughness</th>
  </tr>
  <tr>
    <td width=33.3%><img src="Media/Images/PBR Rock Sample/Displacement.jpg"  alt="Displacement"></td>
    <td width=33.3%><img src="Media/Images/PBR Rock Sample/Normal.jpg"  alt="Normal"></td>
    <td width=33.3%><img src="Media/Images/PBR Rock Sample/Roughness.jpg"  alt="Roughness"></td>
  </tr>
</table>

## Approximations

Achieving perfect light simulation is computationally prohibitive, as it would involve simulating light interaction with every electron in the scene. Therefore, we rely on established equations describing light behavior on specific objects. One example is [Snell's Law](https://en.wikipedia.org/wiki/Snell%27s_law), which describes how light direction changes when entering a new medium (think of how light bends and refracts when passing through water).

As you'll discover, these formulas can also become computationally expensive. To maintain reasonable frame rates for real-time rendering (typically 30 fps or higher), we'll utilize approximations. These approximations will introduce errors in the rendering and we typically add parameters to help reduce the errors. Future chapters will explore how to measure these errors against a "reference".

For simplicity, our reference won't be the real world, as capturing all the necessary lighting information for an accurate comparison would require a complex setup. If you're interested in practical applications of real-world reference data in video games, you can refer to [The Rendering Of Callisto Protocol](https://advances.realtimerendering.com/s2023/SIGGRAPH2023-Advances-The-Rendering-of-The-Callisto-Protocol-JimenezPetersen.pdf).

## Coherence

It’s important to think about all the components of a renderer to make sure they work with your lighting models, this requirement ensures that all the objects in your scene are receiving light the same way which makes the scene visually coherent. Every part of the lighting also needs to be energy-conserving, i.e. it’s important to respect the amount of incoming and outgoing light when simulating light interactions to make sure that no light is created from nothing or that the material absorbs too much.

At first, this idea of coherence might seem obvious to respect PBR principles but you’ll see that this idea of coherence is highly impractical for real-time applications and that approximations that we rely on to make sure we maintain real-time frame rates are often getting more complicated to make sure that the lighting stays coherent.

## References

- 📄 [Moving Frostbite to PBR - Seb Lagarde (PDF)](https://seblagarde.wordpress.com/wp-content/uploads/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf)
- 📄 [Physically based rendering - Wikipedia](https://en.wikipedia.org/wiki/Physically_based_rendering)
- 📄 [RenderMan - Pixar](https://renderman.pixar.com/)
- 📄 [Physically Based Rendering: From Theory to Implementation (4th Edition)](https://pbr-book.org/4ed/contents)
- 📄 [Luminous intensity - Wikipedia](https://en.wikipedia.org/wiki/Luminous_intensity)
- 📄 [SIGGRAPH 2023: Advances in The Rendering of The Callisto Protocol (PDF)](https://advances.realtimerendering.com/s2023/SIGGRAPH2023-Advances-The-Rendering-of-The-Callisto-Protocol-JimenezPetersen.pdf)
