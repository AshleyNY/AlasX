#include "CevBreaker.h"
#include "../../../Client.h"
#include "../../../../Utils/BlockUtils.h"

CevBreaker::CevBreaker() : Module("CevBreaker", "Places obsidian on top/side of player, place crystal, break obsidian and explode crystal", Category::COMBAT) {
	addBoolCheck("Silent", "Switch Silently", &silent);
	addBoolCheck("Packet Place", "Place using packet to make it more consistent", &packetPlace);
	addSlider<float>("Target Range", "Range of the target to be added to targetlist", ValueType::FLOAT_T, &targetRange, 1.f, 20.f);
	addSlider<float>("Place Range", "If a placement's distance is greater than this then we will look for another placement in the placelist", ValueType::FLOAT_T, &placeRange, 1.f, 20.f);
	addBoolCheck("Java", "Disregards the block above the end crystal, because in java servers you can place an end crystal even if thers a block above where the end crystal is going to be", &javaMode);
	addBoolCheck("AirPlace", "Places blocks in air. (NOTE: Only works if server allows it)", &airplace);
	addBoolCheck("Side Cev", "Place on sides", &SideCev);
	addBoolCheck("Self", "Test on Self", &self);
	addSlider<int>("Place Delay", "Ticks passed before we place obsidian", ValueType::INT_T, &placeDelay, 0, 20);
	addSlider<int>("Crystal Delay", "Ticks passed before we place crystal", ValueType::INT_T, &explodeDelay, 0, 20);
	addSlider<int>("Destroy Delay", "Ticks passed before we destroy obby", ValueType::INT_T, &destroyDelay, 0, 20);
}

bool CevBreaker::isPlaceValid(Vec3<int> pos, Actor* target) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	Vec3<float> intersectPos = Vec3<float>(pos.x, pos.y, pos.z); intersectPos.y += 1;
	if (localPlayer->dimension->blockSource->getBlock(Vec3<int>(pos.x, pos.y + 1, pos.z))->blockLegacy->blockName != "air") return false;
	if (!javaMode && localPlayer->dimension->blockSource->getBlock(Vec3<int>(pos.x, pos.y + 2, pos.z))->blockLegacy->blockName != "air") return false;
	if (BlockUtils::getBlockName(Vec3<int>(pos.x, pos.y - 1, pos.z)) != "air" && BlockUtils::getBlockName(Vec3<int>(pos.x, pos.y - 1, pos.z)) != "obsidian") return false;
	if (localPlayer->getHumanPos().dist(pos) > placeRange) return false;
	if (target->getAABB()->intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	for (Actor* currentEnt : entityList) {
		if (currentEnt->getEntityTypeId() == 71) continue;
		AABB targetAABB = *currentEnt->getAABB();
		if (currentEnt->getEntityTypeId() == 319) {
			targetAABB.lower = targetAABB.lower.sub(Vec3<float>(0.1f, 0.f, 0.1f));
			targetAABB.upper = targetAABB.upper.add(0.1f, 0.f, 0.1f);
		} if (targetAABB.intersects(AABB(intersectPos, intersectPos.add(1.f)))) return false;
	}
	return true;
}

Vec3<int> CevBreaker::getBestPlace(Actor* target) {
	Vec3<float> actorPos = target->getPosition()->floor(); actorPos.y -= 1.f;
	Vec3<int> place = Vec3<int>(0, 0, 0);
	std::vector<Vec3<int>> checkList = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };
	if (isPlaceValid(Vec3<int>(actorPos.x, actorPos.y + 2, actorPos.z), target)) return Vec3<int>(actorPos.x, actorPos.y + 2, actorPos.z); /// Placing above them should be fine, i see no point for high cev
	if (SideCev) {
		for (Vec3<int> check : checkList) {
			Vec3<int> pos = Vec3<int>(actorPos.x + check.x, actorPos.y + 1, actorPos.z + check.z);
			if (isPlaceValid(pos, target)) {
				place = pos;
				break;
			}
		}
	}
	return place;
}

int CevBreaker::getItem(std::string name) {
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = inv->getItemStack(i);
		if (itemStack->isValid() && itemStack->getItemPtr()->gettexture_name().find(name) != std::string::npos) return i;
	}
	return plrInv->selectedSlot;
}

void CevBreaker::onNormalTick(Actor* actor) {
	bool b = false;
	LocalPlayer* localPlayer = (LocalPlayer*)actor;
	GameMode* gm = mc.getGameMode();
	PlayerInventory* plrInv = localPlayer->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	PacketMine* packetMine = (PacketMine*)client->moduleMgr->getModule("PacketMine");
	if (!packetMine->isEnabled()) packetMine->setEnabled(true);
	int oldSlot = plrInv->selectedSlot;
	placeList.clear();
	entityList.clear();
	targetList.clear();
	for (Actor* target : localPlayer->getLevel()->getRuntimeActorList()) {
		if (target == nullptr) continue;
		entityList.push_back(target);
		if (!TargetUtils::isTargetValid(target)) continue;
		if (target->getHumanPos().dist(localPlayer->getHumanPos()) > targetRange) continue;
		targetList.push_back(target);
	}
	if (self) targetList.push_back(localPlayer);
	if (!targetList.empty()) {
		if (getBestPlace(targetList[0]) != Vec3<int>(0, 0, 0)) placeList.push_back(getBestPlace(targetList[0]));
		if (!placeList.empty()) {
			if ((gm->destroyProgress == 0.f || packetMine->getBreakPos() != placeList[0]) && !BlockUtils::isReplaceable(placeList[0])) {
				packetMine->setBreakPos(placeList[0], 0);
				gm->startDestroyBlock(placeList[0], 0, b);
			}
			packetMine->shouldDestroy = false;
			if (IplaceDelay == -999) IplaceDelay = 0;
			if (IplaceDelay >= placeDelay) {
				if (BlockUtils::isReplaceable(placeList[0])) {
					for (int i = 0; i < 9; i++) {
						ItemStack* itemStack = inv->getItemStack(i);
						if (itemStack->isValid() && itemStack->getItemPtr()->itemId == 49) {
							if (!packetPlace) plrInv->selectedSlot = i;
							if (silent || packetPlace) {
								MobEquipmentPacket obby(localPlayer->getRuntimeID(), plrInv->inventory->getItemStack(i), i, i);
								mc.getLocalPlayer()->sendNetworkPacket(obby);
								mc.getClientInstance()->loopbackPacketSender->sendToServer(&obby);
							}
						}
					}
					BlockUtils::buildBlock(placeList[0], airplace);
				}
				IplaceDelay = 0;
			}
			else IplaceDelay++;
			if (IexplodeDelay == -999) IexplodeDelay = 0;
			if (IexplodeDelay >= explodeDelay) {
				plrInv->selectedSlot = getItem("end_crystal");
				if (silent) {
					MobEquipmentPacket crystal(localPlayer->getRuntimeID(), plrInv->inventory->getItemStack(getItem("end_crystal")), getItem("end_crystal"), getItem("end_crystal"));
					mc.getLocalPlayer()->sendNetworkPacket(crystal);
					mc.getClientInstance()->loopbackPacketSender->sendToServer(&crystal);
				}
				if (inv->getItemStack(plrInv->selectedSlot)->getItemPtr()->gettexture_name() == "end_crystal") {
					gm->buildBlock(placeList[0], Math::random(0, 5), false);
				}
				IexplodeDelay = 0;
			}
			else IexplodeDelay++;
			if (IdestroyDelay == -999) IdestroyDelay = 0;
			if (IdestroyDelay >= destroyDelay) {
				packetMine->shouldDestroy = true;
				IdestroyDelay = 0;
			}
			else IdestroyDelay++;
			for (Actor* entity : entityList) {
				if (entity->getEntityTypeId() != 71) continue;
				if (entity->getHumanPos().dist(localPlayer->getHumanPos()) > 6.f) continue;
				gm->attack(entity);
			}
			if (silent) {
				plrInv->selectedSlot = oldSlot;
				MobEquipmentPacket crystal(localPlayer->getRuntimeID(), plrInv->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
				mc.getLocalPlayer()->sendNetworkPacket(crystal);
				mc.getClientInstance()->loopbackPacketSender->sendToServer(&crystal);
			}
		}
	}
}

void CevBreaker::onRender(MinecraftUIRenderContext* renderCtx) {
	/// i see no point for rendering when packetmine does that already
}

void CevBreaker::onEnable() {
	placeList.clear();
	entityList.clear();
	targetList.clear();
}

void CevBreaker::onDisable() {
	placeList.clear();
	entityList.clear();
	targetList.clear();
}