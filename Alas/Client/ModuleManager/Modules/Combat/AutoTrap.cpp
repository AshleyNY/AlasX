#include "AutoTrap.h"
#include "../../../../Utils/BlockUtils.h"

AutoTrap::AutoTrap() : Module("AutoTrap", "Trap people", Category::COMBAT) {
	addBoolCheck("Airplace", "Placing on air", &airplace);
	addBoolCheck("Packet Place", "Places using packets, this may look slower but it prevents ghost blocks", &packetPlace);
	addSlider<int>("Trap Delay", "NULL", ValueType::INT_T, &trapDelay, 0, 20);
	addSlider<float>("Range", "Trap range", ValueType::FLOAT_T, &range, 0.f, 12.f);
	addBoolCheck("SelfTest", "For devs", &selfTest);
	addEnumSetting("Mode", "Type of trap we usin", { "Normal", "Minimal" }, &trapMode);
	addEnumSetting("Swap", "NULL", { "None", "Regular", "Packet" }, &swapMode);
}

void AutoTrap::onEnable() {
	entityList.clear();
	targetList.clear();
}

void AutoTrap::onDisable() {
	entityList.clear();
	targetList.clear();
}

int AutoTrap::getItemSlot(int itemId) {
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		if (stack != nullptr && stack->isValid() && stack->getItemPtr()->itemId == itemId) return i;
	} return -1;
}

void AutoTrap::getPlacement() {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	GameMode* gm = mc.getGameMode();
	if (entityList.empty()) return;
	if (targetList.empty()) return;
	std::vector<Vec3<float>> sideBlocks = { {1, 0, 0}, {0, 0, 1}, {-1, 0, 0}, {0, 0, -1} };
	for (Actor* entity : entityList) {
		Vec3<float> playerPos = targetList[0]->getPosition()->floor().sub(Vec3<float>(0.f, 1.f, 0.f));
		AABB playerAABB = *entity->getAABB();
		for (Vec3<float> check : sideBlocks) {
			if (iTrapDelay >= trapDelay) {
				for (int i = -1; i < 3; i++) {
					Vec3<int> posToCheck = (playerPos.add(check)).toInt();
					AABB blockAABB{ posToCheck.toFloat(), posToCheck.add(1, 1, 1).toFloat() };
					if (playerAABB.intersects(blockAABB)) {
						if (BlockUtils::isReplaceable(posToCheck.add(0, i, 0).add(check.toInt()))) BlockUtils::buildBlock(posToCheck.add(0, i, 0).add(check.toInt()), airplace);
						for (int i : {-1, 1}) {
							for (int j : {-1, 1}) {
								Vec3<int> cornerPos = posToCheck.add(check.z * i, check.y, check.x * j);
								if (BlockUtils::isReplaceable(cornerPos.add(0, i, 0))) BlockUtils::buildBlock(cornerPos.add(0, i, 0), airplace);
								AABB cornerAABB{ cornerPos.toFloat(), cornerPos.add(1, 1, 1).toFloat() };
								if (playerAABB.intersects(cornerAABB)) {
									Vec3<int> adjustedPos = cornerPos.add(check.z * i, 0, check.x * j);
									if (BlockUtils::isReplaceable(adjustedPos.add(0, i, 0))) BlockUtils::buildBlock(adjustedPos.add(0, i, 0), airplace);
									AABB adjustedAABB{ adjustedPos.toFloat(), adjustedPos.add(1, 1, 1).toFloat() };
									if (playerAABB.intersects(adjustedAABB)) {
										Vec3<int> moreAdjustedPos = adjustedPos.add(check.z * i, 0, check.x * j);
										if (BlockUtils::isReplaceable(moreAdjustedPos.add(0, i, 0))) BlockUtils::buildBlock(moreAdjustedPos.add(0, i, 0), airplace);
									}
								}
							}
						}
					}
					else {
						if (BlockUtils::isReplaceable(posToCheck.add(0, i, 0))) BlockUtils::buildBlock(posToCheck.add(0, i, 0), airplace);
					}
				}
				iTrapDelay = 0;
			}
			else iTrapDelay++;
		}
	}
}

void AutoTrap::onNormalTick(Actor* actor) {
	LocalPlayer* localPlayer = (LocalPlayer*)actor;
	PlayerInventory* supplies = localPlayer->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	GameMode* gm = mc.getGameMode();
	entityList.clear();
	targetList.clear();
	int oldSlot = supplies->selectedSlot;
	for (Actor* entity : localPlayer->getLevel()->getRuntimeActorList()) {
		if (entity == nullptr) continue;
		if (entity == localPlayer) entityList.push_back(entity);
		if (TargetUtils::isTargetValid(entity, true)) entityList.push_back(entity);
		if (TargetUtils::isTargetValid(entity)) targetList.push_back(entity);
	}
	if (selfTest) targetList.push_back(localPlayer);
	if (swapMode == 1) supplies->selectedSlot = getItemSlot(49);
	if (swapMode == 2) {
		if (!packetPlace) supplies->selectedSlot = getItemSlot(49);
		MobEquipmentPacket obby(localPlayer->getRuntimeID(), inv->getItemStack(getItemSlot(49)), getItemSlot(49), getItemSlot(49));
		mc.getLocalPlayer()->sendNetworkPacket(obby);
		mc.getClientInstance()->loopbackPacketSender->send(&obby);
	}
	if (!targetList.empty()) {
		Vec3<float> actorPos = targetList[0]->getPosition()->floor(); actorPos.y -= 1.f;
		if (trapMode == 1) {
			for (int i = -1; i < 3; i++) {
				if (!airplace) {
					BlockUtils::buildBlock(actorPos.add(1, i, 0));
					BlockUtils::buildBlock(actorPos.add(0, 2, 0));
				}
				else {
					gm->buildBlock(actorPos.add(1, i, 0).toInt(), 0, false);
					gm->buildBlock(actorPos.add(0, 2, 0).toInt(), 0, false);
				}
			}
		}
		else {
			getPlacement();
			if (!airplace) BlockUtils::buildBlock(actorPos.add(0, 2, 0));
			else gm->buildBlock(actorPos.add(0, 2, 0).toInt(), 0, false);
		}
		if (swapMode == 2) {
			if (!packetPlace) supplies->selectedSlot = oldSlot;
			MobEquipmentPacket switchBack(localPlayer->getRuntimeID(), inv->getItemStack(oldSlot), oldSlot, oldSlot);
			mc.getLocalPlayer()->sendNetworkPacket(switchBack);
			mc.getClientInstance()->loopbackPacketSender->send(&switchBack);
		}
	}
}