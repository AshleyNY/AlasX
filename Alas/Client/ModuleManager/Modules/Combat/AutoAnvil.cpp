#include "AutoAnvil.h"
#include "../../../../Utils/BlockUtils.h"

AutoAnvil::AutoAnvil() : Module("AutoAnvil", "Spam shit in they head", Category::PLAYER) {
	addBoolCheck("Airplace", "Place on air", &airPlace);
	addEnumSetting("Block", "Block Type", { "Anvil", "Sand", "Gravel", "Smart" }, &block);
	addSlider<float>("Range", "NULL", ValueType::FLOAT_T, &targetRange, 0.f, 10.f);
	addSlider<int>("Place Delay", "NULL", ValueType::INT_T, &placedelay, 0, 10);
	addBoolCheck("OnlyOnce", "Only places once", &onlyOnce);
}

int AutoAnvil::getItemSlot(int itemId) {
	PlayerInventory* supplies = mc.getLocalPlayer()->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	for (int i = 0; i < 9; i++) {
		ItemStack* stack = inv->getItemStack(i);
		unsigned int smartIds[] = { 145, 12, 13 };
		for (int smart : smartIds) {
			if (stack->isValid()) {
				if (itemId != 0 && stack->getItemPtr()->itemId == itemId) return i;
				if (itemId == 0 && stack->getItemPtr()->itemId == smart) return i;
			}
		}
	}
	return -1;
}

void AutoAnvil::onNormalTick(Actor* lp) {
	LocalPlayer* localPlayer = (LocalPlayer*)lp;
	GameMode* gm = mc.getGameMode();
	PlayerInventory* supplies = localPlayer->getPlayerInventory();
	Inventory* inv = supplies->inventory;
	targetList.clear();
	for (Actor* actor : localPlayer->getLevel()->getRuntimeActorList()) {
		if (!TargetUtils::isTargetValid(actor)) continue;
		if (actor->getEyePos().dist(localPlayer->getEyePos()) > targetRange) continue;
		targetList.push_back(actor);
	}
	if (targetList.empty()) return;
	int bestSlot = 0;
	if (block == 0) bestSlot = getItemSlot(145);
	if (block == 1) bestSlot = getItemSlot(12);
	if (block == 2) bestSlot = getItemSlot(13);
	if (block == 3) bestSlot = getItemSlot(0);
	int oldSlot = supplies->selectedSlot;
	MobEquipmentPacket pk(localPlayer->getRuntimeID(), inv->getItemStack(bestSlot), bestSlot, bestSlot);
	mc.getLocalPlayer()->sendNetworkPacket(pk);
	mc.getClientInstance()->loopbackPacketSender->send(&pk);
	for (Actor* target : targetList) {
		Vec3<float> targetPos = target->getPosition()->floor(); targetPos.y -= 1.f;
		Vec3<int> loc = targetPos.toInt().add(0, 2, 0);
		if (onlyOnce && BlockUtils::getBlockName(targetPos) != "air") return;
		if (tickPlace >= placedelay) {
			if (!airPlace) BlockUtils::buildBlock(loc);
			else gm->buildBlock(loc, Math::random(0, 5), false);
			tickPlace = 0;
		}
		else tickPlace++;
	}
	MobEquipmentPacket old(localPlayer->getRuntimeID(), inv->getItemStack(oldSlot), oldSlot, oldSlot);
	mc.getLocalPlayer()->sendNetworkPacket(old);
	mc.getClientInstance()->loopbackPacketSender->send(&old);
}