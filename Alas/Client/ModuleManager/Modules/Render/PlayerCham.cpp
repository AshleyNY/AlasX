#include "PlayerCham.h"

PlayerCham::PlayerCham() : Module("PlayerCham", "PlayerCham", Category::RENDER) {
	addColorPicker("Color", "NULL", &color);
	addColorPicker("LineColor", "NULL", &lineColor);
	//addBoolCheck("Mobs", "ESP Mobs", &mobs);
}

void PlayerCham::onRender(MinecraftUIRenderContext* ctx) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();

	if (localPlayer == nullptr) return;
	if (!mc.canUseMoveKeys()) return;
	Level* level = localPlayer->getLevel();
	if (level == nullptr) return;
	PlayerChamList.clear();

	for (Actor* ent : level->getRuntimeActorList()) {
		bool isValid = TargetUtils::isTargetValid(ent, false);
		if (isValid) PlayerChamList.push_back(ent);
	}

	for (Actor* ent : PlayerChamList) {
		switch (renderMode) {
		case 0: {
			//AABB mobAABB = *ent->getAABB();
			//RenderUtils::drawBox(mobAABB, color, lineColor, .3f, true, false);
			Vec3<float> Pos = ent->stateVectorComponent->pos.add(0, 0, 0);
			RenderUtils::drawBoxCustom(Pos, 0.25, 0.23f, 0.23f, color, color, 0.5f, true, true);
			RenderUtils::drawBoxCustom(Pos.add(0, -0.6f, 0), 0.2f, 0.37f, 0.26f, color, color, 0.5f, true, true);
			RenderUtils::drawBoxCustom(Pos.add(0, -0.5f, 0.39), 0.2f, 0.27f, 0.13f, color, color, 0.5f, true, true);
			RenderUtils::drawBoxCustom(Pos.add(0, -0.5f, -0.39), 0.2f, 0.27f, 0.13f, color, color, 0.5f, true, true);
			RenderUtils::drawBoxCustom(Pos.add(0, -1.27f, 0.13), 0.2f, 0.27f, 0.13f, color, color, 0.5f, true, true);
			RenderUtils::drawBoxCustom(Pos.add(0, -1.27f, -0.13), 0.2f, 0.27f, 0.13f, color, color, 0.5f, true, true);
			;			break;
		}
		}
	}
}
