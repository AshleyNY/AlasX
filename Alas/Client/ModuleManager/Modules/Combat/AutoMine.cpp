#include "AutoMine.h"
#include "../../../Client.h"
#include "../../../../Utils/BlockUtils.h"

AutoMine::AutoMine() : Module("AutoMine", "Mine their city automatically", Category::COMBAT) {
	addSlider<float>("Range", "Target range", ValueType::FLOAT_T, &targetRange, 0.f, 12.f);
	addBoolCheck("EatStop", "stopping on eating", &eatStop);
	addBoolCheck("Face Mine", "Mine their faceplec", &faceMine);
	addBoolCheck("Burrow Mine", "Mine their burrow", &burrowMine);
	addBoolCheck("Self Test", "Test on u", &selfTestt);
}

bool AutoMine::isBreakValid(Vec3<int> breakPos) {
	if (BlockUtils::isReplaceable(breakPos)) return false;
	if (BlockUtils::getBlockId(breakPos) == 26) return false;
	if (BlockUtils::getBlockId(breakPos) == 7) return false;
	return true;
}

bool AutoMine::sortByDistance(Actor* a1, Actor* a2) {
	Vec3<float> localPlayerPos = mc.getLocalPlayer()->getHumanPos();
	return a1->getHumanPos().dist(localPlayerPos) < a2->getHumanPos().dist(localPlayerPos);
}

Vec3<int> AutoMine::getBreakList(Actor* actor) {
	GameMode* gm = mc.getGameMode();
	float breakProgress = *(float*)((__int64)gm + 0x24);
	if (breakProgress != 0.f) return Vec3<int>(0, 0, 0);
	Vec3<float> floorPos = actor->getPosition()->floor();
	Vec3<int> actorPos = Vec3<int>(floorPos.x, floorPos.y - 1.f, floorPos.z);
	Vec3<float> tPos = *actor->getPosition();
	Vec3<float> blockPos = tPos - floorPos;
	float sumX = blockPos.x - 0.5;
	float sumZ = blockPos.z - 0.5;
	if (isBreakValid(actorPos.toInt()) && burrowMine) return Vec3<int>(actorPos);
	if (sumZ >= 0.2 && isBreakValid(actorPos.add(0, 0, 1))) return Vec3<int>(actorPos.add(0, 0, 1));
	if (sumZ <= -0.2 && isBreakValid(actorPos.add(0, 0, -1))) return Vec3<int>(actorPos.add(0, 0, -1));
	if (sumX >= 0.2 && isBreakValid(actorPos.add(1, 0, 0))) return Vec3<int>(actorPos.add(1, 0, 0));
	if (sumX <= -0.2 && isBreakValid(actorPos.add(-1, 0, 0))) return Vec3<int>(actorPos.add(-1, 0, 0));
	if (faceMine) {
		if (sumZ >= 0.2 && isBreakValid(actorPos.add(0, 1, 1))) return Vec3<int>(actorPos.add(0, 1, 1));
		if (sumZ <= -0.2 && isBreakValid(actorPos.add(0, 1, -1))) return Vec3<int>(actorPos.add(0, 1, -1));
		if (sumX >= 0.2 && isBreakValid(actorPos.add(1, 1, 0))) return Vec3<int>(actorPos.add(1, 1, 0));
		if (sumX <= -0.2 && isBreakValid(actorPos.add(-1, 1, 0))) return Vec3<int>(actorPos.add(-1, 1, 0));
	}
	std::vector<Vec3<int>> checkList = { {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1} };
	Vec3<int> pos;
	for (Vec3<int> check : checkList) {
		if (isBreakValid(actorPos.add(check))) {
			pos = actorPos.add(check);
			break;
		}
	}
	return pos;
}
void AutoMine::onNormalTick(Actor* lp) {
	LocalPlayer* localPlayer = (LocalPlayer*)lp;
	GameMode* gm = mc.getGameMode();
	PacketMine* packetMine = (PacketMine*)client->moduleMgr->getModule("PacketMine");
	if (!packetMine->isEnabled()) packetMine->setEnabled(true);
	bool b = false;
	targetList.clear();
	for (Actor* actor : localPlayer->getLevel()->getRuntimeActorList()) {
		if (!TargetUtils::isTargetValid(actor)) continue;
		if (actor->getEyePos().dist(localPlayer->getEyePos()) > targetRange) continue;
		targetList.push_back(actor);
	}
	if (selfTestt) targetList.push_back(localPlayer);
	if (!targetList.empty()) {
		std::sort(targetList.begin(), targetList.end(), sortByDistance);
		if (localPlayer->getItemUseDuration() > 0 && eatStop) return;
		Vec3<int> breakPos = getBreakList(targetList[0]);
		if (breakPos == Vec3<int>(0, 0, 0)) return;
		if (gm->destroyProgress == 0.f || packetMine->getBreakPos() != breakPos) {
			gm->startDestroyBlock(breakPos, 0, b);
			packetMine->setBreakPos(breakPos, 0);
		}
	}
}

void AutoMine::onDisable() {
	GameMode* gm = mc.getGameMode();
	PacketMine* packetMine = (PacketMine*)client->moduleMgr->getModule("PacketMine");
	if (gm != nullptr) *(float*)((__int64)gm + 0x24) = 0.f;
	packetMine->setBreakPos(Vec3<int>(0, 0, 0), 0);
}