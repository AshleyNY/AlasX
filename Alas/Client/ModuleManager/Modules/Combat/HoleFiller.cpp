#include "HoleFiller.h"
#include "../../../../Utils/BlockUtils.h"

HoleFiller::HoleFiller() : Module("HoleFiller", "Fill obsidian and bedrock hole to anger them", Category::COMBAT) {
	addSlider<float>("Target Range", "If a players distance is greater than this then we wont consider them a potential target", ValueType::FLOAT_T, &targetRange, 1.f, 20.f);
	addSlider<float>("Place Range", "If a placement's distance is greater than this then we will look for another placement in the placelist", ValueType::FLOAT_T, &placeRange, 1.f, 20.f);
	addSlider<float>("Place Speed", "Amount of times we try to place in a second(20 = place every tick)", ValueType::FLOAT_T, &placeSpeed, 1.f, 20.f);
	addSlider<float>("Place Proximity", "NULL", ValueType::FLOAT_T, &placeProximity, 1.f, 12.f);
	addBoolCheck("Smart", "Only fills holes if target is near it", &smart);
	addBoolCheck("Safe", "Only fills holes if you are in a hole", &safeMode);
	addBoolCheck("Eat Stop", "Stop on eat", &eatStop);
	addBoolCheck("Rotate", "Rotate to where ur placing", &rotate);
	addEnumSetting("Switch", "NULL", { "None", "Normal", "Spoof", "Packet" }, &switchType);
}

void HoleFiller::onEnable() {
	targetList.clear();
	placeList.clear();
}

void HoleFiller::onDisable() {
	targetList.clear();
	placeList.clear();
}

int HoleFiller::getBlockSlot() {
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		if (stack->isValid()) {
			if (stack->getItemPtr()->itemId == 49) return i;
		}
	} return -1;
}

bool HoleFiller::isValidHole(Vec3<int> pos) {
	if (pos.dist(mc.getLocalPlayer()->getHumanPos()) > placeRange) return false;
	if (!BlockUtils::isReplaceable(pos)) return false;
	if (!BlockUtils::isReplaceable(pos.add(0, 1, 0))) return false;
	if (!BlockUtils::isReplaceable(pos.add(0, 2, 0))) return false;
	std::vector<Vec3<int>> checkList = { {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };
	for (Vec3<int> check : checkList) if (BlockUtils::getBlockId(pos.add(check)) != 7 && BlockUtils::getBlockId(pos.add(check)) != 49 && !BlockUtils::isContainer(pos.add(check))) return false;
	return true;
}

void HoleFiller::checkForHoles(Actor* actor) {
	int range = placeProximity;
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			for (int z = -range; z <= range; z++) {
				Vec3<float> targetPos = actor->getPosition()->floor().add(0, -1, 0);
				Vec3<int> blockPos((int)targetPos.x + x, (int)targetPos.y + y, (int)targetPos.z + z);
				if (isValidHole(blockPos)) {
					if (BlockUtils::canBuildBlock(blockPos)) {
						if (smart && isValidHole(targetPos.toInt())) return; // targets in a hole lets just calculate all the holes so we are ready onces he comes out
						placeList.push_back(blockPos);
					}
				}
			}
		}
	}
}

void HoleFiller::fillHole() {
	Vec3<float> playerPos = mc.getLocalPlayer()->getPosition()->floor(); playerPos.y -= 1.f;
	if (placeList.empty()) return;
	if (!isValidHole(playerPos.toInt()) && safeMode) return;
	if (placeIncr >= (20 - placeSpeed)) {
		for (Vec3<int> place : placeList) {
			if (BlockUtils::isReplaceable(place)) {
				BlockUtils::buildBlock(place);
			}
		}
		if (BlockUtils::isReplaceable(placeList[0])) BlockUtils::buildBlock(placeList[0]);
		placeIncr = 0;
	}
	else placeIncr++;
}

void HoleFiller::onNormalTick(Actor* actor) {
	LocalPlayer* localPlayer = (LocalPlayer*)actor;
	PlayerInventory* supplies = localPlayer->getPlayerInventory();
	if (localPlayer == nullptr) return;
	targetList.clear();
	placeList.clear();
	if (eatStop && localPlayer->getItemUseDuration() > 0) return;
	if (smart) {
		for (Actor* entity : localPlayer->getLevel()->getRuntimeActorList()) {
			if (!TargetUtils::isTargetValid(entity)) continue;
			if (entity->getEyePos().dist(localPlayer->getEyePos()) > targetRange) continue;
			targetList.push_back(entity);
		}
	}
	else targetList.push_back(localPlayer);
	if (targetList.empty()) return;
	for (auto& target : targetList) checkForHoles(target);
	int oldSlot = supplies->selectedSlot;
	if (switchType != 0) {
		if (switchType != 3) supplies->selectedSlot = getBlockSlot();
		else {
			MobEquipmentPacket pk(localPlayer->getRuntimeID(), localPlayer->getPlayerInventory()->inventory->getItemStack(getBlockSlot()), getBlockSlot(), getBlockSlot());
			mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
		}
	}
	fillHole();
	if (switchType == 2) supplies->selectedSlot = oldSlot;
	else if (switchType == 3) {
		MobEquipmentPacket pk(localPlayer->getRuntimeID(), localPlayer->getPlayerInventory()->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
	}
}

void HoleFiller::onSendPacket(Packet* packet, bool& shouldCancel) {
	if (rotate && !placeList.empty()) {
		Vec2<float> angle;
		for (Vec3<int> place : placeList) angle = mc.getLocalPlayer()->getPosition()->CalcAngle(place.toFloat());
		if (packet->getId() == PacketID::PlayerAuthInput) {
			PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
			authPkt->rotation = angle;
			authPkt->headYaw = angle.y;
		}
		if (packet->getId() == PacketID::MovePlayerPacket) {
			MovePlayerPacket* movePkt = (MovePlayerPacket*)packet;
			movePkt->rotation = angle;
			movePkt->headYaw = angle.y;
		}
	}
}
