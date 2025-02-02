#pragma once
#include "../Module.h"

class Surround : public Module {
public:
	std::vector<Actor*> entityList;
public:
	bool disableComplete = false;
	bool center = false;
	bool jumpStop = true;
	bool onlyOnGround = false;
	bool airplace = false;
	bool packetPlace = false;
	bool echestFloor = false;
	bool attackCrystal = true;
	int maxExtend = 1;
	int switchMode = 0;
	int getItem(int id);
	bool doWeHaveItem(int id);
public:
	Surround();
	virtual void onEnable();
	virtual void onDisable();
	virtual void onNormalTick(Actor* actor) override;
};