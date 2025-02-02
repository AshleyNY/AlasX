#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class SelfTrap : public Module {
public:
	std::vector<Actor*> targetList;
protected:
	bool smart = false;
	bool eatStop = true;
	float targetRange = 3.f;
	int mode = 0;
public:
	SelfTrap();
	virtual void onNormalTick(Actor* actor) override;
};