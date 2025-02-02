#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class HoleKicker : public Module {
public:
	std::vector<Vec3<int>> redstonePos;
	std::vector<Vec3<int>> placeList;
	std::vector<Actor*> targetList;
	std::vector<Actor*> entityList;
	float targetRange = 12.f;
	bool facePlaceOnly = false;
	bool selfTest = false;
	bool renderHoleKick = true;
	bool airplace = false;
	int redstoneDelay = 0;
	int pistonDelay = 0;
	int redstoneDelayIncr = 0;
	int pistonDelayIncr = 0;
	int realYaw = 0;
protected:
	static bool compareTarget(Actor* a1, Actor* a2);
	bool isPlaceValid(const Vec3<int>& pos, Actor* actor);
	void generatePlacement(Actor* actor);
	int getItem(int itemId);
public:
	HoleKicker();
	virtual void onNormalTick(Actor* lp) override;
	virtual void onSendPacket(Packet* packet, bool& shouldCancel) override;
	virtual void onRender(MinecraftUIRenderContext* renderCtx) override;
	virtual void onEnable();
	virtual void onDisable();
};