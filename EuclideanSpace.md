# The Euclidean Space

Imagine a space that perfectly reflects our intuitive understanding of the real world – a space where straight lines are the shortest paths between two points, and distances are measured exactly as we expect. This, in essence, is 3D Euclidean space.

In this course we'll exclusively look at 2D and 3D Euclidean spaces. The 3D space is mainly used represent the 3D virtual world of a scene we're trying to render. This is what we call the "World Space" where each object can be identified with a unique position. 2D Euclidean spaces will be useful to work with flat surfaces like a screen where the 3D scene will get projected for example.

// TODO: insert 3D and 2D view of euclidean space without the grid

It's worth noting that the we'll be using the Cartesian coordinate system, it establishes an origin point (0, 0, 0) where three axes, X, Y, and Z, intersect at right angles (same in 2D but without the Z axis). These axes become the foundation for specifying the location of every object in our virtual world as every point in space can be reached by combining these 3 axises. This coordinate system is called the  if you want to learn more.

// TODO: insert view of a moving point in Euclidean space with dotted line for the axises

The last thing missing we need to choose before starting to place objects into our space is the handedness, this mean in which direction our axises points. Since this course is built upon the [Bevy](https://bevyengine.org/) engine, we'll adopt the right-handed Y-up convention.  Imagine your right hand with your thumb pointing along the X-axis, your index finger along the Y-axis (pointing upwards), and your middle finger curled along the Z-axis. This configuration defines the positive direction for each axis.

![Handedness](Media/Images/handedness.png) (source: [Bevy documentation](https://bevy-cheatbook.github.io/fundamentals/coords.html))

With this foundational understanding of Euclidean space in hand, we're now equipped to explore what we can do with Vectors! 

### References

https://en.wikipedia.org/wiki/Cartesian_coordinate_system
https://en.wikipedia.org/wiki/Euclidean_space