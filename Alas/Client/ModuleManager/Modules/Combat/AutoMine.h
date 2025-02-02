#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class AutoMine : public Module {
public:
	std::vector<Actor*> targetList;
protected:
	bool selfTestt = false;
	float targetRange = 10.f;
	bool eatStop = true;
	bool faceMine = false;
	bool burrowMine = false;
protected:
	static bool sortByDistance(Actor* a1, Actor* a2);
protected:
	Vec3<int> getBreakList(Actor* actor);
	bool isBreakValid(Vec3<int> breakPos);
public:
	AutoMine();
	virtual void onNormalTick(Actor* lp) override;
	virtual void onDisable();
};