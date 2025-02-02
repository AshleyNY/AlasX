#include "PistonAura.h"
#include "../../../Client.h"
#include "../../../../Utils/BlockUtils.h"
#include "../../../../Utils/MoveUtils.h"

PistonAura::PistonAura() : Module("PistonAura", "Pushes crystals using pistons into them and blow it up", Category::COMBAT) {
	addBoolCheck("Java", "Places crystals like its java", &java);
	addBoolCheck("Airplace", "Places on air", &airplace);
	addBoolCheck("EatStop", "stop on eat", &eatStop);
	addEnumSetting("Rotation", "NULL", { "Normal", "Packet", "Both" }, &rots);
	addSlider<int>("Max Height", "Maximum height of the pistonaura", ValueType::INT_T, &maxHeight, 1, 5);
	addSlider<int>("Piston Delay", "Delay of placing pistons", ValueType::INT_T, &pistonDelay, 0, 20);
	addSlider<int>("Redstone Delay", "Delay of placing pistons", ValueType::INT_T, &redstoneDelay, 0, 20);
	addSlider<int>("Crystal Delay", "Delay of placing pistons", ValueType::INT_T, &crystalDelay, 0, 20);
	addSlider<int>("Attack Delay", "Delay of placing pistons", ValueType::INT_T, &attackDelay, 0, 20);
	addBoolCheck("Self", "self test", &self);
}

int PistonAura::getPiston() {
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		if (stack->isValid() && stack != nullptr) {
			if (stack->getItemPtr()->itemId == 33 || stack->getItemPtr()->itemId == 29) return i;
		}
	} return -1;
}

int PistonAura::getItem(int id) {
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		if (stack->isValid() && stack != nullptr) {
			if (stack->getItemPtr()->itemId == id) return i;
		}
	} return -1;
}

bool PistonAura::isPistonValid(Vec3<int> place) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	Vec3<float> intersectPos = place.toFloat(); intersectPos.y += 1.f;
	if (BlockUtils::getBlockId(place) == 33 || BlockUtils::getBlockId(place) == 29) return true;
	if (!BlockUtils::isReplaceable(place) && BlockUtils::getBlockId(place) != 33 && BlockUtils::getBlockId(place) != 29) return false;
	if (!BlockUtils::canBuildBlock(place) && !airplace) return false;
	for (Actor* currentEnt : entityList) {
		if (!TargetUtils::isTargetValid(currentEnt, true) || currentEnt->getEntityTypeId() == 71) continue;
		AABB targetAABB = *currentEnt->getAABB();
		if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	}
	return true;
}

bool PistonAura::isRedstoneValid(Vec3<int> place) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	Vec3<float> intersectPos = place.toFloat(); intersectPos.y += 1.f;
	if (BlockUtils::getBlockId(place) == 152) return true;
	if (!BlockUtils::isReplaceable(place) && BlockUtils::getBlockId(place) != 152) return false;
	if (!BlockUtils::canBuildBlock(place) && !airplace) return false;
	for (Actor* currentEnt : entityList) {
		if (!TargetUtils::isTargetValid(currentEnt, true) || currentEnt->getEntityTypeId() == 71) continue;
		AABB targetAABB = *currentEnt->getAABB();
		if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	}
	return true;
}

bool PistonAura::isCrystalValid(Vec3<int> placePos) {
	Vec3<float> intersectPos = placePos.toFloat(); intersectPos.y += 1;
	if (!(BlockUtils::getBlockId(placePos) == 49 || BlockUtils::getBlockId(placePos) == 7)) return false;
	for (Actor* entity : entityList) {
		if (entity->getEntityTypeId() != 71) continue;
		if (BlockUtils::getBlockId(placePos.add(0, 1, 0)) == 34 || BlockUtils::getBlockId(placePos.add(0, 1, 0)) == 472) {
			AABB targetAABB = *entity->getAABB();
			if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f).add(0.f, java ? 0.5f : 0.f, 0.f)))) return true; // piston has pushed crystal
		}
	}
	if (BlockUtils::getBlockName(placePos.add(0, 1, 0)) != "air") return false;
	if (!java && BlockUtils::getBlockName(placePos.add(0, 2, 0)) != "air") return false;
	for (Actor* currentEnt : entityList) {
		if (currentEnt->getEntityTypeId() == 71) continue;
		AABB targetAABB = *currentEnt->getAABB();
		if (currentEnt->getEntityTypeId() == 319) {
			if (!java) { // Nukkit hitbox are way fatter than java so
				targetAABB.lower = targetAABB.lower.sub(Vec3<float>(0.1f, 0.f, 0.1f));
				targetAABB.upper = targetAABB.upper.add(0.1f, 0.f, 0.1f);
			}
		} if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f).add(0.f, java ? 0.5f : 0.f, 0.f)))) return false;
	}
	return true;
}
bool PistonAura::isGoodPosition(Vec3<int> pistonPos, Vec3<int> redstonePos, Vec3<int> crystalPos) {
	//Debugger* debugMod = (Debugger*)client->moduleMgr->getModule("Debugger");
	if (!isCrystalValid(crystalPos)) {
		
		return false;
	}
	if (!isPistonValid(pistonPos)) {
		
		return false;
	}
	if (!isRedstoneValid(redstonePos)) {
		return false;
	}
	return true;
}

void PistonAura::generatePlacement(Actor* actor) { // LOL this reminds me of zodiac bedaura xDDDDDDDDD
	Vec3<int> actorPos = actor->getPosition()->floor().toInt(); actorPos.y -= 1.f;
	if (isGoodPosition(actorPos.add(2, 1, 0), actorPos.add(3, 1, 0), actorPos.add(1, 0, 0))) {
		realYaw = -90;
		pistonList.push_back(actorPos.add(2, 1, 0));
		redstoneList.push_back(actorPos.add(3, 1, 0));
		crystalList.push_back(actorPos.add(1, 0, 0));
	}
	else if (isGoodPosition(actorPos.add(-2, 1, 0), actorPos.add(-3, 1, 0), actorPos.add(-1, 0, 0))) {
		realYaw = 90;
		pistonList.push_back(actorPos.add(-2, 1, 0));
		redstoneList.push_back(actorPos.add(-3, 1, 0));
		crystalList.push_back(actorPos.add(-1, 0, 0));
	}
	else if (isGoodPosition(actorPos.add(0, 1, 2), actorPos.add(0, 1, 3), actorPos.add(0, 0, 1))) {
		realYaw = 0;
		pistonList.push_back(actorPos.add(0, 1, 2));
		redstoneList.push_back(actorPos.add(0, 1, 3));
		crystalList.push_back(actorPos.add(0, 0, 1));
	}
	else if (isGoodPosition(actorPos.add(0, 1, -2), actorPos.add(0, 1, -3), actorPos.add(0, 0, -1))) {
		realYaw = 180;
		pistonList.push_back(actorPos.add(0, 1, -2));
		redstoneList.push_back(actorPos.add(0, 1, -3));
		crystalList.push_back(actorPos.add(0, 0, -1));
	}
	if (isGoodPosition(actorPos.add(2, 1, 0), actorPos.add(3, 1, 0), actorPos.add(1, 0, 0))) {
		realYaw = -90;
		pistonList.push_back(actorPos.add(2, 1, 0));
		redstoneList.push_back(actorPos.add(3, 1, 0));
		crystalList.push_back(actorPos.add(1, 0, 0));
	}
	else if (isGoodPosition(actorPos.add(-2, 2, 0), actorPos.add(-3, 2, 0), actorPos.add(-1, 1, 0))) {
		realYaw = 90;
		pistonList.push_back(actorPos.add(-2, 2, 0));
		redstoneList.push_back(actorPos.add(-3, 2, 0));
		crystalList.push_back(actorPos.add(-1, 1, 0));
	}
	else if (isGoodPosition(actorPos.add(0, 2, 2), actorPos.add(0, 2, 3), actorPos.add(0, 1, 1))) {
		realYaw = 0;
		pistonList.push_back(actorPos.add(0, 2, 2));
		redstoneList.push_back(actorPos.add(0, 2, 3));
		crystalList.push_back(actorPos.add(0, 1, 1));
	}
	else if (isGoodPosition(actorPos.add(0, 2, -2), actorPos.add(0, 2, -3), actorPos.add(0, 1, -1))) {
		realYaw = 180;
		pistonList.push_back(actorPos.add(0, 2, -2));
		redstoneList.push_back(actorPos.add(0, 2, -3));
		crystalList.push_back(actorPos.add(0, 1, -1));
	}
}

void PistonAura::placePiston(GameMode* gm) {
	if (pistonList.empty()) return;
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	MobEquipmentPacket piston(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getPiston()), getPiston(), getPiston());
	if (ipistonDelay == -999) ipistonDelay = 0;
	if (ipistonDelay >= pistonDelay) { // 50ms = 1 tick, 1x20 = 20 = 20ticks
		plrInv->selectedSlot = getPiston();
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&piston);
		BlockUtils::buildBlock(pistonList[0], airplace);
		ipistonDelay = 0;
	}
	else ipistonDelay++;
}

void PistonAura::placeRedstone(GameMode* gm) {
	if (redstoneList.empty()) return;
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	MobEquipmentPacket redstone(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(152)), getItem(152), getItem(152));
	if (iredstoneDelay == -999) iredstoneDelay = 0;
	if (iredstoneDelay >= redstoneDelay) {
		plrInv->selectedSlot = getItem(152);
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&redstone);
		for (Actor* crystal : entityList) {
			if (crystal == nullptr) continue;
			if (crystal->getEntityTypeId() != 71) continue;
			if (crystal->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > 6.f) continue;
			Vec3<int> entityPos = crystal->getPosition()->floor().toInt();
			if (entityPos == crystalList[0].add(0, 1, 0)) {
				BlockUtils::buildBlock(redstoneList[0], airplace);
				break;
			}
		}
		iredstoneDelay = 0;
	}
	else iredstoneDelay++;
}

void PistonAura::placeCrystal(GameMode* gm) {
	if (crystalList.empty()) return;
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	Disabler* disabler = (Disabler*)client->moduleMgr->getModule("Disabler");
	MobEquipmentPacket crystal(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(720)), getItem(720), getItem(720));
	if (icrystalDelay == -999) icrystalDelay = 0;
	if (icrystalDelay >= crystalDelay) {
		plrInv->selectedSlot = getItem(720);
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&crystal);
		if (disabler->isEnabled()) mc.getLocalPlayer()->getLevel()->setHitResult(HitResultType::AIR);
		if (BlockUtils::getBlockId(pistonList[0]) == 29 || BlockUtils::getBlockId(pistonList[0]) == 33) gm->buildBlock(crystalList[0], 0, false);
		icrystalDelay = 0;
	}
	else icrystalDelay++;
}

void PistonAura::onNormalTick(Actor* localPlayer) {
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	GameMode* gm = mc.getGameMode();
	pistonList.clear();
	entityList.clear();
	redstoneList.clear();
	crystalList.clear();
	int oldSlot = plrInv->selectedSlot;
	MobEquipmentPacket old(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(oldSlot), oldSlot, oldSlot);
	if (eatStop && mc.getLocalPlayer()->getItemUseDuration() > 0) return;
	for (Actor* target : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (target == nullptr) continue;
		entityList.push_back(target);
		if (!TargetUtils::isTargetValid(target)) continue;
		if (target->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > range) continue;
		targetList.push_back(target);
	}
	if (self) targetList.push_back(mc.getLocalPlayer());
	if (targetList.empty()) return;
	if (!targetList.empty()) {
		if (!MoveUtils::isMoving()) {
			if (rots == 0 || rots == 2) {
				mc.getLocalPlayer()->rotationComponent->rotation.y = realYaw;
				mc.getLocalPlayer()->rotationComponent->rotation.x = 0; // so piston dont face up or down LOL
			}
		}
		if (rots == 1) {
			PlayerAuthInputPacket authPkt;
			authPkt.rotation.y = realYaw;
			authPkt.rotation.x = 0;
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&authPkt);
		}
		generatePlacement(targetList[0]);
		placePiston(gm);
		placeCrystal(gm);
		placeRedstone(gm);
		if (entityList.empty()) return;
		if (iattackDelay == -999) iattackDelay = 0;
		if (iattackDelay >= attackDelay) {
			for (Actor* entity : entityList) {
				if (entity == nullptr) continue;
				if (entity->getEntityTypeId() != 71) continue;
				if (entity->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > 6.f) continue;
				if (BlockUtils::getBlockId(redstoneList[0]) == 152) gm->attack(entity);
			}
			iattackDelay = 0;
		}
		else iattackDelay++;
		plrInv->selectedSlot = oldSlot;
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&old);
	}
}

void PistonAura::onRender(MinecraftUIRenderContext* renderCtx) {
	if (!pistonList.empty()) RenderUtils::drawBox(pistonList[0].toFloat(), UIColor(0, 255, 0, 70), UIColor(0, 255, 0, 50), 0.3f, true, false);
	if (!redstoneList.empty()) RenderUtils::drawBox(redstoneList[0].toFloat(), UIColor(255, 0, 0, 70), UIColor(255, 0, 0, 50), 0.3f, true, false);
}

void PistonAura::onSendPacket(Packet* packet, bool& cancel) {
	if (rots == 1 || rots == 2) {
		if (packet->getId() == PacketID::PlayerAuthInput) {
			PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
			authPkt->rotation.y = realYaw;
			authPkt->rotation.x = 0;
		}
		if (packet->getId() == PacketID::MovePlayerPacket) {
			MovePlayerPacket* movePkt = (MovePlayerPacket*)packet;
			movePkt->rotation.y = realYaw;
			movePkt->rotation.x = 0;
		}
	}
}
void PistonAura::onEnable() {
	pistonList.clear();
	entityList.clear();
	redstoneList.clear();
	crystalList.clear();
}

void PistonAura::onDisable() {
	pistonList.clear();
	entityList.clear();
	redstoneList.clear();
	crystalList.clear();
}