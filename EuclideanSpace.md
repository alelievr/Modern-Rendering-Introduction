# The Euclidean Space

The Euclidean space is the most intuitive type of space to get a feel of, especially in 3D and 2D because all of the rules we know in experience in the real world also apply in this space.

Mathematically, this space is infinite in all directions and to represent the coordinates of this space we usually use floating point numbers.

In this course we'll exclusively look at both 2D and 3D Euclidean spaces.
Most of the time, the 3D space corresponds to the 3D virtual world representing the scene we're trying to render and 2D Euclidean space represent the 2D screen where the 3D world gets rendered.

It's worth noting that the we'll be using the Cartesian coordinate system, this coordinate system have one origin at the coordinate 0, this origin is crossed by axises called X, Y and Z respectively in 3D. These axises are the basis of the coordinate system we'll use to store the positions of the objects in our virtual world.

The last thing missing we need to choose before starting to place objects into our space is the handedness, this basically mean in which direction your axis points. Because this course is based on [Bevy](https://bevyengine.org/), we'll also use the right handed Y up model:

![Handedness](Media/Images/handedness.png) (source: [Bevy documentation](https://bevy-cheatbook.github.io/fundamentals/coords.html))