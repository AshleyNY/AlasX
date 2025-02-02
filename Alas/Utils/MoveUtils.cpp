#include "MoveUtils.h"

Vec2<float> MoveUtils::getMotion(float speed) {
    float playerYaw =mc.getLocalPlayer()->rotationComponent->Get().y;
    bool w = mc.isKeyDown('W');
    bool a = mc.isKeyDown('A');
    bool s = mc.isKeyDown('S');
    bool d = mc.isKeyDown('D');
    if (w) {
        if (!a && !d) playerYaw += 90.0f;
        if (a) playerYaw += 45.0f;
        else if (d) playerYaw += 135.0f;
    }
    else if (s) {
        if (!a && !d) playerYaw -= 90.0f;
        if (a) playerYaw -= 45.0f;
        else if (d) playerYaw -= 135.0f;
    }
    else {
        if (!a && d) playerYaw += 180.0f;
    }
    float calcYaw = playerYaw * 0.01745329251f; //0.01745329251f = PI / 180
    return Vec2<float>(cos(calcYaw) * speed / 10.0f, sin(calcYaw) * speed / 10.0f);
}

void MoveUtils::setSpeed(float speed) {
    Vec2<float> motion = getMotion(speed);
    if (!isMoving(false)) {
        motion.x = 0.f;
        motion.y = 0.f;
    }
    mc.getLocalPlayer()->stateVectorComponent->velocity.x = motion.x;
   mc.getLocalPlayer()->stateVectorComponent->velocity.z = motion.y;
}

bool MoveUtils::isMoving(bool jumpCheck) {
    if (!mc.canUseMoveKeys()) return false;
    if (mc.isKeyDown('W') || mc.isKeyDown('A') || mc.isKeyDown('S') || mc.isKeyDown('D')) return true;
    if (jumpCheck && mc.isKeyDown(VK_SPACE)) return true;
    return false;
}