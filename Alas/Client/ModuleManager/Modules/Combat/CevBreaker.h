#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class CevBreaker : public Module {
public:
	std::vector<Vec3<int>> placeList;
	std::vector<Actor*> entityList;
	std::vector<Actor*> targetList;
public:
	bool javaMode = true;
	bool SideCev = true;
	bool airplace = false;
	bool packetPlace = false;
	bool self = false;
	bool silent = false;
	float targetRange = 12.f;
	float placeRange = 6.f;
	int placeDelay = 5;
	int destroyDelay = 0;
	int explodeDelay = 3;
	int switchBackDelay = 0;
	int IdestroyDelay = -999;
	int IplaceDelay = -999;
	int IexplodeDelay = -999;
	int IswitchBackDelay = -999;
public:
	bool isPlaceValid(Vec3<int> pos, Actor* target);
	Vec3<int> getBestPlace(Actor* target);
	int getItem(std::string name);
public:
	CevBreaker();
	virtual void onNormalTick(Actor* actor) override;
	virtual void onRender(MinecraftUIRenderContext* renderCtx) override;
	virtual void onDisable();
	virtual void onEnable();
};
