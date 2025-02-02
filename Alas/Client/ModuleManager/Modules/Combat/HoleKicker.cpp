#include "HoleKicker.h"
#include "../../../../Utils/BlockUtils.h"
#include "../../../../Utils/MoveUtils.h"

HoleKicker::HoleKicker() : Module("HoleKicker", "Kick people off their holes using pistonz, USE REDSTONE BLOCK", Category::COMBAT) {
	addBoolCheck("Faceplace Only", "Only faceplace", &facePlaceOnly);
	addBoolCheck("Self Test", "Test on uself", &selfTest);
	addBoolCheck("Airplace", "Place on air", &airplace);
	addBoolCheck("Render", "Render it!", &renderHoleKick);
	addSlider<int>("Piston Delay", "Delay for placing piston", ValueType::INT_T, &pistonDelay, 0, 20);
	addSlider<int>("Redstone Delay", "Delay for placing redstone", ValueType::INT_T, &redstoneDelay, 0, 20);
	addSlider<float>("Target Range", "NULL", ValueType::FLOAT_T, &targetRange, 0.f, 20.f);
}

void HoleKicker::onEnable() {
	targetList.clear();
	entityList.clear();
	redstonePos.clear();
	placeList.clear();
}

void HoleKicker::onDisable() {
	targetList.clear();
	entityList.clear();
	redstonePos.clear();
	placeList.clear();
}
bool HoleKicker::compareTarget(Actor* a1, Actor* a2) {
	Vec3<float> localPlayerPos = mc.getLocalPlayer()->getHumanPos();
	return a1->getHumanPos().dist(localPlayerPos) < a2->getHumanPos().dist(localPlayerPos);
}

bool HoleKicker::isPlaceValid(const Vec3<int>& pos, Actor* actor) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	Vec3<float> intersectPos = Vec3<float>(pos.x, pos.y, pos.z); intersectPos.y += 1.f;
	if (!BlockUtils::isReplaceable(pos) && ((BlockUtils::getBlockId(pos) != 33 || BlockUtils::getBlockId(pos) != 29) && BlockUtils::getBlockId(pos) != 152)) return false;
	if (localPlayer->getHumanPos().dist(pos) > 6) return false;
	if (actor->getAABB()->intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	if (entityList.empty()) return false;
	for (Actor* currentEnt : entityList) {
		if (!TargetUtils::isTargetValid(currentEnt, true)) continue;
		AABB targetAABB = *currentEnt->getAABB();
		if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	}
	return true;
}

void HoleKicker::generatePlacement(Actor* actor) {
	std::vector<Vec3<int>> validPos = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}, {0, 1, 0}, {0, -1, 0} };
	for (auto& valid : validPos) {
		for (int y = 0; y <= 1; y++) {
			if (facePlaceOnly) y = 1;
			Vec3<float> targetPos = actor->getPosition()->floor(); targetPos.y -= 1.f;
			Vec3<int> blockPos = targetPos.toInt().add(valid.x, y, valid.z);
			if (isPlaceValid(blockPos, actor)) {
				placeList.push_back(blockPos);
				if (!placeList.empty()) {
					Vec3<float> pos = Vec3<float>(targetPos.x - placeList[0].x, 0.f, targetPos.z - placeList[0].z);
					if (pos == Vec3<float>(1, 0, 0)) realYaw = 90;
					if (pos == Vec3<float>(-1, 0, 0)) realYaw = -90;
					if (pos == Vec3<float>(0, 0, 1)) realYaw = 180;
					if (pos == Vec3<float>(0, 0, -1)) realYaw = 0;
					Vec3<int> redPos = placeList[0].add(valid);
					if (isPlaceValid(redPos, actor)) redstonePos.push_back(redPos);
				}
			}
		}
	}
}

int HoleKicker::getItem(int itemId) {
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		if (stack->isValid() && stack != nullptr) {
			if (itemId != 0 && stack->getItemPtr()->itemId == itemId) return i;
			if (itemId == 0 && (stack->getItemPtr()->itemId == 33 || stack->getItemPtr()->itemId == 29)) return i;
		}
	} return -1;
}

void HoleKicker::onNormalTick(Actor* lp) {
	LocalPlayer* localPlayer = (LocalPlayer*)lp;
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	GameMode* gm = mc.getGameMode();
	targetList.clear();
	entityList.clear();
	redstonePos.clear();
	placeList.clear();
	for (Actor* actor : localPlayer->getLevel()->getRuntimeActorList()) {
		if (actor == nullptr) continue;
		entityList.push_back(actor);
		if (!TargetUtils::isTargetValid(actor)) continue;
		if (actor->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > 6.f) continue;
		targetList.push_back(actor);
	}
	if (selfTest) targetList.push_back(localPlayer);
	if (!targetList.empty()) {
		std::sort(targetList.begin(), targetList.end(), compareTarget);
		int oldSlot = supplies->selectedSlot;
		if (!MoveUtils::isMoving()) {
			mc.getLocalPlayer()->rotationComponent->rotation.y = realYaw;
		}
		for (auto& target : targetList) {
			generatePlacement(target);
			if (!placeList.empty() && !redstonePos.empty()) {
				if (pistonDelayIncr >= pistonDelay) {
					supplies->selectedSlot = getItem(0);
					MobEquipmentPacket piston(localPlayer->getRuntimeID(), inv->getItemStack(getItem(0)), getItem(0), getItem(0));
					mc.getLocalPlayer()->sendNetworkPacket(piston);
					mc.getClientInstance()->loopbackPacketSender->send(&piston);
					if (inv->getItemStack(supplies->selectedSlot)->isValid()) {
						if (inv->getItemStack(supplies->selectedSlot)->getItemPtr()->itemId == 33 || inv->getItemStack(supplies->selectedSlot)->getItemPtr()->itemId == 29) {
							BlockUtils::buildBlock(placeList[0], airplace);
						}
					}
					pistonDelayIncr = 0;
				}
				else pistonDelayIncr++;
				if (redstoneDelayIncr >= redstoneDelay) {
					supplies->selectedSlot = getItem(152);
					MobEquipmentPacket redstone(localPlayer->getRuntimeID(), inv->getItemStack(getItem(152)), getItem(152), getItem(152));
					mc.getLocalPlayer()->sendNetworkPacket(redstone);
					mc.getClientInstance()->loopbackPacketSender->send(&redstone);
					if (inv->getItemStack(supplies->selectedSlot)->isValid()) {
						if (inv->getItemStack(supplies->selectedSlot)->getItemPtr()->itemId == 152) {
							BlockUtils::buildBlock(redstonePos[0], airplace);
						}
					}
					redstoneDelayIncr = 0;
				}
				else redstoneDelayIncr++;
			}
		}
		supplies->selectedSlot = oldSlot;
		MobEquipmentPacket old(localPlayer->getRuntimeID(), inv->getItemStack(oldSlot), oldSlot, oldSlot);
		mc.getLocalPlayer()->sendNetworkPacket(old);
		mc.getClientInstance()->loopbackPacketSender->send(&old);
	}
}

void HoleKicker::onSendPacket(Packet* packet, bool& shouldCancel) {
}

void HoleKicker::onRender(MinecraftUIRenderContext* renderCtx) {
	if (placeList.empty()) return;
	if (redstonePos.empty()) return;
	if (!renderHoleKick) return;
	RenderUtils::drawBox(placeList[0].toFloat(), UIColor(0, 255, 0, 50), UIColor(0, 255, 0, 70), 0.3, true, true);
	RenderUtils::drawBox(redstonePos[0].toFloat(), UIColor(255, 0, 0, 50), UIColor(255, 0, 0, 70), 0.3, true, true);
}