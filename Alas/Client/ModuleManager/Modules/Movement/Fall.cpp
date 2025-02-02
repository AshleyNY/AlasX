#include "Fall.h"
#include "../../../../Utils/BlockUtils.h"

Fall::Fall() : Module("Fall", "Step but reverse", Category::MOVEMENT) {
}

void Fall::onNormalTick(Actor* actor) {
	LocalPlayer* localPlayer = (LocalPlayer*)actor;
	Vec3<float> actorPos = localPlayer->getPosition()->floor(); actorPos.y -= 1.f;
	if (BlockUtils::getBlockName(actorPos) != "air") return;
	if (localPlayer->isOnGround() && !mc.isKeyDown(VK_SPACE)) localPlayer->stateVectorComponent->velocity.y = -1;
}