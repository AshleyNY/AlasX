#include "Fly.h"
#include "../../../../Utils/MoveUtils.h"

Fly::Fly() : Module("Fly", "Fly around like a bird", Category::MOVEMENT) {
    addSlider<float>("X-Speed", "NULL", ValueType::FLOAT_T, &Speed, 0.f, 20.f);
    addSlider<float>("Y-Speed", "NULL", ValueType::FLOAT_T, &Speedy, 0.f, 20.f);
    addSlider<float>("Glide", "Amount of gliding, the more the faster we glide", ValueType::FLOAT_T, &glideValue, -0.15f, 0.00f);
}

void Fly::onNormalTick(Actor* actor) {
    LocalPlayer* localPlayer = mc.getLocalPlayer();
    MoveUtils::setSpeed(Speed);
    if (mc.isKeyDown(VK_SPACE)) localPlayer->stateVectorComponent->velocity.y = Speedy / 10.0f;
    else if (mc.isKeyDown(VK_SHIFT)) localPlayer->stateVectorComponent->velocity.y = -Speedy / 10.0f;
    else localPlayer->stateVectorComponent->velocity.y = glideValue;
}
