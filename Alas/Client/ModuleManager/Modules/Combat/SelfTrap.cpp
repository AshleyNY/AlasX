#include "SelfTrap.h"
#include "../../../../Utils/BlockUtils.h"

SelfTrap::SelfTrap() : Module("SelfTrap", "GO TRAP YOURSELF NOW!", Category::COMBAT) {
	addBoolCheck("Smart", "Only self trap when target near", &smart);
	addSlider<float>("Target Range", "If a players distance is greater than this then we wont consider them a potential target", ValueType::FLOAT_T, &targetRange, 1.f, 12.f);
	addEnumSetting("Mode", "Type of mode", { "Normal", "Minimal" }, &mode);
}

void SelfTrap::onNormalTick(Actor* actor) {
	LocalPlayer* localPlayer = (LocalPlayer*)actor;
	targetList.clear();
	if (smart) {
		for (Actor* target : localPlayer->getLevel()->getRuntimeActorList()) {
			if (!TargetUtils::isTargetValid(target)) continue;
			if (target->getEyePos().dist(localPlayer->getEyePos()) > targetRange) continue;
			targetList.push_back(target);
		}
		if (targetList.empty()) return;
	}
	PlayerInventory* supplies = localPlayer->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	Vec3<float> actorPos = localPlayer->getPosition()->floor(); actorPos.y -= 1.f;
	std::vector<Vec3<int>> generalPlacement = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };
	int oldSlot = supplies->selectedSlot;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		if (stack->isValid() && stack != nullptr && stack->getItemPtr()->itemId == 49) {
			MobEquipmentPacket obby(localPlayer->getRuntimeID(), inv->getItemStack(i), i, i);
			mc.getLocalPlayer()->sendNetworkPacket(obby);
			mc.getClientInstance()->loopbackPacketSender->send(&obby);
		}
	}
	for (auto& general : generalPlacement) {
		for (int i = -1; i < 3; i++) {
			if (mode == 0) {
				Vec3<float> pos = actorPos.add(general.x, i, general.z);
				BlockUtils::buildBlock(pos);
			}
			if (mode == 1) BlockUtils::buildBlock(actorPos.add(1, i, 0));
			BlockUtils::buildBlock(actorPos.add(0, 2, 0));
		}
	}
	MobEquipmentPacket switchBack(localPlayer->getRuntimeID(), inv->getItemStack(oldSlot), oldSlot, oldSlot);
	mc.getLocalPlayer()->sendNetworkPacket(switchBack);
	mc.getClientInstance()->loopbackPacketSender->send(&switchBack);
}