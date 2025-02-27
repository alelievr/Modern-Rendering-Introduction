#pragma once

#include <glm/glm.hpp>

// https://iquilezles.org/articles/frustumcorrect/
struct Frustum
{
    // The data of the 6 planes of the frustum
    glm::vec3 normal0;
    float dist0;
    glm::vec3 normal1;
    float dist1;
    glm::vec3 normal2;
    float dist2;
    glm::vec3 normal3;
    float dist3;
    glm::vec3 normal4;
    float dist4;
    glm::vec3 normal5;
    float dist5;

    // The data of the 8 corners of the frustum
    glm::vec4 corner0;
    glm::vec4 corner1;
    glm::vec4 corner2;
    glm::vec4 corner3;
    glm::vec4 corner4;
    glm::vec4 corner5;
    glm::vec4 corner6;
    glm::vec4 corner7;
};
