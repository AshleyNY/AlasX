#include "Burrow.h"
#include "../../../Client.h"
#include "../../../../Utils/BlockUtils.h"

Burrow::Burrow() : Module("Burrow", "Burrow urself into blokc", Category::MOVEMENT) {
	addEnumSetting("Mode", "mode", { "Lagback", "Invalid Packet" }, &mode);
	addBoolCheck("Packet Place", "Use Packets to place for no ghost block", &packetPlace);
	addSlider<int>("Retry Time", "Amount of times we try to build", ValueType::INT_T, &retryTimes, 1, 5);
	addBoolCheck("Obsidian", "Use obsidian to burrow", &obsidian);
	addBoolCheck("Echest", "Use echest to burrow", &echest);
	addSlider<float>("Speed", "How fast we fly up to lagback ourselves", ValueType::FLOAT_T, &speed, 0.1f, 2.5f);
}

void Burrow::onEnable() {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	if (localPlayer == nullptr) return;
	Vec3<float> plrPos = localPlayer->getPosition()->floor(); plrPos.y -= 1.f;
	placePos = plrPos;
}

bool Burrow::isPlayerBurrowed(LocalPlayer* localPlayer) {
	AABB blockAABB = AABB(placePos, placePos.add(1.f, 1.f, 1.f));
	Vec3<float> plrPos = localPlayer->getPosition()->floor(); plrPos.y -= 1.f;
	if ((BlockUtils::getBlockId(placePos) == 49 || BlockUtils::getBlockId(placePos) == 130) && localPlayer->getAABB()->intersects(blockAABB) && plrPos == placePos) return true;
	return false;
}

void Burrow::onNormalTick(Actor* lp) {
	LocalPlayer* localPlayer = (LocalPlayer*)lp;
	PlayerInventory* plrInv = localPlayer->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	Burrow* burrow = (Burrow*)client->moduleMgr->getModule("Burrow");
	if (isPlayerBurrowed(localPlayer)) burrow->setEnabled(false);
	float yVelocity = 0;
	if (mode == 0) {
		yVelocity += speed;
		localPlayer->stateVectorComponent->velocity.y = yVelocity;
	}
	int oldSlot = plrInv->selectedSlot;
	int bestSlot = 0;
	for (int i = 0; i < 9; i++) {
		ItemStack* itemStack = inv->getItemStack(i);
		if (itemStack->isValid() && itemStack->getItemPtr()->itemId == 49 && obsidian && !echest) bestSlot = i;
		if (itemStack->isValid() && itemStack->getItemPtr()->itemId == 130 && !obsidian && echest) bestSlot = i;
		if ((itemStack->isValid() && (itemStack->getItemPtr()->itemId == 49 || itemStack->getItemPtr()->itemId == 130)) && obsidian && echest) bestSlot = i;
	}
	if (!packetPlace) plrInv->selectedSlot = bestSlot;
	MobEquipmentPacket obby(localPlayer->getRuntimeID(), plrInv->inventory->getItemStack(bestSlot), bestSlot, bestSlot);
	mc.getLocalPlayer()->sendNetworkPacket(obby);
	mc.getClientInstance()->loopbackPacketSender->sendToServer(&obby);
	for (int i = 0; i < retryTimes; i++) { /// Minty lesson: you can make a code be executed multiple amount of times by including it in a for loop like this, if retry time is at 3 it gets executed 3 times in a single tick
		BlockUtils::buildBlock(placePos, false);
	}
	if (!packetPlace) plrInv->selectedSlot = oldSlot;
	MobEquipmentPacket old(localPlayer->getRuntimeID(), plrInv->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
	mc.getLocalPlayer()->sendNetworkPacket(old);
	mc.getClientInstance()->loopbackPacketSender->sendToServer(&old);
}

void Burrow::onSendPacket(Packet* packet, bool& shouldCancel) {
	if (mode == 1) {
		mc.getLocalPlayer()->stateVectorComponent->velocity.y += 0.4f;
		if (packet->getId() == PacketID::MovePlayerPacket) {
			MovePlayerPacket* movePkt = (MovePlayerPacket*)packet;
			movePkt->position.y += speed;
			movePkt->onGround = false;
			movePkt->tick = 20;
		}
		if (packet->getId() == PacketID::PlayerAuthInput) {
			PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
			authPkt->position.y += speed;
			authPkt->downVelocity += speed;
			authPkt->ticksAlive = 20;
		}
	}
}