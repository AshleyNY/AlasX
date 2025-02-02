#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class PistonAura : public Module {
public:
	std::vector<Vec3<int>> pistonList;
	std::vector<Vec3<int>> redstoneList;
	std::vector<Vec3<int>> crystalList;
public:
	std::vector<Actor*> targetList;
	std::vector<Actor*> entityList;
public:
	int realYaw = 0;
public:
	float range = 6.f;
	int maxHeight = 3;
	int rots = 0;
	bool airplace = true;
	bool java = true;
	bool self = false;
	bool eatStop = false;
	int crystalDelay = 0;
	int pistonDelay = 0;
	int redstoneDelay = 0;
	int attackDelay = 0;
	int icrystalDelay = -999;
	int ipistonDelay = -999;
	int iredstoneDelay = -999;
	int iattackDelay = -999;
public:
	bool isPistonValid(Vec3<int> place);
	bool isRedstoneValid(Vec3<int> place);
	bool isCrystalValid(Vec3<int> placePos);
	bool isGoodPosition(Vec3<int> pistonPos, Vec3<int> redstonePos, Vec3<int> crystalPos);
	void generatePlacement(Actor* actor);
	int getPiston();
	int getItem(int id);
	void placePiston(GameMode* gm);
	void placeRedstone(GameMode* gm);
	void placeCrystal(GameMode* gm);
public:
	PistonAura();
	virtual void onNormalTick(Actor* localPlayer) override;
	virtual void onRender(MinecraftUIRenderContext* renderCtx) override;
	virtual void onSendPacket(Packet* packet, bool& cancel) override;
	virtual void onEnable();
	virtual void onDisable();
};