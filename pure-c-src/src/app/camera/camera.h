#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/struct/vec3.h>
#include <cglm/struct/mat4.h>
#include <cglm/struct/cam.h>
#include <cglm/util.h>


typedef enum 
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
} CameraMovement;


typedef struct
{
//  camera Attributes
    vec3s position;
    vec3s front;
    vec3s up;
    vec3s right;
    vec3s worldUp;

//  euler Angles
    float yaw;
    float pitch;

//  camera options
    float movementSpeed;
    float mouseSensitivity;
} Camera;


void Camera_init(Camera* camera);
void Camera_processKeyboard(Camera* camera, CameraMovement direction, float deltaTime);
void Camera_processMouseMovement(Camera* camera, float xoffset, float yoffset);
mat4s Camera_getViewMatrix(Camera* camera);


#endif // !CAMERA_H