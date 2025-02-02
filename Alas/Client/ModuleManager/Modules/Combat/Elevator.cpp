#include "Elevator.h"
#include "../../../../Utils/BlockUtils.h"

Elevator::Elevator() : Module("Elevator", "Pushes them out of their hole", Category::COMBAT) {
	addSlider<float>("Range", "NULL", ValueType::FLOAT_T, &range, 0.f, 12.f);
	addBoolCheck("Airplace", "air on place.. place on air", &airplace);
	addSlider<int>("Piston Delay", "NULL", ValueType::INT_T, &pistonDelay, 0, 25);
	addSlider<int>("Redstone Delay", "NULL", ValueType::INT_T, &redstoneDelay, 0, 25);
	addBoolCheck("Self", "self on test", &selfTest);
}

int Elevator::getItem(int id) {
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		if (stack->isValid() && stack != nullptr) {
			if (id != 0 && stack->getItemPtr()->itemId == id) return i;
			if (id == 0 && (stack->getItemPtr()->itemId == 33 || stack->getItemPtr()->itemId == 29)) return i;
		}
	} return -1;
}

bool Elevator::isValid(ElevatorPlace placement) {
	Vec3<float> pistonPos = placement.pistonPos;
	Vec3<float> redstonePos = placement.redstonePos;
	if (!BlockUtils::isReplaceable(pistonPos) && (BlockUtils::getBlockType(pistonPos) != BLOCKTYPE::PISTON_NORMAL && BlockUtils::getBlockType(pistonPos) != BLOCKTYPE::PISTON_STICKY)) return false;
	if (!BlockUtils::isReplaceable(redstonePos) && BlockUtils::getBlockType(redstonePos) != BLOCKTYPE::REDSTONE_BLOCK) return false;
	if (!BlockUtils::canBuildBlock(pistonPos, airplace)) return false;
	if (!BlockUtils::canBuildBlock(redstonePos, airplace)) return false;
	if (pistonPos == Vec3<float>(0, 0, 0)) return false;
	if (redstonePos == Vec3<float>(0, 0, 0)) return false;
	if (pistonPos == redstonePos) return false;
	return true;
}

ElevatorPlace Elevator::getPlacement(Actor* actor) {
	Vec3<float> actorPos = actor->getPosition()->floor(); actorPos.y -= 1.f;
	ElevatorPlace place = ElevatorPlace(Vec3<float>(0, 0, 0), Vec3<float>(0, 0, 0));
	for (ElevatorPlace check : checkList) {
		ElevatorPlace pos = ElevatorPlace(actorPos.add(check.pistonPos.add(0, 1, 0)), actorPos.add(check.redstonePos.add(0, 1, 0)));
		if (isValid(pos)) {
			place = pos;
			break;
		}
	}
	Vec3<float> yawPos = Vec3<float>(actorPos.x - place.pistonPos.x, 0.f, actorPos.z - place.pistonPos.z);
	if (yawPos == Vec3<float>(1, 0, 0)) realYaw = 90;
	if (yawPos == Vec3<float>(-1, 0, 0)) realYaw = -90;
	if (yawPos == Vec3<float>(0, 0, 1)) realYaw = 180;
	if (yawPos == Vec3<float>(0, 0, -1)) realYaw = 0;
	return place;
}

void Elevator::onNormalTick(Actor* lp) {
	targetList.clear();
	if (!mc.passedNullCheck()) return;
	PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	for (Actor* actor : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (!TargetUtils::isTargetValid(actor)) continue;
		if (actor->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > range) continue;
		targetList.push_back(actor);
	}
	if (selfTest) targetList.push_back(mc.getLocalPlayer());
	int oldSlot = plrInv->selectedSlot;
	if (!targetList.empty()) {
		PlayerAuthInputPacket authPkt;
		authPkt.rotation = Vec2<float>(0, realYaw);
		mc.getClientInstance()->loopbackPacketSender->sendToServer(&authPkt);

		ElevatorPlace placePos = getPlacement(targetList[0]);
		MobEquipmentPacket old(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(oldSlot), oldSlot, oldSlot);
		MobEquipmentPacket piston(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(0)), getItem(0), getItem(0));
		MobEquipmentPacket redstone(mc.getLocalPlayer()->getRuntimeID(), inv->getItemStack(getItem(152)), getItem(152), getItem(152));
		if (isValid(placePos)) {
			if (iPistonDelay == -999) iPistonDelay = 0;
			if (iPistonDelay >= pistonDelay) {
				plrInv->selectedSlot = getItem(0);
				mc.getClientInstance()->loopbackPacketSender->send(&piston);
				if (mc.passedItemNullCheck()) {
					if (mc.getLocalPlayer()->getCarriedItem()->getItemPtr()->itemId == 33 || mc.getLocalPlayer()->getCarriedItem()->getItemPtr()->itemId == 29) {
						BlockUtils::buildBlock(placePos.pistonPos, airplace);
					}
				}
				iPistonDelay = 0;
			}
			else iRedstoneDelay++;
			if (iRedstoneDelay == -999) iRedstoneDelay = 0;
			if (iRedstoneDelay >= redstoneDelay) {
				plrInv->selectedSlot = getItem(152);
				mc.getClientInstance()->loopbackPacketSender->send(&redstone);
				if (mc.passedItemNullCheck()) {
					if (mc.getLocalPlayer()->getCarriedItem()->getItemPtr()->itemId == 152) {
						BlockUtils::buildBlock(placePos.redstonePos, airplace);
					}
				}
				iRedstoneDelay = 0;
			}
			else iRedstoneDelay++;
		}
		plrInv->selectedSlot = oldSlot;
		mc.getClientInstance()->loopbackPacketSender->send(&old);
	}
}

void Elevator::onEnable() {
	targetList.clear();
}

void Elevator::onDisable() {
	targetList.clear();
}