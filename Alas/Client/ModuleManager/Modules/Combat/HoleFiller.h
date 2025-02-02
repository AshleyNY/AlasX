#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class HoleFiller : public Module {
protected:
	int switchType = 0;
	bool eatStop = true;
	bool smart = true;
	bool safeMode = true;
	float placeProximity = 6.f;
	bool anyBlock = false;
	bool rotate = true;
	float placeSpeed = 20.f;
	float placeRange = 6.f;
	float targetRange = 12.f;
	int placeIncr = 0;
public:
	std::vector<Vec3<int>> placeList;
	std::vector<Actor*> targetList;
protected:
	void checkForHoles(Actor* actor);
	void fillHole();
	bool isValidHole(Vec3<int> pos);
	int getBlockSlot();
public:
	HoleFiller();
	virtual void onNormalTick(Actor* actor) override;
	virtual void onSendPacket(Packet* packet, bool& shouldCancel) override;
	virtual void onEnable();
	virtual void onDisable();
};