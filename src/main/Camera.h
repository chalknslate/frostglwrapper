#ifndef CAMERA_H_
#define CAMERA_H_

#include "Vec.h"
#include "Matrix4.h"
#include "Shader.h"
#include "math.h"
float rad(float degree) {
    return degree * 0.017453292519943295f; // π/180
}


struct Camera {
    struct Vec3 position;
    struct Vec3 rotation;   // pitch, yaw, roll
    // add these to your Camera struct
    struct Vec3 target;        // the player position or point to follow
    float distanceFromTarget;  // distance behind the player
    float heightOffset;        // height above the player
    float angleAroundTarget;   // how much to orbit around
    float distanceToPlayer;
    float angleAroundPlayer;

    float speed;
    float sensitivity;

    void (*moveForward)(struct Camera* this, float amount);
    void (*moveRight)(struct Camera* this, float amount);
    void (*moveUp)(struct Camera* this, float amount);
    void (*rotate)(struct Camera* this, float pitch, float yaw);

    void (*apply)(struct Camera* this, struct Program* prog);
};

static void Camera_moveForward(struct Camera* this, float amount) {
    float x = (float) sin(rad(this->rotation.getY(&this->rotation))) * amount;
    float z = (float) cos(rad(this->rotation.getY(&this->rotation))) * amount;
    this->position.x+=(0-x);
    this->position.z+=(0-z);
}


static void Camera_moveRight(struct Camera* this, float amount) {
    this->position.x += cosf(rad(this->rotation.y)) * amount;
    this->position.z += sinf(rad(this->rotation.y)) * amount;
}

static void Camera_moveUp(struct Camera* this, float amount) {
    this->position.y += amount;
}

static void Camera_rotate(struct Camera* this, float pitch, float yaw) {
    this->rotation.x += pitch;
    this->rotation.y += yaw;
}

static void Camera_apply(struct Camera* this, struct Program* prog) {
    struct Mat4 view = Mat4.new();
    view.view(&view,
              this->position.x, this->position.y, this->position.z,
              this->rotation.x, this->rotation.y, this->rotation.z);

    struct Uniform uview = Uniform.new(GL_MAT4, prog, "view");
    uview.ld(&uview, (void*)&view);
}
// Camera_updateThirdPerson
void Camera_updateThirdPerson(struct Camera* cam, struct Vec3* playerPos) {
    float horizontalDistance = cam->distanceFromTarget * cos(rad(cam->rotation.x));
    float verticalDistance   = cam->distanceFromTarget * sin(rad(cam->rotation.x));
    float theta = rad(cam->angleAroundTarget);

    cam->position.x = playerPos->x - sin(theta) * horizontalDistance;
    cam->position.z = playerPos->z - cos(theta) * horizontalDistance;
    cam->position.y = playerPos->y + verticalDistance + cam->heightOffset;

    cam->rotation.y = cam->angleAroundTarget;
}



inline static struct Camera Camera_new() {
    struct Camera cam;
    cam.position = Vec3.new(0,0,3);
    cam.rotation = Vec3.new(0,0,0);
    cam.speed = 0.05f;
    cam.sensitivity = 0.1f;

    cam.moveForward = &Camera_moveForward;
    cam.moveRight   = &Camera_moveRight;
    cam.moveUp      = &Camera_moveUp;
    cam.rotate      = &Camera_rotate;
    cam.apply       = &Camera_apply;

    return cam;
}
static const struct {
    struct Camera (*new)();
} Camera = { .new = &Camera_new, };

#endif
