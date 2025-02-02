#include "AnchorAura.h"
#include "../../../Client.h"
#include "../../../../Utils/BlockUtils.h"
bool fpAnchor = false;
AnchorAura::AnchorAura() : Module("AnchorAura", "Places and explodes anchor", Category::COMBAT) {
	// Place
	addBoolCheck("Place", "Place anchors at the best position", &placeAnchor);
	addBoolCheck("Face", "Place anchors at their face", &fpAnchor);
	addBoolCheck("Air Place", "Place anchors without needing for any support/neighboring blocks", &airplace);
	addSlider<int>("Place Delay", "Delay in ticks between each placement(Respawn Anchor)", ValueType::INT_T, &anchorDelay, 0, 20);
	addSlider<int>("Glowstone Delay", "Delay in ticks between each placement(GlowStone)", ValueType::INT_T, &gsDelay, 0, 20);
	addSlider<float>("Place Proximity", "Maximum distance of a placement from the target before switching to another position, its like place range but for target", ValueType::FLOAT_T, &placeProximity, 1.f, 12.f);
	addSlider<float>("Place Range", "If a placement's distance is greater than this then we will look for another placement in the placelist", ValueType::FLOAT_T, &placeRange, 1.f, 20.f);
	addSlider<float>("Local Damage.", "If a placement's damage will this more than this to you then we will use another placement", ValueType::FLOAT_T, &localDamagePlace, 1.f, 36.f);
	addSlider<float>("Enemy Damage.", "If a placement's damage to the target will deal less than this then we use another placement", ValueType::FLOAT_T, &enemyDamagePlace, 1.f, 20.f);
	// Explode
	addBoolCheck("Explode", "Explode best anchor", &explodeAnchor);
	addSlider<int>("Explode Delay", "Delay in ticks between each placement(Respawn Anchor)", ValueType::INT_T, &explodeDelay, 0, 20);
	addSlider<float>("Explode Range", "If an anchors's distance is greater than this then we will look for another anchor in the anchorlist", ValueType::FLOAT_T, &explodeRange, 1.f, 20.f);
	addSlider<float>("Explode Proximity", "Maximum distance of a placement from the target before switching to another position, its like place range but for target", ValueType::FLOAT_T, &explodeProximity, 1.f, 12.f);
	addSlider<float>("Local Break Damage", "If a crystal's damage will this more than this to you then we will use another crystal", ValueType::FLOAT_T, &localDamageExplode, 1.f, 36.f);
	addSlider<float>("Enemy Break Damage", "If a crystal's damage to the target will deal less than this then we use another crystal", ValueType::FLOAT_T, &enemyDamageExplode, 1.f, 20.f);
	// Logic
	addBoolCheck("EatStop", "Stops when eating", &eatStop);
	addSlider<float>("Target Range", "If a targets's distance is greater than this then we will look for another target", ValueType::FLOAT_T, &targetRange, 1.f, 20.f);
	addEnumSetting("Trap", "Make it so they cant escape lololol", { "Disabled", "Normal", "Minimal" }, &trapMode);
	addBoolCheck("Self", "Test on self", &self);
	// Render
	addBoolCheck("Render", "Render placement", &render);
	addBoolCheck("Render 2D", "Make that shit FLAT", &render2d);
	addBoolCheck("Fade", "Cool fade renders from Gamesense Tech", &fade);
	addSlider<float>("Fade Duration", "Duration of fading in seconds", ValueType::FLOAT_T, &fadeDur, 0.f, 3.f);
	addSlider<int>("Alpha", "Opacity", ValueType::INT_T, &alpha, 0, 255);
	addSlider<int>("Line Alpha", "Opacity", ValueType::INT_T, &lineAlpha, 0, 255);
}

void AnchorAura::onEnable() {

}

void AnchorAura::onDisable() {

}

bool AnchorAura::isPlaceValid(const Vec3<float>& placePos, Actor* target) {
	Vec3<float> plrPos = target->getPosition()->floor(); plrPos.y -= 1.f;
	if (mc.getLocalPlayer()->getHumanPos().dist(placePos) > placeRange) return false;
	if (BlockUtils::getBlockId(placePos) == 66063) return true;
	if (!BlockUtils::isReplaceable(placePos)) return false;
	if (!BlockUtils::canBuildBlock(placePos) && !airplace) return false;
	if (!fpAnchor && placePos.y == plrPos.y + 1) return false;
	Vec3<float> intersectPos = Vec3<float>(placePos.x, placePos.y, placePos.z);
	for (Actor* entity : entityList) {
		if (entity->getEyePos().dist(target->getEyePos()) > 6.f) continue;
		if (!TargetUtils::isTargetValid(entity, true)) continue;
		AABB targetAABB = *entity->getAABB();
		if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	}
	return true;
}

bool AnchorAura::isExplodeValid(const Vec3<float>& explodePos, Actor* target) {
	if (mc.getLocalPlayer()->getHumanPos().dist(explodePos) > explodeRange) return false;
	if (BlockUtils::getBlockId(explodePos) != 66063) return false;
	return true;
}

bool AnchorAura::sortByDamage(AnchorStruct a1, AnchorStruct a2) {
	return a1.targetDmg > a2.targetDmg;
}

void AnchorAura::getPlaceList(Actor* target) {
	int radius = (int)placeProximity;
	for (int x = -radius; x <= radius; x++) {
		for (int y = -radius; y <= 5.f; y++) {
			for (int z = -radius; z <= radius; z++) {
				Vec3<float> targetPos = target->getHumanPos();
				Vec3<int> blockPos(targetPos.x + x, targetPos.y + y, targetPos.z + z);
				if (isPlaceValid(blockPos.toFloat(), target)) {
					AnchorPlace placement(blockPos, target);
					if (placement.localDmg <= localDamagePlace && placement.targetDmg >= enemyDamagePlace) placeList.push_back(placement);
				}
			}
		}
	}
	std::sort(placeList.begin(), placeList.end(), sortByDamage);
}

void AnchorAura::getExplodeList(Actor* target) {
	int radius = (int)explodeProximity;
	for (int x = -radius; x <= radius; x++) {
		for (int y = -radius; y <= 5.f; y++) {
			for (int z = -radius; z <= radius; z++) {
				Vec3<float> targetPos = target->getHumanPos();
				Vec3<int> blockPos(targetPos.x + x, targetPos.y + y, targetPos.z + z);
				if (isPlaceValid(blockPos.toFloat(), target) && placeAnchor) {
					AnchorPlace placement(blockPos, target);
					if (placement.localDmg <= localDamagePlace && placement.targetDmg >= enemyDamagePlace) placeList.push_back(placement);
				}
				if (isExplodeValid(blockPos.toFloat(), target) && explodeAnchor) {
					AnchorPlace explode(blockPos, target);
					if (explode.localDmg <= localDamageExplode && explode.targetDmg >= enemyDamageExplode) explodeList.push_back(explode);
				}
			}
		}
	}
	std::sort(placeList.begin(), placeList.end(), sortByDamage);
	std::sort(explodeList.begin(), explodeList.end(), sortByDamage);
}

int AnchorAura::getItem(int id) {
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = inv->getItemStack(i);
		if (itemStack->isValid() && itemStack != nullptr && itemStack->getItemPtr()->itemId == id) return i;
	}
	return plrInv->selectedSlot;
}

void AnchorAura::mineBlocker(Actor* actor) {
	PacketMine* packetMine = (PacketMine*)client->moduleMgr->getModule("PacketMine");
	Vec3<int> plrPos = actor->getPosition()->floor().toInt().sub(Vec3<int>(0, 1, 0));
	Vec3<int> block = plrPos.add(0, 2, 0);
	bool b = false;
	if (packetMine->getBreakPos() == Vec3<int>(0, 0, 0) || mc.getGameMode()->destroyProgress <= 0.f) {
		packetMine->setBreakPos(block, 0);
		mc.getGameMode()->startDestroyBlock(block, 0, b);
	}
	if (BlockUtils::getBlockId(block) == 527 || BlockUtils::isReplaceable(block)) packetMine->shouldDestroy = false;
	else if (BlockUtils::getBlockId(block) != 527 && !BlockUtils::isReplaceable(block)) packetMine->shouldDestroy = true;
}
void AnchorAura::trap(Actor* actor) {
	std::vector<Vec3<int>> generalPlacements = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };
	Vec3<int> plrPos = actor->getPosition()->floor().toInt().sub(Vec3<int>(0, 1, 0));
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	MobEquipmentPacket anchor(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(49)), getItem(49), getItem(49));
	plrInv->selectedSlot = getItem(49);
	mc.getClientInstance()->loopbackPacketSender->sendToServer(&anchor);
	for (Vec3<int> check : generalPlacements) {
		if (trapMode == 2) {
			BlockUtils::buildBlock(plrPos.add(check).add(0, 2, 0), airplace);
		}
		else if (trapMode == 1) {
			for (int y = -1; y < 3; y++) {
				BlockUtils::buildBlock(plrPos.add(check).add(0, y, 0), airplace);
			}
		}
	}
}
void AnchorAura::placeAnchors(GameMode* gm) {
	if (placeList.empty()) return;
	//Debugger* debug = (Debugger*)client->moduleMgr->getModule("Debugger");
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	if (iAnchorDelay >= anchorDelay) {
		MobEquipmentPacket anchor(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(65264)), getItem(65264), getItem(65264));
		plrInv->selectedSlot = getItem(65264);
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&anchor);
		for (const AnchorPlace placement : placeList) {
			if (inv->getItemStack(plrInv->selectedSlot)->getItemPtr()->itemId == 65264) {
				BlockUtils::buildBlock(placement.placePos, airplace);
			}
			break;
		}
		iAnchorDelay = 0;
	}
	else iAnchorDelay++;
}

void AnchorAura::igniteAnchors(GameMode* gm) {
	//Debugger* debug = (Debugger*)client->moduleMgr->getModule("Debugger");
	Disabler* disabler = (Disabler*)client->moduleMgr->getModule("Disabler");
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	if (explodeList.empty()) {
		
		return;
	}
	if (iGsDelay >= gsDelay) {
		MobEquipmentPacket glowstone(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(89)), getItem(89), getItem(89));
		plrInv->selectedSlot = getItem(89);
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&glowstone);
		for (const AnchorPlace explode : explodeList) {
			
			if (BlockUtils::getBlockName(explode.placePos) != "air" && !BlockUtils::isReplaceable(explode.placePos)) {
				if (disabler->isEnabled()) mc.getLocalPlayer()->getLevel()->setHitResult(HitResultType::AIR);
				gm->buildBlock(explode.placePos, 0, false);
				if (disabler->isEnabled()) mc.getLocalPlayer()->getLevel()->setHitResult(HitResultType::AIR);
			}
			break;
		}
		iGsDelay = 0;
	}
	else iGsDelay++;
}

void AnchorAura::explodeAnchors(GameMode* gm) {
	
	Disabler* disabler = (Disabler*)client->moduleMgr->getModule("Disabler");
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	if (explodeList.empty()) {
		return;
	}
	if (iExplodeDelay >= explodeDelay) {
		for (const AnchorPlace explode : explodeList) {
			MobEquipmentPacket anchor(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(65264)), getItem(65264), getItem(65264));
			plrInv->selectedSlot = getItem(65264);
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&anchor);
			if (!BlockUtils::isReplaceable(explode.placePos)) {
				
				if (disabler->isEnabled()) mc.getLocalPlayer()->getLevel()->setHitResult(HitResultType::AIR);
				gm->buildBlock(explode.placePos, 0, false);
				if (disabler->isEnabled()) mc.getLocalPlayer()->getLevel()->setHitResult(HitResultType::AIR);
			}
			break;
		}
		iExplodeDelay = 0;
	}
	else iExplodeDelay++;
}

void AnchorAura::onNormalTick(Actor* actor) {
	GameMode* gm = mc.getGameMode();
	Disabler* disabler = (Disabler*)client->moduleMgr->getModule("Disabler");
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	placeList.clear();
	explodeList.clear();
	entityList.clear();
	targetList.clear();
	int oldSlot = plrInv->selectedSlot;
	MobEquipmentPacket old(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(oldSlot), oldSlot, oldSlot);
	if (eatStop && mc.getLocalPlayer()->getItemUseDuration() > 0) return;
	for (Actor* target : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (target == nullptr) continue;
		entityList.push_back(target);
		if (!TargetUtils::isTargetValid(target)) continue;
		if (target->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > targetRange) continue;
		targetList.push_back(target);
		break;
	}
	if (self) targetList.push_back(mc.getLocalPlayer());
	if (!targetList.empty()) {
		for (Actor* target : targetList) {
			//if (placeAnchor) getPlaceList(target);
			getExplodeList(target);
			break;
		}
		if (trapMode != 0) trap(targetList[0]);
		igniteAnchors(gm);
		if (placeAnchor) placeAnchors(gm);
		if (explodeAnchor) explodeAnchors(gm);
		plrInv->selectedSlot = oldSlot;
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&old);
	}
}
Vec3<float> fadePos;
void AnchorAura::onRender(MinecraftUIRenderContext* renderCtx) {
	Colors* colorMod = (Colors*)client->moduleMgr->getModule("Colors");
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
	for (AnchorPlace placement : placeList) {
		if (fade && fadePos != placement.placePos.toFloat()) fadeList.push_back({ fadePos, 0.f, fadeDur });
		RenderUtils::drawBox(placement.placePos.toFloat().add(0, render2d ? 1 : 0, 0), UIColor(colorMod->getColor().r, colorMod->getColor().g, colorMod->getColor().b, alpha), UIColor(colorMod->getColor().r, colorMod->getColor().g, colorMod->getColor().b, lineAlpha), 0.3f, true, false);
		fadePos = placement.placePos.toFloat();
		break;
	}
}
