# Camera

In 3D, a camera is a virtual object that transform a 3D scene into a 2D image, exactly like real-life camera works. This operation of going from a 3D space to 2D is called a projection so we can say that the 3D world gets projected to a screen. This is a process that should feel very intuitive as we're used to handle cameras as they became so common in mobile phones.

## How a camera is defined

We need a set of properties to define the characteristics of our camera, for that we'll use the vectors we've seen in the previous chapiters.

The camera is an object in 3D, so it have a position vector of 3 component storing it's location in space. The camera also need a direction to where it's pointing to, this will represent the direction we want to film, so we need another vector to represent the direction.

When rendering a scene, we often limit from how far we can see objects on the camera level, this is mainly an optimization to avoid rendering things that are too far and probably wouldn't contribute to the look of the scene but it's also important to keep a reasonable range of distance between the camera and the scene's object to avoid precision issues. These limits are controlled by the near and far planes of the camera, everything outside of those planes (farther than the far plane or between the near plane and the camera position) will get culled out.

![](Media/Recordings/Camera%2001.png)

This alone is not enough, we need to constrain the size of the camera somehow to focus on a particular part of the 3D scene. This is called the field of view. Changing the FoV of a camera is the same as using the zoom on a true camera, it effectively changes how much of the 3D scene you're viewing.

![](Media/Recordings/Camera%2000.gif)

There is another kind of camera that is a bit less intuitive as it doesn't really exists in real life. It's a camera that perform an orthographic projection instead of the standard perspective projection we're used to. In this projection, the perception of depth doesn't exists as the size of the objects on screen are not defined by their distance to the camera. This kind of camera is simpler to model because it doesn't have a field of view, instead it's a 2D size that determines what the camera can see.

## Exposure

Because we're using physical values to model our scene, we need to take that into account in the camera. Though this can quickly become quite complicated to take in account the lens and the sensor of the camera so we'll focus on a simple model for now. The last parameter we need to add is exposure, it allows us to control the luminosity of the final image. Usually exposure is the result of the combination of several settings on a camera but this simplification allows us to avoid the complexity of having to handle [shutter speed](https://en.wikipedia.org/wiki/Shutter_speed), [ISOs](https://en.wikipedia.org/wiki/Film_speed#ISO), [aperture](https://en.wikipedia.org/wiki/Aperture), etc.

## References

https://en.wikipedia.org/wiki/Shutter_speed

https://en.wikipedia.org/wiki/Film_speed#ISO

https://en.wikipedia.org/wiki/Aperture
