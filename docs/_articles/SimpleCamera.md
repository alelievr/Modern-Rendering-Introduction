---
title: "Simple Camera"
order: 30
author: Antoine Lelievre
category: Math3D 
layout: post
---

In 3D, a camera is a virtual object that transform a 3D scene into a 2D image, exactly like real-life camera works. This operation of going from a 3D space to 2D is called a projection so we can say that the 3D world gets projected to a screen. This is a process that should feel very intuitive as we're used to handle cameras as they became so common in mobile phones.

## Let's build a camera

We need a set of properties to define the characteristics of our camera, for that we'll use the vectors we've seen in the previous chapters.

The camera is an object in 3D, so it have a position vector of 3 component storing it's location in space. The camera also need a direction to where it's pointing to, this will represent the direction we want to film, so we need another vector to represent the direction.

When rendering a scene, we often limit from how far we can see objects on the camera level, this is mainly an optimization to avoid rendering things that are too far and probably wouldn't contribute to the look of the scene but it's also important to keep a reasonable range of distance between the camera and the scene's object to avoid precision issues. These limits are controlled by the near and far planes of the camera, everything outside of those planes (farther than the far plane or between the near plane and the camera position) will get culled out.

![](/assets/Recordings/Camera%2001.png)

This alone is not enough, we need to constrain the size of the camera somehow to focus on a particular part of the 3D scene. This is called the field of view. Changing the FoV of a camera is the same as using the zoom on a true camera, it effectively changes how much of the 3D scene you're viewing. As you can see this forms a kind of square based pyramid shape with the tip pointing to the camera position cut off, this shape is called a [Frustum](https://en.wikipedia.org/wiki/Frustum), it's a kind of mathematical shape that is often used to represent what that camera can see.

![](/assets/Recordings/Camera%2000.gif)

This kind of projection with the field of view is called [perspective](https://en.wikipedia.org/wiki/Perspective_(graphical)) projection, this is the kind of projection we're most used to as it keeps nicely the notion of distance (far objects appear smaller than close ones).

One nice way of visualizing how the objects inside the frustum gets projected onto the 2D surface is to imagine the Frustum and all the objects inside getting squished towards the camera. This way of interpreting the transformation gives us a good insight of how the 3D space gets turned into 2D space where the Z coordinate essentially disappears. In this example, notice how the scene gets projected on the near plane of the frustum, this essentially is how we get the image of the 3D scene.

![](/assets/Recordings/Camera%2002.gif)

There is another kind of camera that is a bit less intuitive as it doesn't really exists in real life. It's a camera that perform an orthographic projection instead of the standard perspective projection we're used to. In this projection, the perception of depth doesn't exists as the size of the objects on screen are not defined by their distance to the camera. This kind of camera is simpler to model because it doesn't have a field of view, instead it's a 2D size that determines what the camera can see. This is the projection I've been using for most of the illustrations, it has the neat property of keeping lines parallels to each other and makes really clean schematics.

## Screen

The last missing step to get our camera working is some way to store what the camera sees. For this we will simply use a special kind of texture called Render Target (or Frame Buffer, Render Texture depending on the API). This Render Target texture has the same properties as a regular texture except that the GPU can write to it which allows our objects to be rendered inside this texture. Because it's a texture, it means that it has a resolution (width and height) in pixels or more specifically because it's a texture, we call it a [texel](https://en.wikipedia.org/wiki/Texel_(graphics)) (because it doesn't necessarily represent a physical pixel of a monitor). This texture width and height determines the resolution of the final image we'll generate and it also drives the [aspect ratio](https://en.wikipedia.org/wiki/Aspect_ratio_(image)) of the camera (this is often represented by a single number which is width divided by height).

## Exposure

Because we're using physical values to model our scene, we need to take that into account in the camera. Though this can quickly become quite complicated to take in account the lens and the sensor of the camera so we'll focus on a simple model for now.
The last parameter we need to add is exposure, it allows us to control the luminosity of the final image.
Usually exposure is the result of the combination of several settings on a camera but this simplification allows us to avoid the complexity of having to handle [shutter speed](https://en.wikipedia.org/wiki/Shutter_speed), [ISOs](https://en.wikipedia.org/wiki/Film_speed#ISO), [aperture](https://en.wikipedia.org/wiki/Aperture), etc.

## References

- 📄 [Shutter speed - Wikipedia](https://en.wikipedia.org/wiki/Shutter_speed)
- 📄 [Film speed (ISO) - Wikipedia](https://en.wikipedia.org/wiki/Film_speed#ISO)
- 📄 [Aperture - Wikipedia](https://en.wikipedia.org/wiki/Aperture)
- 📄 [Texel - Wikipedia](https://en.wikipedia.org/wiki/Texel_(graphics))
- 📄 [Aspect ratio - Wikipedia](https://en.wikipedia.org/wiki/Aspect_ratio_(image))
