#include "AutoCrystal.h"
#include "../../../Client.h"
#include "../../../Client.h"
#include "../../../../Utils/DamageUtils.h"
#include "../../../../Utils/BlockUtils.h"
#include "../../../../Utils/EffectUtils.h"
AutoCrystal::AutoCrystal() : Module("AutoCrystal", "Places and breaks crystals automatically.", Category::COMBAT) {
	// Place
	addBoolCheck("Place", "Place end crystals at the best position", &placeCrystal);
	addSlider<int>("Place Delay", "Delay in ticks between each placement", ValueType::INT_T, &placeDelay, 0, 20);
	addSlider<int>("Place Amount", "Amount of crystals we place everytime", ValueType::INT_T, &placeAmount, 1,5);
	addSlider<float>("Place Range", "If a placement's distance is greater than this then we will look for another placement in the placelist", ValueType::FLOAT_T, &placeRange, 1.f, 20.f);
	addSlider<float>("Proximity", "Maximum distance of a placement from the target before switching to another position, its like place range but for target", ValueType::FLOAT_T, &placeProximity, 1.f, 12.f);
	addSlider<float>("Max Y", "like proximity but for y level", ValueType::FLOAT_T, &maxY, 1.f, 12.f);
	addSlider<float>("Local Place Damage", "If a placement's damage will this more than this to you then we will use another placement", ValueType::FLOAT_T, &localDamagePlace, 1.f, 36.f);
	addSlider<float>("Enemy Place Damage", "If a placement's damage to the target will deal less than this then we use another placement", ValueType::FLOAT_T, &enemyDamagePlace, 1.f, 20.f);
	// Explode
	addBoolCheck("Explode", "Explode end crystals that deal the most damage", &explodeCrystal);
	addEnumSetting("Explode Type", "How we explode end crystal", { "Smart", "All" }, &breakType);
	addSlider<int>("Explode Delay", "Delay in ticks between each attack", ValueType::INT_T, &explodeDelay, 0, 20);
	addSlider<float>("Explode Range", "If a crystal's distance is greater than this then we will look for another end crystal in the terrain", ValueType::FLOAT_T, &breakRange, 1.f, 20.f);
	addSlider<float>("Local Break Damage", "If a crystal's damage will this more than this to you then we will use another crystal", ValueType::FLOAT_T, &localDamageBreak, 1.f, 36.f);
	addSlider<float>("Enemy Break Damage", "If a crystal's damage to the target will deal less than this then we use another crystal", ValueType::FLOAT_T, &enemyDamageBreak, 1.f, 20.f);
	addBoolCheck("Anti-Weakness", "When have weakness we switch to slot that deals most dmg", &antiWeakness);
	// Logic
	addEnumSetting("Protocol", "Logic for the ca to use, use the Java protocol for servers like endzone", { "Vanilla", "Java" }, &protocol);
	addSlider<float>("Target Range", "If a players distance is greater than this then we wont consider them a potential target", ValueType::FLOAT_T, &targetRange, 1.f, 20.f);
	addBoolCheck("Eat Check", "Stops the hack when eating", &eatCheck);
	addBoolCheck("Rotate", "Rotate to where ur placing", &rotate);
	addBoolCheck("Swing", "Swings your arm", &swing);
	addEnumSetting("Switch", "How we switch to end crystal", { "None", "Regular", "Spoof", "Packet" }, &switchType);
	addBoolCheck("Self", "Targets self for test", &test);
	// IDPredict
	addBoolCheck("Predict", "Predicts the next end crystal runtime id to attack it before it even spawns thus attacking it faster than normally possible", &idPredict);
	addSlider<int>("Predict Delay", "Delay in ticks between each predict", ValueType::INT_T, &boostDelay, 0, 20);
	addSlider<int>("Predict Packets", "Amount of packets we will send, the more the faster", ValueType::INT_T, &idPacket, 1, 10);
	addBoolCheck("Set Dead", "Sets end crystal as dead, this will make crystalaura slightly faster. be sure that your crystal will infact die when this is on, some anticheats can prevent this thus making u ghost crystal", &setDead);
	// Extrapolation
	addBoolCheck("Extrapolation", "Gets their velocity(if the server sends it) and predicts their next pos and places near there", &extrapolation);
	addSlider<float>("Intensity", "Amount of velocity you wanna multiply and predict idk", ValueType::FLOAT_T, &extrapolateAmount, 0.f, 20.f);
	// Render
	addBoolCheck("Render", "Render placement", &render);
	addBoolCheck("Render 2D", "Make that shit FLAT", &render2d);
	addBoolCheck("Render Damage", "Renders the amount of damage we are dealing", &renderDamage);
	addBoolCheck("Fade", "Cool fade renders from Gamesense Tech", &fade);
	addSlider<float>("Fade Duration", "Duration of fading in seconds", ValueType::FLOAT_T, &fadeDur, 0.f, 3.f);
	addSlider<int>("Alpha", "Opacity", ValueType::INT_T, &alpha, 0, 255);
	addSlider<int>("Line Alpha", "Opacity", ValueType::INT_T, &lineAlpha, 0, 255);
}

std::string AutoCrystal::getModName() {
	return protocol == 0 ? "Vanilla" : "Java";
}

void AutoCrystal::onEnable() {
	placeList.clear();
	breakList.clear();
	targetList.clear();
	entityList.clear();
}

void AutoCrystal::onDisable() {
	placeList.clear();
	breakList.clear();
	targetList.clear();
	entityList.clear();
}

float CrystalUtil::getExplosionDamage(const Vec3<float>& crystalPos, Actor* target) {
	AutoCrystal* autoCrystal = (AutoCrystal*)client->moduleMgr->getModule("AutoCrystal");
	return DamageUtils::getExplosionDamage(crystalPos, target, autoCrystal->extrapolation ? autoCrystal->extrapolateAmount : 0.f, 0);
}

bool AutoCrystal::isPlaceValid(const Vec3<int>& placePos, Actor* target) {
	Vec3<float> intersectPos = Vec3<float>(placePos.x, placePos.y, placePos.z); intersectPos.y += 1;
	if (!(BlockUtils::getBlockId(placePos) == 49 || BlockUtils::getBlockId(placePos) == 7)) return false;
	if (mc.getLocalPlayer()->dimension->blockSource->getBlock(Vec3<int>(placePos.x, placePos.y + 1, placePos.z))->blockLegacy->blockName != "air") return false;
	if (protocol == 0 && mc.getLocalPlayer()->dimension->blockSource->getBlock(Vec3<int>(placePos.x, placePos.y + 2, placePos.z))->blockLegacy->blockName != "air") return false;
	if (mc.getLocalPlayer()->getHumanPos().dist(placePos) > placeRange) return false;
	if (target->getAABB()->intersects(AABB(intersectPos, intersectPos.add(1.f).add(0.f, protocol == 1 ? 0.5f : 0.f, 0.f)))) return false;
	for (Actor* currentEnt : entityList) {
		if (currentEnt->getEntityTypeId() == 71) continue;
		AABB targetAABB = *currentEnt->getAABB();
		if (currentEnt->getEntityTypeId() == 319) {
			if (protocol == 0) { // Nukkit hitbox are way fatter than java so
				targetAABB.lower = targetAABB.lower.sub(Vec3<float>(0.1f, 0.f, 0.1f));
				targetAABB.upper = targetAABB.upper.add(0.1f, 0.f, 0.1f);
			}
		} if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f).add(0.f, protocol == 1 ? 0.5f : 0.f, 0.f)))) return false;
	}
	return true;
}

int AutoCrystal::getEndCrystal() {
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = inv->getItemStack(i);
		if (itemStack->isValid() && itemStack->getItemPtr()->gettexture_name() == "end_crystal") return i;
	}
	return plrInv->selectedSlot;
}

int AutoCrystal::getBestItem() {
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
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

bool AutoCrystal::sortCrystal(CrystalUtil a1, CrystalUtil a2) {
	return a1.targetDamage > a2.targetDamage;
}

void AutoCrystal::generatePlacements(Actor* target) {
	const int radius = (int)placeProximity;
	for (auto x = -radius; x <= radius; x++) {
		for (auto y = -maxY; y <= 3; y++) {
			for (auto z = -radius; z <= radius; z++) {
				Vec3<float> targetPos = target->getHumanPos();
				Vec3<int> blockPos = Vec3<int>(static_cast<int>(targetPos.x + (target->stateVectorComponent->velocity.x * extrapolateAmount)) + x,
					static_cast<int>(targetPos.y + (target->stateVectorComponent->velocity.y * extrapolateAmount)) + y,
					static_cast<int>(targetPos.z + (target->stateVectorComponent->velocity.z * extrapolateAmount)) + z); // Extra po lat ion
				if (isPlaceValid(blockPos, target)) {
					PlaceUtils placement(blockPos, target);
					if (placement.localDamage < localDamagePlace && placement.targetDamage >= enemyDamagePlace) placeList.push_back(placement);
				}
			}
		}
	}
	std::sort(placeList.begin(), placeList.end(), AutoCrystal::sortCrystal);
}

void AutoCrystal::getCrystalList(Actor* target) {
	for (Actor* entity : entityList) {
		if (entity->getEntityTypeId() != 71) continue;
		if (entity->stateVectorComponent->pos.dist(mc.getLocalPlayer()->stateVectorComponent->pos) > breakRange) continue;
		BreakUtils crystalBreak(entity, target);
			breakList.push_back(crystalBreak);
			if (idPredict) highestId = entity->getRuntimeID();
	}
	std::sort(breakList.begin(), breakList.end(), sortCrystal);
}

void AutoCrystal::placeEndCrystal(GameMode* gm) {
	if (placeList.empty()) return;
	int maxPlace = 0;
	if (mc.getLocalPlayer()->getPlayerInventory()->selectedSlot != getEndCrystal() && switchType != 3) return;
	if (IplaceDelay >= placeDelay) {
		for (PlaceUtils& place : placeList) {
			if (swing) mc.getLocalPlayer()->swing();
			mc.getLocalPlayer()->getLevel()->setHitResult(HitResultType::AIR);
			gm->buildBlock(place.placePos, Math::random(0, 5), false);
			mc.getLocalPlayer()->getLevel()->setHitResult(HitResultType::AIR);
			maxPlace++;
			if (maxPlace >= placeAmount) break;
		}
		IplaceDelay = 0;
	}
	else IplaceDelay++;
}

void AutoCrystal::explodeEndCrystal(GameMode* gm) {
	if (breakList.empty()) return;
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	if (IexplodeDelay >= explodeDelay) {
		if (antiWeakness) {
			if (((DamageUtils::getPlayerAttackDamage() <= 0) || (EffectUtils::hasEffect(EFFECTID::WEAKNESS) && !EffectUtils::hasEffect(EFFECTID::STRENGTH)))) {
				plrInv->selectedSlot = getBestItem();
				MobEquipmentPacket pk(mc.getLocalPlayer()->getRuntimeID(), mc.getLocalPlayer()->getPlayerInventory()->inventory->getItemStack(getBestItem()), getBestItem(), getBestItem());
				mc.getLocalPlayer()->sendNetworkPacket(pk);
				mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
			}
		}
		if (idPredict) highestId = breakList[0].endCrystal->getRuntimeID();
		if (breakType == 1) {
			for (Actor* crystal : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
				if (crystal == nullptr) continue;
				if (crystal->getEntityTypeId() != 71) continue;
				if (crystal->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > breakRange) continue;
				gm->attack(crystal);
				break;
			}
		}
		gm->attack(breakList[0].endCrystal);
		if (setDead) {
			breakList[0].endCrystal->kill();
			breakList[0].endCrystal->despawn(); // CLIENT SIDED FAST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			gm->attack(breakList[0].endCrystal);
		}
		IexplodeDelay = 0;
	}
	else IexplodeDelay++;
}

void AutoCrystal::predictEndCrystal(GameMode* gm) {
	if (placeList.empty()) return;
	int realPacket = idPacket * 5;
	if (IboostDelay >= boostDelay) {
		shouldChange = true;
		for (int i = 0; i < realPacket; i++) {
			InteractPacket inter(InteractAction::LEFT_CLICK, placeList[0].targetEntity->getRuntimeID(), placeList[0].targetEntity->stateVectorComponent->pos.sub(Vec3<int>(0.f, 0.2f, 0.f)));
			if (!breakList.empty()) gm->attack(breakList[0].endCrystal);
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&inter);
			highestId++;
		}
		highestId -= realPacket;
		shouldChange = false;
		IboostDelay = 0;
	}
	else IboostDelay++;
}
void AutoCrystal::onNormalTick(Actor* randomActorIDFK) {
	GameMode* gm = mc.getGameMode();
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	placeList.clear();
	breakList.clear();
	targetList.clear();
	entityList.clear();
	int oldSlot = plrInv->selectedSlot;
	if (eatCheck && mc.getLocalPlayer()->getItemUseDuration() > 0) return;
	if (mc.getLocalPlayer()->getLevel()->getRuntimeActorList().empty()) return;
	for (Actor* target : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (target == nullptr) continue;
		entityList.push_back(target);
		if (!TargetUtils::isTargetValid(target)) continue;
		if (target->getHumanPos().dist(mc.getLocalPlayer()->getHumanPos()) > targetRange) continue;
		targetList.push_back(target);
	}
	if (test) targetList.push_back(mc.getLocalPlayer());
	if (!targetList.empty()) {
		for (Actor* target : targetList) {
			if (placeCrystal) generatePlacements(target);
			if (explodeCrystal) getCrystalList(target);
			if (switchType != 0) {
				if (switchType != 3) plrInv->selectedSlot = getEndCrystal(); // homeless method MOBEQ ON TOP
				if (switchType != 1) {
					MobEquipmentPacket pk(mc.getLocalPlayer()->getRuntimeID(), mc.getLocalPlayer()->getPlayerInventory()->inventory->getItemStack(getEndCrystal()), getEndCrystal(), getEndCrystal());
					mc.getLocalPlayer()->sendNetworkPacket(pk);
					mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
				}
			}
			if (placeCrystal) placeEndCrystal(gm);
			if (explodeCrystal) explodeEndCrystal(gm);
			if (idPredict) predictEndCrystal(gm);
			break;
		}
		if (switchType == 2) plrInv->selectedSlot = oldSlot;
		if (switchType != 1 && switchType != 0) {
			MobEquipmentPacket pk(mc.getLocalPlayer()->getRuntimeID(), mc.getLocalPlayer()->getPlayerInventory()->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
			mc.getLocalPlayer()->sendNetworkPacket(pk);
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
		}
	}
}

void AutoCrystal::onSendPacket(Packet* packet, bool& shouldCancel) {
	if (rotate) {
		Vec2<float> angle2 = mc.getLocalPlayer()->getPosition()->CalcAngle(*breakList[0].endCrystal->getPosition());
		if (packet->getId() == PacketID::MovePlayerPacket) {
			auto* movePkt = (MovePlayerPacket*)packet;
			movePkt->rotation = angle2;
			movePkt->headYaw = angle2.y;
		}
	}
}
Vec3<float> lerpPos;
void AutoCrystal::onRender(MinecraftUIRenderContext* renderCtx) {
	Colors* colorMod = (Colors*)client->moduleMgr->getModule("Colors");
	int placed = 0;
	for (auto it = fadeList.begin(); it != fadeList.end(); ) {
		it->fadeTimer += ImGui::GetIO().DeltaTime;
		float fadeAlpha = 1.f - (it->fadeTimer / it->fadeDuration);
		if (fadeAlpha <= 0.0f) it = fadeList.erase(it);
		else {
			float alpha2 = (alpha / 255.f) * fadeAlpha;
			float lalpha2 = (alpha / 255.f) * fadeAlpha;
			RenderUtils::drawBox(it->lastPos.add(0, render2d ? 1 : 0, 0), UIColor(colorMod->getColor().r, colorMod->getColor().g, colorMod->getColor().b, alpha2 * 255), UIColor(colorMod->getColor().r, colorMod->getColor().g, colorMod->getColor().b, lalpha2 * 255), 0.3f, true, false);
			it++;
		}
	}
	if (placeList.empty()) return;
	for (PlaceUtils& placement : placeList) {
		if (fade && lerpPos != placement.placePos.toFloat()) fadeList.push_back({ lerpPos, 0.f, fadeDur });
		RenderUtils::drawBox(placement.placePos.toFloat().add(0, render2d ? 1 : 0, 0), UIColor(colorMod->getColor().r, colorMod->getColor().g, colorMod->getColor().b, alpha), UIColor(colorMod->getColor().r, colorMod->getColor().g, colorMod->getColor().b, lineAlpha), 0.3f, true, false);
		lerpPos = placement.placePos.toFloat();
		placed++;
		if (placed >= placeAmount) break;
	}
}

void AutoCrystal::onImGuiRender(ImDrawList* d) {
	if (mc.getLocalPlayer() == nullptr) return;
	if (mc.getLocalPlayer()->getLevel() == nullptr) return;
	if (!mc.getClientInstance()->minecraftGame->canUseKeys) return;
	if (!renderDamage) return;
	int placed = 0;
	if (placeList.empty()) return;
	for (PlaceUtils& placement : placeList) {
		Vec2<float> pos;
		if (ImGuiUtils::worldToScreen(placement.placePos.toFloat().add(0.f, 0.75f, 0.f), pos)) {
			float size = 1.f;
			std::string name = std::to_string((int)placement.targetDamage);
			float textSize = 2.5f * size;
			float textWidth = ImGuiUtils::getTextWidth(name, textSize);
			float textHeight = ImGuiUtils::getTextHeight(textSize);
			Vec2<float> textPos = Vec2<float>(pos.x - textWidth / 2.f, pos.y - textHeight / 2.f);
			ImGuiUtils::drawText(textPos, name, UIColor(255, 255, 255, 255), textSize, true);
		}
		placed++;
		if (placed >= placeAmount) break;
	}
}
