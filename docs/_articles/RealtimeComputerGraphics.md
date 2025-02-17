---
title: "Realtime Computer Graphics"
order: 30
author: Antoine Lelievre
category: Fundamentals 
layout: post
---

# Realtime Computer Graphics

First, I'd like to say a few words about computer graphics, and specifically real-time rendering.
Real-time graphics leverage the power of hardware acceleration, particularly through GPUs, and employ various approximations and simplifications to optimize the rendering process. It is a field that is at the crossroad of programming, art and physics with a strong emphasis on optimization due to the extreme workloads the graphic pipeline handles.

## The Importance of Perception in Computer Graphics

The essence of computer graphics lies in the creation of visual content that ultimately appears on a screen and is perceived by a human viewer (hopefully it's still the case). This element of human perception sets computer graphics apart from other fields of computer science. What ultimately matters is how the viewer perceives the rendered image or scene and not the "correctness" of the algorithms used to create the image.

Human vision is susceptible to illusions and perceptual tricks, which influences the design of graphical algorithms. In many instances, algorithms prioritize perceptual appeal over mathematical accuracy. For example, perceptual parameterization or visually pleasing distributions, such as blue noise rather than quasirandom sequences, are often preferred.
It also influences heavily how artistic controls over a certain alogrithm are exposed, for example when moving the knobs of a color picker, it's expected that the changes happen in a linear fashion as you move the slider. The problem is that our eyes doens't percieve colors linearly, so what we call linear is in fact non-linear for our code. You'll learn more about that in the chapter about color spaces and perception.

## The Subjectivity of Correctness

A unique challenge in computer graphics is defining what constitutes a "correct" rendering. Since the goal is to create visually appealing images, correctness can be subjective. For example, while a path-traced reference image from another renderer might be used as a benchmark, this reference is not infallible. Similarly, in the creation of highly stylized content with custom lighting or fantastical effects, the notion of correctness becomes even more elusive, as these elements do not exist in the real world.

This subjectivity means that intuition plays a significant role in developing rendering algorithms. Algorithms might work mainly because they produce visually acceptable results, even if they are not technically accurate or even some times the result can be wrong but hidden by another algorithm further down the pipeline. Developers must be vigilant, as small errors can slip into rendering algorithms and go undetected until a specific scene highlights the issue. And it's very easy to be tricked into doing something that we think is okay while in practice it can easily introduce rendering errors that trickes down and gets amplified by other algorithms in the pipeline.

## The Challenges of Testing and Validation

Testing in computer graphics presents a unique set of challenges. Unlike other areas of software development, where unit tests and defined performance metrics can pinpoint errors, graphics algorithms rely heavily on visual output. Mathematical metrics often fall short in assessing whether a change improves or degrades the visual outcome.

The vast combination of effects, 3D models, and lighting scenarios makes it practically impossible to test every potential case exhaustively. As a result, we often resort to what are known as graphics tests that serve as benchmarks. When an algorithm is changed, developers compare the new output against these reference images. If the images differ significantly, the developer must update the reference image to reflect the new standard.

This process inherently involves a degree of intuition. Developers must use their judgment to decide whether the visual changes represent an improvement. Unlike traditional computer science, where clearly defined performance metrics or unit tests can quickly identify errors, graphical testing relies heavily on subjective human evaluation.

Understanding these challenges is crucial. It allows us to develop more effective graphics algorithms, despite the inherent difficulties in defining and testing "correctness" in the visual domain. In certain cases it can also be beneficial to give up quality for performances, in which case the performance improvement must outweight the visual downgrade, this is another example where a subjective decision needs to be made.

## Video Games

A significant portion of real-time renderers is utilized in games, as they effectively capture the responsiveness of gameplay while delivering compelling visuals that enhance the overall experience.

In games, various factors influence visual quality, including textures, meshes, and visual effects. Numerous techniques can be employed to achieve impressive results by utilizing alternative methods and innovative thinking. This is particularly important given that many approximations are used; it’s easy to assume that a method is more accurate simply because it aligns closely with a real-world formula. However, this formula is often itself an approximation, and at times, a different approach may yield results that are closer to the desired outcome even if it doesn't seem physically accurate.

One key aspect to consider is the balance between quality and performance. Game developers often face the challenge of creating visuals that look good while still running smoothly on a variety of hardware. This means that some methods are chosen not because they are the most accurate, but because they strike a good balance between visual fidelity and computational efficiency.

To have a sneak peek on what awaits in this course regarding real-time rendering for video games, check out this video: [How do Video Game Graphics Work?](https://www.youtube.com/watch?v=C8YtdC8mxTU).

## Conclusion

Throughout this course, you will explore **one** approach to creating a rendering pipeline. However, it is essential to recognize that there are countless ways to arrange graphical techniques, and no single solution is universally optimal. The best architecture depends on various factors, including the type of rendering you aim to achieve, the nature of the content, the number and complexity of lights, the geometric complexity of your scenes, etc.

Moreover, the end-users of your renderer are also important, the skill level of the users must be considered when parameterizing algorithms. While a more complex parameterization may offer greater power and flexibility, it also requires a deeper understanding of how the algorithm operates.

In each module of this course, we will provide external resources and references to help you deepen your understanding of specific topics. While our primary focus will be on simpler and foundational approaches that are easy to grasp, these concepts will equip you with the knowledge and skills needed to learn and implement more complex techniques in the future.
