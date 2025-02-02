#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class AutoAnvil : public Module {
public:
	std::vector<Actor*> targetList;
public:
	int placedelay = 1;
	int tickPlace = 0;
	float targetRange = 12.f;
	bool airPlace = false;
	bool onlyOnce = false;
	int block = 0;
protected:
	int getItemSlot(int itemId);
public:
	AutoAnvil();
	virtual void onNormalTick(Actor* lp) override;
};