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
#include "../../../../Utils/Player/EffectUtils.h"

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
	addBoolCheck("Anti-Weakness", "antiWeakness checks if the player has weakness and not strength, then equips the best item to counteract weakness.", &antiWeakness, 1);
	//ID
	addBoolCheck("ID Predict", "NULL", &idPredict);
	addBoolCheck("Set Dead", "is a flag that indicates if the target (e.g., crystal) should be destroyed and despawned, triggering an attack.", &setDead, 3);
	addSlider<int>("Packets", "NULL", ValueType::INT_T, &packets, 1, 100);
	addEnumSetting("Switch", "Switch Crystal", { "None", "Regular", "Spoof", "Packet" }, &switchType);
	// Extrapolation
	addBoolCheck("Extrapolation", "", &extrapolation, 4);
	addSlider<float>("Intensity", "", ValueType::FLOAT_T, &extrapolateAmount, 0.f, 20.f, 4);
	addBoolCheck("EatStop", "If you eat,stop tou crystal work", &eatstop, 2);
	addBoolCheck("Swing", "Swings your arm", &swing, 2);
	//addSlider<int>("Ticks", "NULL", ValueType::INT_T, &Ticks, 0, 100);
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
int AutoCrystal::getBest() {
	PlayerInventory* plrInv = mc.player()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	float damage = 0.f;
	int slot = plrInv->selectedSlot;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = inv->getItemStack(i);
		if (itemStack->isValid()) {
			float currentDamage = itemStack->getItemPtr()->getAttackDamage() + (1.25f * itemStack->getEnchantLevel(EnchantID::sharpness));
			if (currentDamage > damage) {
				damage = currentDamage;
				slot = i;
			}
		}
	}
	return slot;
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
int AutoCrystal::getEndCrystal() {
	PlayerInventory* plrInv = mc.player()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = inv->getItemStack(i);
		if (itemStack->isValid() && itemStack->getItemPtr()->gettexture_name() == "end_crystal") return i;
	}
	return plrInv->selectedSlot;
}

void AutoCrystal::placeCrystal(GameMode* gm) {
	if (placeList.empty()) return;
	int maxPlace = 0;
	if (mc.player()->getPlayerInventory()->selectedSlot != getEndCrystal() && switchType != 3) return;
	if (placeDelayTick >= placeDelay) {
		for (CrystalPlacement& place : placeList) {
			if (swing) mc.player()->swing();
			mc.player()->getLevel()->setHitResult(HitResultType::AIR);
			gm->buildBlock(place.placePos, Math::random(0, 5), false);
			mc.player()->getLevel()->setHitResult(HitResultType::AIR);
			maxPlace++;
			if (maxPlace >= multiPlace) break;
		}
		placeDelayTick = 0;
	}
	else placeDelayTick++;
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
void AutoCrystal::explodeEndCrystal(GameMode* gm) {
	if (breakList.empty()) return;
	PlayerInventory* plrInv = mc.player()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	if (IexplodeDelay >= breakDelay) {
		if (antiWeakness) {
			if (((DamageUtils::getPlayerAttackDamage() <= 0) || (EffectUtils::hasEffect(EFFECTID::WEAKNESS) && !EffectUtils::hasEffect(EFFECTID::STRENGTH)))) {
				plrInv->selectedSlot = getBest();
				MobEquipmentPacket pk(mc.player()->getRuntimeID(), mc.player()->getPlayerInventory()->inventory->getItemStack(getBest()), getBest(), getBest());
				mc.player()->sendNetworkPacket(pk);
				mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
			}
		}
		if (idPredict) highestID = breakList[0].crystalActor->getRuntimeID();
		for (Actor* crystal : mc.player()->getLevel()->getRuntimeActorList()) {
			if (crystal == nullptr) continue;
			if (crystal->getEntityTypeId() != 71) continue;
			if (crystal->getEyePos().dist(mc.player()->getEyePos()) > breakRange) continue;
			gm->attack(crystal);
			break;
		}
		gm->attack(breakList[0].crystalActor);
		if (setDead) {
			breakList[0].crystalActor->kill();
			breakList[0].crystalActor->despawn(); 
			gm->attack(breakList[0].crystalActor);
		}
		IexplodeDelay = 0;
	}
	else IexplodeDelay++;
}

void AutoCrystal::breakIdPredictCrystal(GameMode* gm) {
	if (placeList.empty()) return;
	if (sendDelayTick >= sendDelay) {
		shouldChangeID = true;
		for (int i = 0; i < packets; i++) {
			InteractPacket inter(InteractAction::LEFT_CLICK, placeList[0].targetActor->getRuntimeID(), placeList[0].targetActor->stateVectorComponent->pos.sub(Vec3<int>(0.f, 0.2f, 0.f)));
			if (!breakList.empty()) gm->attack(breakList[0].crystalActor);
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&inter);
			highestID++;
		}
		highestID -= packets;
		shouldChangeID = false;
		sendDelayTick = 0;
	}
	sendDelayTick++;
}


#include <string> // for string and to_string()
void AutoCrystal::onNormalTick(Actor* randomActorIDFK) {
	GameMode* gm = mc.getGameMode();
	PlayerInventory* plrInv = mc.player()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	placeList.clear();
	breakList.clear();
	targetList.clear();
	entityList.clear();
	int oldSlot = plrInv->selectedSlot;
	if (eatstop && mc.player()->getItemUseDuration() > 0) return;
	if (mc.player()->getLevel()->getRuntimeActorList().empty()) return;
	for (Actor* target : mc.player()->getLevel()->getRuntimeActorList()) {
		if (target == nullptr) continue;
		entityList.push_back(target);
		if (!TargetUtils::isTargetValid(target,false)) continue;
		if (target->getHumanPos().dist(mc.player()->getHumanPos()) > targetRange) continue;
		targetList.push_back(target);
	}
	if (Mob) targetList.push_back(mc.player());
	if (!targetList.empty()) {
		for (Actor* target : targetList) {
			if (autoPlace) generatePlacements(target);
			if (autoBreak) getCrystalActorList(target);
			if (switchType != 0) {
				if (switchType != 3) plrInv->selectedSlot = getEndCrystal(); // homeless method MOBEQ ON TOP
				if (switchType != 1) {
					MobEquipmentPacket pk(mc.player()->getRuntimeID(), mc.player()->getPlayerInventory()->inventory->getItemStack(getEndCrystal()), getEndCrystal(), getEndCrystal());
					mc.getLocalPlayer()->sendNetworkPacket(pk);
					mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
				}
			}
			if (autoPlace) placeCrystal(gm);
			if (autoBreak) explodeEndCrystal(gm);
			if (idPredict) breakIdPredictCrystal(gm);
			break;
		}
		if (switchType == 2) plrInv->selectedSlot = oldSlot;
		if (switchType != 1 && switchType != 0) {
			MobEquipmentPacket pk(mc.player()->getRuntimeID(), mc.player()->getPlayerInventory()->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
			mc.getLocalPlayer()->sendNetworkPacket(pk);
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
		}
	}
}
void AutoCrystal::onSendPacket(Packet* packet, bool& shouldCancel) {
	if (packet->getId() == PacketID::PlayerAuthInput) {
		auto* authPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);
		authPacket->rotation = rotAnglePlace;
		authPacket->headYaw = rotAnglePlace.y;
	}
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