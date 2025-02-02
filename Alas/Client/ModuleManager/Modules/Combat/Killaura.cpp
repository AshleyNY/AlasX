#include "Killaura.h"

Killaura::Killaura() : Module("Killaura", "Attack entities around you", Category::COMBAT) {
	addSlider<float>("Target Range", "Range we will get targetlist", ValueType::FLOAT_T, &targetRange, 1.f, 20.f);
	addSlider<float>("Attack Range", "Range we will attack targets", ValueType::FLOAT_T, &attackRange, 1.f, 20.f);
	addSlider<int>("Attack Delay", "Delay between each attack!", ValueType::INT_T, &attackDelay, 0, 25);
	addSlider<int>("Max Targets", "Maximum targets we will attack", ValueType::INT_T, &maxTargets, 0, 25);
	addSlider<int>("Hit Attempts", "Amount of times we will attack them in a single tick", ValueType::INT_T, &hitAttempts, 0, 50);
	addSlider<int>("Packets", "Amount of packet we will send to packet attack", ValueType::INT_T, &attackPackets, 0, 25);
	addBoolCheck("Packet Attack", "We attack using packets", &packetAttack);
	addEnumSetting("WeatherSwap", "How we swap to best weapon", { "None", "Regular", "Spoof", "Packet" }, &switchType);
	addEnumSetting("Rotation", "How we rotate", { "None", "Regular", "Packet" }, &rot);
	addEnumSetting("Strafe", "How we strafe", { "None", "Full", "Semi" }, &strafeType);
	addSlider<float>("Alpha", "Opacity of renders", ValueType::FLOAT_T, &alpha, 0.f, 1.f);
}

void Killaura::onEnable() {
	targetList.clear();
}

void Killaura::onDisable() {
	targetList.clear();
}

void Killaura::forEachEntity() {
	for (Actor* actor : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (!TargetUtils::isTargetValid(actor)) continue;
		if (actor->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > targetRange) continue;
		targetList.push_back(actor);
	}
}

void Killaura::attack(Actor* target) {
	GameMode* gm = mc.getGameMode();
	if (target->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > attackRange) return;
	for (int i = 0; i < hitAttempts; i++) {
		gm->attack(target);
		mc.getLocalPlayer()->swing();
	}
}
int Killaura::getItemSlot() {
	auto localPlayer = mc.getLocalPlayer();
	//float health = mc.getLocalPlayer()->getHealth();
	bool isInWater = mc.getLocalPlayer()->isInWater();
	if (!localPlayer)
		return -1;

	auto plrInv = localPlayer->getPlayerInventory();
	if (!plrInv)
		return -1;

	auto inv = plrInv->inventory;
	if (!inv)
		return -1;

	float damage = 0.f;
	int slot = plrInv->selectedSlot;

	for (int i = 0; i < 36; i++) {//背包里面也切
		auto itemStack = inv->getItemStack(i);
		if (itemStack && itemStack->isValid()) { // or you can do itemStack->getItemPtr()->hasRecipeTag("minecraft:is_sword") for sword find only 
			if (isInWater) {
				if (itemStack->getItemPtr()->itemId == 554) {
					slot = i;
				}
			}
			else {
				if (itemStack->getItemPtr()->itemId == 319) {
					slot = i;
				}
			}
		}
	}
	return slot;
}
/*int Killaura::getItemSlot(int itemId) {
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
}*/

void Killaura::attackByPacket(Actor* target) {
	return; // BROKEN
	GameMode* gm = mc.getGameMode();
	if (target->getEyePos().dist(mc.getLocalPlayer()->getEyePos()) > attackRange) return;
	shouldChangeID = true;
	if (*(int*)target->getRuntimeID() > packetID) packetID = *(int*)target->getRuntimeID();
	packetID = *(int*)target->getRuntimeID();
	for (int i = 0; i < attackPackets; i++) {
		InteractPacket inter(InteractAction::LEFT_CLICK, target->getRuntimeID(), target->stateVectorComponent->pos.sub(Vec3<int>(0.f, 0.2f, 0.f)));
		gm->attack(target);
		packetID++;
	}
	packetID -= attackPackets;
	shouldChangeID = false;
}

void Killaura::onNormalTick(Actor* actor) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	GameMode* gm = localPlayer->getGameMode();
	auto* plrInv = localPlayer->getPlayerInventory();
	auto* inv = plrInv->inventory;
	targetList.clear();
	forEachEntity();
	if (!targetList.empty()) {
		std::sort(targetList.begin(), targetList.end(), TargetUtils::sortByDist);
		int m = 0;
		int oldSlot = plrInv->selectedSlot;
		if (switchType != 0) {
			plrInv->selectedSlot = getItemSlot();
			if (switchType == 3) {
				MobEquipmentPacket pk(localPlayer->getRuntimeID(), plrInv->inventory->getItemStack(getItemSlot()), getItemSlot(), getItemSlot());
				mc.getLocalPlayer()->sendNetworkPacket(pk);
				mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
			}
		}
		angle = localPlayer->getPosition()->CalcAngle(*targetList[0]->getPosition());
		for (auto target : targetList) {
			if (attackIncr >= attackDelay) {
				attack(target);
				if (packetAttack) attackByPacket(target);
				attackIncr = 0;
			}
			else attackIncr++;
			m++;
			if (m >= maxTargets) break;
		}
		if (switchType == 2 || switchType == 3) {
			plrInv->selectedSlot = oldSlot;
			if (switchType == 3) {
				MobEquipmentPacket old(localPlayer->getRuntimeID(), plrInv->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
				mc.getLocalPlayer()->sendNetworkPacket(old);
				mc.getClientInstance()->loopbackPacketSender->sendToServer(&old);
			}
		}
	}
}

void Killaura::onRender(MinecraftUIRenderContext* renderCtx) {
	if (targetList.empty()) return;
	int m = 0;
	for (auto& target : targetList) {
		AABB mobAABB = *target->getAABB();
		RenderUtils::drawBox(mobAABB, UIColor(255, 0, 0, (int)alpha * 255), UIColor(255, 0, 0, (int)((alpha + 0.2f) * 255)), 0.3f, true, true);
		m++;
		if (m >= maxTargets) break;
	}
}

void Killaura::onSendPacket(Packet* packet, bool& shouldCancel) {
	if (targetList.empty()) return;
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	Vec2<float> angle2 = localPlayer->getPosition()->CalcAngle(*targetList[0]->getPosition());
	if (rot == 2) {
		if (packet->getId() == PacketID::PlayerAuthInput) {
			PlayerAuthInputPacket* authPkt = (PlayerAuthInputPacket*)packet;
			authPkt->rotation = angle2;
			authPkt->headYaw = angle2.y;
		}
		if (packet->getId() == PacketID::MovePlayerPacket) {
			auto* movePkt = (MovePlayerPacket*)packet;
			movePkt->rotation = angle2;
			movePkt->headYaw = angle2.y;
		}
	}
	if (rot == 1) {
		localPlayer->rotationComponent->rotation = angle2;
	}
	if (strafeType == 1) localPlayer->rotationComponent->Set(angle2);
	if (strafeType == 2) localPlayer->rotationComponent->Set(angle);
}