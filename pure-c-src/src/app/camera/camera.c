#include "app/camera/camera.h"


// Default camera values
static const float YAW         = -90.0f;
static const float PITCH       =  0.0f;
static const float SPEED       =  2.5f;
static const float SENSITIVITY =  0.1f;


static void update_camera_vectors(Camera* camera)
{
    // calculate the new Front vector
    vec3s front;
    front.x = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    front.y = sin(glm_rad(camera->pitch));
    front.z = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    camera->front = glms_vec3_normalize(front);
    
    // also re-calculate the Right and Up vector
    camera->right = glms_vec3_normalize(glms_cross(camera->front, camera->worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    camera->up    = glms_cross(camera->right, camera->front);
}



void Camera_init(Camera* camera)
{
    camera->position = (vec3s){0.f, 0.f,  3.f};
    camera->front    = (vec3s){0.f, 0.f, -1.f};
    camera->up       = (vec3s){0.f, 1.f,  0.f};
    camera->right    = (vec3s){0.f, 0.f,  0.f};
    camera->worldUp  = (vec3s){0.f, 1.f,  0.f};

    camera->yaw   = YAW;
    camera->pitch = PITCH;

    camera->movementSpeed    = SPEED; 
    camera->mouseSensitivity = SENSITIVITY; 

    update_camera_vectors(camera);
}


void Camera_processKeyboard(Camera* camera, CameraMovement direction, float deltaTime)
{
    float velocity = camera->movementSpeed * deltaTime;

    switch (direction)
    {
        case FORWARD:  glm_vec3_muladds(camera->front.raw, velocity, camera->position.raw); break;
        case BACKWARD: glm_vec3_mulsubs(camera->front.raw, velocity, camera->position.raw); break;
        case LEFT:     glm_vec3_mulsubs(camera->right.raw, velocity, camera->position.raw); break;
        case RIGHT:    glm_vec3_muladds(camera->right.raw, velocity, camera->position.raw); break;
        
        default:
            break;
    }
}


void Camera_processMouseMovement(Camera* camera, float xoffset, float yoffset)
{
    xoffset *= camera->mouseSensitivity;
    yoffset *= camera->mouseSensitivity;

    camera->yaw   += xoffset;
    camera->pitch += yoffset;

//  make sure that when pitch is out of bounds, screen doesn't get flipped
    if (camera->pitch > 89.f)
        camera->pitch = 89.f;

    if (camera->pitch < -89.f)
        camera->pitch = -89.f;

//  update Front, Right and Up Vectors using the updated Euler angles
    update_camera_vectors(camera);
}


mat4s Camera_getViewMatrix(Camera* camera)
{
    vec3s center = glms_vec3_add(camera->position, camera->front);

    return glms_lookat(camera->position, center, camera->up);
}