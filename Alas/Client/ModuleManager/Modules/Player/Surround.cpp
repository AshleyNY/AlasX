#include "Surround.h"
#include "../../../Client.h"
#include "../../../../Utils/BlockUtils.h"

bool stopOnEat = false;
bool Placeabove = false;
Surround::Surround() : Module("Surround", "Place blocks around you.", Category::COMBAT) {
	addBoolCheck("AirPlace", "place on airing", &airplace);
	addBoolCheck("EatStop", "Stops when eating", &stopOnEat);
	addBoolCheck("Center", "Centers you", &center);
	addBoolCheck("Disable Complete", "Disables when surround is completed", &disableComplete);
	addEnumSetting("Switch", "NULL", { "None", "Spoof", "Silent" }, &switchMode);
	addBoolCheck("Jump Stop", "Disable surround when jumping", &jumpStop);
	addBoolCheck("Only OnGround", "Only work when player is on ground", &onlyOnGround);
	addBoolCheck("Above place", "Place blocks above your head", &Placeabove);
	addBoolCheck("Attack Crystal", "Attacks crystals so you can place", &attackCrystal);
}

void Surround::onEnable() {
	entityList.clear();
}

void Surround::onDisable() {
	entityList.clear();
}

int Surround::getItem(int id) {
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	int slot = plrInv->selectedSlot;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = inv->getItemStack(i);
		if (itemStack != nullptr) {
			if (itemStack->isValid()) {
				if (itemStack->getItemPtr() != nullptr) {
					if (itemStack->getItemPtr()->itemId == id) {
						return i;
					}
				}
			}
		}
	}
	return slot;
}

bool Surround::doWeHaveItem(int id) {
	auto* supplies = mc.getLocalPlayer()->getPlayerInventory();
	auto* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* current = inv->getItemStack(i);
		if (current->isValid() && current != nullptr && current->getItemPtr()->itemId == id) return true;
	}
	for (int i = 9; i > 0; i--) {
		ItemStack* current = inv->getItemStack(i);
		if (current->isValid() && current != nullptr && current->getItemPtr()->itemId == id) return true;
	}
	return false;
}
std::vector<Vec3<float>> sideBlocks = { {1, 0, 0}, {0, 0, 1}, {-1, 0, 0}, {0, 0, -1}, {0, -1, 0} };
void Surround::onNormalTick(Actor* actor) {
	GameMode* gm = mc.getGameMode();
	if (!gm) return;
	if (!mc.getLocalPlayer()) return;
	if (!mc.getLocalPlayer()->getLevel()) return;
	if (mc.getLocalPlayer()->getLevel()->getRuntimeActorList().empty()) return;
	entityList.clear();
	if (stopOnEat && mc.getLocalPlayer()->getItemUseDuration() > 0) return;
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	int oldSlot = plrInv->selectedSlot;
	for (Actor* entity : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (entity == nullptr) continue;
		if (entity->getEntityTypeId() == 71) continue;
		if (entity == mc.getLocalPlayer()) entityList.push_back(entity);
		if (!TargetUtils::isTargetValid(entity, true)) continue;
		if (entity->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > 6) continue;
		entityList.push_back(entity);
	}
	if (attackCrystal) {
		for (Actor* ent : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
			if (ent == nullptr) continue;
			if (ent->getEntityTypeId() != 71) continue;
			if (ent->getEyePos().dist(mc.getLocalPlayer()->getHumanPos()) > 3) continue;
			mc.getGameMode()->attack(ent);
		}
	}
	if (!entityList.empty()) {
		if (switchMode == 1) {
			if (doWeHaveItem(49)) plrInv->selectedSlot = getItem(49);
			else if (doWeHaveItem(130)) plrInv->selectedSlot = getItem(130);
		}
		else if (switchMode == 2) {
			if (doWeHaveItem(49)) {
				MobEquipmentPacket pk(mc.getLocalPlayer()->getRuntimeID(), plrInv->inventory->getItemStack(getItem(49)), getItem(49), getItem(49));
				mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
			}
			else if (doWeHaveItem(130)) {
				MobEquipmentPacket pk(mc.getLocalPlayer()->getRuntimeID(), plrInv->inventory->getItemStack(getItem(130)), getItem(130), getItem(130));
				mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
			}
		}
		Vec3<float> playerPos = mc.getLocalPlayer()->getPosition()->floor().sub(Vec3<float>(0.f, 1.f, 0.f));
		for (Actor* entity : entityList) {
			AABB playerAABB = *entity->getAABB();
			for (Vec3<float> check : sideBlocks) {
				Vec3<int> posToCheck = (playerPos.add(check)).toInt();
				AABB blockAABB{ posToCheck.toFloat(), posToCheck.add(1, 1, 1).toFloat() };
				if (playerAABB.intersects(blockAABB)) {
					if (BlockUtils::isReplaceable(posToCheck.add(check.toInt()))) {
						if (center) mc.getLocalPlayer()->setPos(mc.getLocalPlayer()->getPosition()->floor().add(0.5f, 0.75f, 0.5f));
						BlockUtils::buildBlock(posToCheck.add(check.toInt()), airplace);
						BlockUtils::buildBlock(posToCheck.add(check.toInt()).add(0, -1, 0), airplace);
					}
					for (int i : {-1, 1}) {
						for (int j : {-1, 1}) {
							Vec3<int> cornerPos = posToCheck.add(check.z * i, check.y, check.x * j);
							if (BlockUtils::isReplaceable(cornerPos)) {
								if (center) mc.getLocalPlayer()->setPos(mc.getLocalPlayer()->getPosition()->floor().add(0.5f, 0.75f, 0.5f));
								BlockUtils::buildBlock(cornerPos, airplace);
								BlockUtils::buildBlock(cornerPos.add(0, -1, 0), airplace);
							}
							AABB cornerAABB{ cornerPos.toFloat(), cornerPos.add(1, 1, 1).toFloat() };
							if (playerAABB.intersects(cornerAABB)) {
								if (center) mc.getLocalPlayer()->setPos(mc.getLocalPlayer()->getPosition()->floor().add(0.5f, 0.75f, 0.5f));
								Vec3<int> adjustedPos = cornerPos.add(check.z * i, 0, check.x * j);
								if (BlockUtils::isReplaceable(adjustedPos)) {
									BlockUtils::buildBlock(adjustedPos, airplace);
									BlockUtils::buildBlock(adjustedPos.add(0, -1, 0), airplace);
								}
								AABB adjustedAABB{ adjustedPos.toFloat(), adjustedPos.add(1, 1, 1).toFloat() };
								if (playerAABB.intersects(adjustedAABB)) {
									Vec3<int> moreAdjustedPos = adjustedPos.add(check.z * i, 0, check.x * j);
									if (BlockUtils::isReplaceable(moreAdjustedPos)) {
										BlockUtils::buildBlock(moreAdjustedPos, airplace);
										BlockUtils::buildBlock(moreAdjustedPos.add(0, -1, 0), airplace);
									}
								}
							}
						}
					}
				}
				else {
					if (BlockUtils::isReplaceable(posToCheck)) {
						if (center) mc.getLocalPlayer()->setPos(mc.getLocalPlayer()->getPosition()->floor().add(0.5f, 0.75f, 0.5f));
						BlockUtils::buildBlock(posToCheck, airplace);
						BlockUtils::buildBlock(posToCheck.add(0, -1, 0), airplace);
					}
				}
			}
			if (Placeabove) BlockUtils::buildBlock(Vec3<float>(playerPos.add(Vec3<float>(0.f, 2.f, 0.f))).toInt(), airplace);
		}
		if (switchMode == 1) plrInv->selectedSlot = oldSlot;
		else if (switchMode == 2) {
			MobEquipmentPacket switchBack(mc.getLocalPlayer()->getRuntimeID(), mc.getLocalPlayer()->getPlayerInventory()->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&switchBack);
		}
		if (disableComplete) this->setEnabled(false);
	}
}
