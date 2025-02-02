#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class Burrow : public Module {
public:
	Vec3<float> placePos;
public:
	bool packetPlace = false;
	bool obsidian = true;
	bool echest = false;
	int mode = 0;
	int retryTimes = 1;
	float speed = 0.8f;
public:
	bool isPlayerBurrowed(LocalPlayer* localPlayer);
public:
	Burrow();
	virtual void onEnable();
	virtual void onNormalTick(Actor* lp) override;
	virtual void onSendPacket(Packet* packet, bool& shouldCancel) override;
};