#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

struct ElevatorPlace {
public:
	Vec3<float> pistonPos;
	Vec3<float> redstonePos;
	ElevatorPlace(Vec3<float> piston, Vec3<float> redstone) {
		pistonPos = piston;
		redstonePos = redstone;
	}
};

class Elevator : public Module {
public:
	std::vector<Actor*> targetList;
	std::vector<ElevatorPlace> checkList = { // all possible holekicker placements
		{ElevatorPlace(Vec3<float>(1, 0, 0), Vec3<float>(2, 0, 0))}, {ElevatorPlace(Vec3<float>(-1, 0, 0), Vec3<float>(-2, 0, 0))},
		{ElevatorPlace(Vec3<float>(0, 0, 1), Vec3<float>(0, 0, 2))}, {ElevatorPlace(Vec3<float>(0, 0, -1), Vec3<float>(0, 0, -2))},
		{ElevatorPlace(Vec3<float>(1, 0, 0), Vec3<float>(1, 1, 0))}, {ElevatorPlace(Vec3<float>(-1, 0, 0), Vec3<float>(-1, 1, 0))},
		{ElevatorPlace(Vec3<float>(0, 0, 1), Vec3<float>(0, 1, 1))}, {ElevatorPlace(Vec3<float>(0, 0, -1), Vec3<float>(0, 1, -1))},
		{ElevatorPlace(Vec3<float>(1, 0, 0), Vec3<float>(1, 0, 1))}, {ElevatorPlace(Vec3<float>(-1, 0, 0), Vec3<float>(-1, 0, -1))},
		{ElevatorPlace(Vec3<float>(0, 0, 1), Vec3<float>(1, 0, 1))}, {ElevatorPlace(Vec3<float>(0, 0, -1), Vec3<float>(-1, 0, -1))},
		{ElevatorPlace(Vec3<float>(1, 0, 0), Vec3<float>(1, -1, 0))}, {ElevatorPlace(Vec3<float>(-1, 0, 0), Vec3<float>(-1, -1, 0))},
		{ElevatorPlace(Vec3<float>(0, 0, 1), Vec3<float>(0, -1, 1))}, {ElevatorPlace(Vec3<float>(0, 0, -1), Vec3<float>(0, -1, -1))},
	};
public:
	int realYaw = 0;
public:
	float range = 6.f;
	int pistonDelay = 0;
	int redstoneDelay = 0;
	bool selfTest = false;
	bool airplace = true;
public:
	int iRedstoneDelay = -999;
	int iPistonDelay = -999;
public:
	bool isValid(ElevatorPlace placement);
	ElevatorPlace getPlacement(Actor* actor);
	int getItem(int id);
public:
	Elevator();
	virtual void onNormalTick(Actor* lp) override;
	virtual void onEnable();
	virtual void onDisable();
};