---
title: "The Euclidean Space"
order: 10
author: Antoine Lelievre
category: Math3D 
layout: post
---

Imagine a space that perfectly reflects our intuitive understanding of the real world – a space where straight lines are the shortest paths between two points, and distances are measured exactly as we expect. This, in essence, is 3D Euclidean space.

In this course we'll exclusively look at 2D and 3D Euclidean spaces. The 3D space is mainly used represent the virtual world of a scene containing some objects. This is what we call the "World Space" where each object can be identified with a unique position.  
2D Euclidean spaces will be useful to work with flat surfaces like a screen where the 3D scene will get projected on or doing specific operations on the surface of objects like applying a texture on the surface of a plane.

<table>
  <tr><th>2D</th><th>3D</th></tr>
  <tr>
    <td><img src="Media/Recordings/Euclidean%20Space%2000.png"  alt="2D" width = 100% height = 100% ></td>
    <td><img src="Media/Recordings/Euclidean%20Space%2001.png"  alt="3D" width = 100% height = 100% ></td>
  </tr>
</table>

It's worth noting that the we'll be using the Cartesian coordinate system, it establishes an origin point (0, 0, 0) where three axes, X, Y, and Z, intersect at right angles (same in 2D but without the Z axis). These axes become the foundation for specifying the location of every object in our virtual world as every point in space can be reached by combining these 3 axises. 

![](/assets/Recordings/Euclidean%20Space%2002.gif)

The last thing missing we need to choose before starting to place objects into our space is the handedness, this mean in which direction our axises points. Imagine your right hand with your thumb pointing along the X-axis, your index finger along the Y-axis (pointing upwards), and your middle finger curled along the Z-axis. This configuration defines the positive direction for each axis.

![Handedness](/assets/Images/handedness.png) (source: [Bevy documentation](https://bevy-cheatbook.github.io/fundamentals/coords.html))

For this course we'll adopt the left-handed Y-up convention but you can use whichever you prefer as long as you're not mixing conventions during transformations operations.

With this foundational understanding of Euclidean space in hand, we're now equipped to explore what we can do with Vectors!

### References

- 📄 [Cartesian coordinate system - Wikipedia](https://en.wikipedia.org/wiki/Cartesian_coordinate_system)
- 📄 [Euclidean space - Wikipedia](https://en.wikipedia.org/wiki/Euclidean_space)
- 📄 [Coordinate Systems (Direct3D 9) - Win32 apps | Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/direct3d9/coordinate-systems)
