#include "AutoCrystal.h"
#include "../Player/PacketMine.h"
#include "../../../Client.h"
#include <algorithm>
#include <vector>
#include <ranges>
#include <mutex>
#include <omp.h>
#include "../../../../Utils/Player/DamageUtils.h"
#include "../../../../Utils/Player/BlockUtils.h"

bool Damagerender = false;
AutoCrystal::AutoCrystal() : Module("AutoCrystal", "NULL", Category::COMBAT, 0x0) {
	addSlider<float>("Target Range", "NULL", ValueType::FLOAT_T, &targetRange, 5.f, 20.f);

	addBoolCheck("Auto Place", "NULL", &autoPlace);
	addSlider<float>("Place Range", "NULL", ValueType::FLOAT_T, &placeRange, 3.f, 10.f);
	addSlider<float>("Max Place Damage", "NULL", ValueType::FLOAT_T, &maxPlaceDame, 1.f, 20.f);
	addSlider<float>("Min Place Damage", "NULL", ValueType::FLOAT_T, &minPlaceDame, 1.f, 20.f);
	addSlider<int>("Multi Place", "NULL", ValueType::INT_T, &multiPlace, 1, 5);
	addSlider<int>("Place Delay", "NULL", ValueType::INT_T, &placeDelay, 0, 20);

	addBoolCheck("Auto Break", "NULL", &autoBreak);
	addSlider<float>("Break Range", "NULL", ValueType::FLOAT_T, &breakRange, 3.f, 10.f);
	addSlider<float>("Max Break Damage", "NULL", ValueType::FLOAT_T, &maxBreakDame, 1.f, 20.f);
	addSlider<float>("Min Break Damage", "NULL", ValueType::FLOAT_T, &minBreakDame, 1.f, 20.f);
	addSlider<int>("Break Delay", "NULL", ValueType::INT_T, &breakDelay, 0, 20);
	addBoolCheck("ID Predict", "NULL", &idPredict);
	// Extrapolation
	addBoolCheck("Extrapolation", "Gets their velocity(if the server sends it) and predicts their next pos and places near there", &extrapolation, 4);
	addSlider<float>("Intensity", "Amount of velocity you wanna multiply and predict idk", ValueType::FLOAT_T, &extrapolateAmount, 0.f, 20.f, 4);


	addSlider<int>("Packets", "NULL", ValueType::INT_T, &packets, 1, 30);
	addSlider<int>("Ticks", "NULL", ValueType::INT_T, &Ticks, 0, 100);
	addSlider<int>("Send Delay", "NULL", ValueType::INT_T, &sendDelay, 0, 20);
	addBoolCheck("Count crystal", "Crystal speed", &Crystalcounter);
	addBoolCheck("Render Damages", "NULL", &Damagerender);
	addBoolCheck("Testing", "For testing", &Mob);

}
std::string AutoCrystal::getModName() {

	return names;

}

bool AutoCrystal::sortCrystalByTargetDame(CrystalStruct a1, CrystalStruct a2) {
	return a1.TgDameTake > a2.TgDameTake;
}

bool AutoCrystal::sortEntityByDist(Actor* a1, Actor* a2) {
	auto localPlayerPos = mc.getLocalPlayer()->stateVectorComponent->pos;
	return (a1->stateVectorComponent->pos.dist(localPlayerPos) < a2->stateVectorComponent->pos.dist(localPlayerPos));
}

bool AutoCrystal::isHoldingCrystal() {
	auto* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	auto* inv = plrInv->inventory;
	auto slot = plrInv->selectedSlot;
	auto* itemStack = inv->getItemStack(slot);
	return (itemStack->isValid()) ? itemStack->getItemPtr()->getitemId() == 720 : false;
}
void AutoCrystal::cleardalist() {
	entityList.clear();
	targetList.clear();
	placeList.clear();
	breakList.clear();
}
int dam;
void AutoCrystal::onEnable() {
	cleardalist();
	placerot = false;
	breakrot = false;
	placing = false;
	breaking = false;
}

void AutoCrystal::onDisable() {
	cleardalist();
	placerot = false;
	breakrot = false;
	placing = false;
	breaking = false;
}


float CrystalStruct::getExplosionDamage(const Vec3<float>& crystalPos, Actor* target) {
	AutoCrystal* autoCrystal = (AutoCrystal*)client->moduleMgr->getModule("AutoCrystal");
	return DamageUtils::getExplosionDamage(crystalPos, target, autoCrystal->extrapolation ? autoCrystal->extrapolateAmount : 0.f, 0);
}

bool AutoCrystal::isPlaceValid(const Vec3<int>& placePos, Actor* target) {
	Vec3<float> intersectPos = Vec3<float>(placePos.x, placePos.y, placePos.z);
	intersectPos.y += 1;

	if (!(BlockUtils::getBlockId(placePos) == 49 || BlockUtils::getBlockId(placePos) == 7)) return false;
	if (mc.player()->dimension->blockSource->getBlock(Vec3<int>(placePos.x, placePos.y + 1, placePos.z))->blockLegacy->blockName != "air") return false;

	if (mc.player()->getHumanPos().dist(placePos) > placeRange) return false;
	if (target->getAABB()->intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;

	for (Actor* currentEnt : entityList) {
		if (currentEnt->getEntityTypeId() == 71) continue;
		AABB targetAABB = *currentEnt->getAABB();
		if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	}

	return true;
}

void AutoCrystal::generatePlacements(Actor* target) {
	const int radius = (int)multiPlace;
	for (auto x = -radius; x <= radius; x++) {
		for (auto y = -targetRange; y <= 3; y++) {
			for (auto z = -radius; z <= radius; z++) {
				Vec3<float> targetPos = target->getHumanPos();
				Vec3<int> blockPos = Vec3<int>(static_cast<int>(targetPos.x + (target->stateVectorComponent->velocity.x * extrapolateAmount)) + x,
					static_cast<int>(targetPos.y + (target->stateVectorComponent->velocity.y * extrapolateAmount)) + y,
					static_cast<int>(targetPos.z + (target->stateVectorComponent->velocity.z * extrapolateAmount)) + z); // Extra po lat ion
				if (isPlaceValid(blockPos, target)) {
					CrystalPlacement placement(blockPos, target);
					if (placement.LpDameTake < maxPlaceDame && placement.TgDameTake >= minPlaceDame) placeList.push_back(placement);
				}
			}
		}
	}
	std::sort(placeList.begin(), placeList.end(), AutoCrystal::sortCrystalByTargetDame);
}

void AutoCrystal::getCrystalActorList(Actor* target) {
	auto* localPlayer = mc.getLocalPlayer();
	std::vector<CrystalBreaker> localBreakList;

	for (auto* ent : entityList) {
		if (ent->getEntityTypeId() != 71 || ent->stateVectorComponent->pos.dist(localPlayer->stateVectorComponent->pos) > breakRange)
			continue;

		CrystalBreaker cBreaker(ent, target);
		if (cBreaker.LpDameTake < maxBreakDame && cBreaker.TgDameTake >= minBreakDame)
			localBreakList.push_back(cBreaker);
	}

	{
		std::lock_guard<std::mutex> lock(breakListMutex);
		breakList.insert(breakList.end(), localBreakList.begin(), localBreakList.end());
		std::sort(breakList.begin(), breakList.end(), AutoCrystal::sortCrystalByTargetDame);
	}
}

void AutoCrystal::placeCrystal() {
	std::lock_guard<std::mutex> lock(placeListMutex);

	if (placeList.empty() || !isHoldingCrystal())
		return;

	auto* gm = mc.getGameMode();

	if (placeDelayTick >= placeDelay) {
		for (CrystalPlacement& placement : placeList) {
			auto rotationToPlacement = mc.getLocalPlayer()->stateVectorComponent->pos.CalcAngle(placement.placePos.toFloat());
			rotAnglePlace = rotationToPlacement;
			placerot = true;
			mc.getLocalPlayer()->swing();
			//InteractPacket inter(InteractAction::LEFT_CLICK, mc.getLocalPlayer()->getRuntimeID(), placement.placePos.toFloat());
			gm->buildBlock(placement.placePos, Math::random(0, 5), false);
			//mc.getClientInstance()->loopbackPacketSender->sendToServer(&inter);
			dam = placement.TgDameTake;

			break; // Exit loop after placing a crystal
		}

		placeDelayTick = 0;
	}
	else {
		rotAnglePlace = {};

		placeDelayTick++;
	}
}
void attack(Actor* target) {
	auto lp = mc.getLocalPlayer();
	if (!mc.getGameMode()->attack(target))  // this returns a bool if it successfully attacks iirc
		return;
	lp->swing();
	//	Game.cpsLeft.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
}
void AutoCrystal::breakCrystal() {
	std::lock_guard<std::mutex> lock(breakListMutex);

	if (!autoBreak || breakList.empty())
		return;

	auto* gm = mc.getGameMode();

	if (breakDelayTick >= breakDelay) {
		highestID = breakList[0].crystalActor->getRuntimeID();



		attack(breakList[0].crystalActor);

		breakDelayTick = 0;
	}
	else {
		rotAngleBreak = {};

		breakDelayTick++;
	}
}

void AutoCrystal::breakIdPredictCrystal() {
	std::lock_guard<std::mutex> lock(breakListMutex);

	if (!autoBreak || placeList.empty())
		return;

	auto* gm = mc.getGameMode();

	if (sendDelayTick >= sendDelay) {
		shouldChangeID = true;

		for (auto i = 0; i < packets; i++) {
			InteractPacket inter(InteractAction::LEFT_CLICK, placeList[0].targetActor->getRuntimeID(), placeList[0].targetActor->stateVectorComponent->pos.sub(Vec3<int>(0.f, 0.2f, 0.f)));
			//gm->attack(placeList[0].targetActor);
			attack(placeList[0].targetActor);
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&inter);
			highestID++;
		}

		highestID -= packets;
		shouldChangeID = false;
		sendDelayTick = 0;
	}
	else {
		sendDelayTick++;
	}
}


int getCrystal() {
	auto* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	auto* inv = plrInv->inventory;

	for (auto i = 0; i < 9; i++) {
		auto* itemStack = inv->getItemStack(i);

		if (itemStack->isValid() && itemStack->getItemPtr()->gettexture_name() == "end_crystal") {
			return i;
		}
	}
	return plrInv->selectedSlot;
}
#include <string> // for string and to_string()
void AutoCrystal::onNormalTick(Actor* actor) {
	cleardalist();
	auto* localPlayer = mc.getLocalPlayer();
	auto* level = localPlayer->getLevel();
	static PacketMine* blockReachMod = (PacketMine*)client->moduleMgr->getModule("PacketMine");

	if (!level || !localPlayer->dimension->blockSource) return;

	auto* gm = mc.getGameMode();
	auto* plrInv = localPlayer->getPlayerInventory();
	auto* inv = plrInv->inventory;

	// Check if PacketMine is actively mining and eating
	auto* itemStack = localPlayer->getCarriedItem();
	auto* item = itemStack->getItemPtr();
	if (item != nullptr && item->isFood()) {
		if (localPlayer->getItemUseDuration() > 0) return;
	}
	if (blockReachMod->getIsMining()) return;

	// Get the initial crystal count
	int initialCrystalsAmount = inv->getItemStack(getCrystal())->stackCount;

	for (auto* target : level->getRuntimeActorList()) {
		entityList.push_back(target);
		if (TargetUtils::isTargetValid(target, Mob) &&
			localPlayer->stateVectorComponent->pos.dist(target->stateVectorComponent->pos) < targetRange) {
			targetList.push_back(target);
		}
	}

	if (targetList.empty()) return;

	std::sort(targetList.begin(), targetList.end(), AutoCrystal::sortEntityByDist);
	float distance = 0;
	std::string namess;
	if (autoPlace) generatePlacements(targetList[0]);
	if (localPlayer->stateVectorComponent->pos.dist(targetList[0]->stateVectorComponent->pos) < targetRange)
	{
		distance = localPlayer->stateVectorComponent->pos.dist(targetList[0]->stateVectorComponent->pos);

	}
	if (targetList.empty())
	{
		namess = "";
	}
	else if (!targetList.empty())
	{
		namess = targetList[0]->getNameTag()->c_str();
	}
	else if (localPlayer->stateVectorComponent->pos.dist(targetList[0]->stateVectorComponent->pos) > targetRange)
	{
		distance = 0;
	}
	currenttarget = targetList[0];
	names = std::string("[") + namess + std::string(",") + std::string(std::to_string(distance)) + std::string("]");
	if (autoBreak) getCrystalActorList(targetList[0]);

	auto bestSlot = getCrystal();
	auto oldSlot = plrInv->selectedSlot;
	auto shouldSwitch = (bestSlot != plrInv->selectedSlot);
	if (shouldSwitch) {
		plrInv->selectedSlot = bestSlot;
	}

	if (isHoldingCrystal()) {
		crystalSpeed = dam;
		prevCrystalsAmount = inv->getItemStack(plrInv->selectedSlot)->stackCount;
		localPlayer->addExperience(1);
		placeCrystal();
		breakCrystal();
		if (idPredict) breakIdPredictCrystal();
	}
	if (shouldSwitch) plrInv->selectedSlot = oldSlot;
}


/*	/*if (packet->getId() == PacketID::PlayerAuthInput || packet->getId() == PacketID::MovePlayerPacket) {
		auto* authPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);
		auto* movePacket = reinterpret_cast<MovePlayerPacket*>(packet);
		authPacket->ticksAlive = Ticks;
		movePacket->tick = Ticks;
		movePacket->actorRuntimeID = mc.getLocalPlayer()->getRuntimeID();
		authPacket->rotation = rotAnglePlace;
		movePacket->rotation = rotAnglePlace;
		authPacket->headYaw = rotAnglePlace.y;
		movePacket->headYaw = rotAnglePlace.y;

			if (!targetList.empty() && rotMode == 1 && packet->getId() == PacketID::PlayerAuthInput) {
		PlayerAuthInputPacket* authPacket = (PlayerAuthInputPacket*)packet;
		authPacket->rotation = rotAngle;
		authPacket->headYaw = rotAngle.y;
	}*/
void AutoCrystal::onSendPacket(Packet* packet, bool& shouldCancel) {
	if (packet->getId() == PacketID::PlayerAuthInput) {
		auto* authPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);
		authPacket->rotation = rotAnglePlace;
		authPacket->headYaw = rotAnglePlace.y;
	}

	/*
	}

	if (!shouldChangeID) return;

	if (packet->getId() == PacketID::InventoryTransaction) {
		auto* invPacket = reinterpret_cast<InventoryTransactionPacket*>(packet);
		auto* invComplex = invPacket->transaction.get();

		if (invComplex->type == ComplexInventoryTransaction::Type::ItemUseOnEntityTransaction) {
			*(int*)((uintptr_t)(invComplex)+0x68) = highestID;
		}
	}
	else if (packet->getId() == PacketID::LevelSoundEvent) {
		shouldCancel = true;
	}*/
}

void AutoCrystal::onRender(MinecraftUIRenderContext* renderCtx) {
	int placed = 0;
	for (CrystalPlacement& placement : placeList) {
		Vec3<float> drawboxCenter = placement.placePos.toFloat().add(0.5f, 1.5f, 0.5f);
		RenderUtils::drawBox(placement.placePos.toFloat(), UIColor(0, 255, 255, 50), UIColor(0, 255, 255, 255), 0.3f, true, false);
		placed++;

		if (placed >= multiPlace) break;
	}
}
void AutoCrystal::onImGuiRender(ImDrawList* d) {
	LocalPlayer* lp = mc.getLocalPlayer();
	if (lp == nullptr) return;
	if (lp->getLevel() == nullptr) return;
	if (!mc.getClientInstance()->minecraftGame->canUseKeys) return;
	static Colors* colorsMod = (Colors*)client->moduleMgr->getModule("Colors");
	UIColor mainColor = colorsMod->getColor();
	int placed = 0;
	for (CrystalPlacement& placement : placeList) {
		Vec2<float> pos;
		if (ImGuiUtils::worldToScreen(placement.placePos.toFloat().add(0.f, 0.5f, 0.f), pos)) {
			float dist = placement.placePos.toFloat().dist(mc.getLocalPlayer()->stateVectorComponent->pos);

			float size = fmax(0.65f, 3.f / dist);
			if (size > 2.f) size = 2.f;
			std::string name = std::to_string(dam);
			name = Utils::sanitize(name);
			float textSize = 2.5f * size;
			float textWidth = ImGuiUtils::getTextWidth(name, textSize);
			float textHeight = ImGuiUtils::getTextHeight(textSize);
			Vec2<float> textPos = Vec2<float>(pos.x - textWidth / 2.f, pos.y - textHeight / 2.f);

			if (Damagerender) {
				ImGuiUtils::drawText(textPos, name, UIColor(255, 255, 255, 255), textSize, true);
			}
		}
		// Break loop after rendering the current placement
		break;
	}
}