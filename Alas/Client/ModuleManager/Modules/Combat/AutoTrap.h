#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class AutoTrap : public Module {
public:
	std::vector<Actor*> entityList;
	std::vector<Actor*> targetList;
	std::vector<Vec3<int>> placeList;
protected:
	float range = 12.f;
	bool airplace = false;
	bool packetPlace = false;
	bool selfTest = false;
	int trapMode = 0;
	int trapDelay = 0;
	int iTrapDelay = 0;
	int swapMode = 2;
protected:
	int getItemSlot(int itemId);
	void getPlacement();
public:
	AutoTrap();
	virtual void onNormalTick(Actor* actor) override;
	virtual void onEnable();
	virtual void onDisable();
};