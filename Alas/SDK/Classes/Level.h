#pragma once
#include "../../Utils/Math.h"
#include "../../Utils/MemoryUtils.h"

class Actor;
enum class HitResultType : int32_t
{
	BLOCK,
	ENTITY,
	ENTITY_OUT_OF_RANGE,
	AIR
};
class WeakEntityRef
{
	
};
class HitResult {
public:
	Vec3<float> liquidHitLocation;
	Vec3<float> raycastDirection;
	HitResultType type;
	int32_t selectedFace;
	Vec3<int> blockPos;
	Vec3<float> endPos;
	WeakEntityRef entity;
	bool isLiquid;
	char pad_004D[3];
	int32_t liquidFace;
	Vec3<int> liquidBlockPos;
	Vec3<float> liquidPos;
	bool indirectHit;
	char pad_006D[3];
};
class Level {
public:
	HitResult* getHitResult() { // reversed by MerciDiZep <3
		return reinterpret_cast<HitResult*>((uintptr_t)(this) + 0xA48);
	}
	Vec3<float> getPointingBlockPos() {
		Vec3<float> pos;
		pos.x = *reinterpret_cast<float*>((uintptr_t)(this) + 0xAE4);
		pos.y = *reinterpret_cast<float*>((uintptr_t)(this) + 0xAE8);
		pos.z = *reinterpret_cast<float*>((uintptr_t)(this) + 0xAEC);
		return pos;
	}
	void setHitResult(HitResultType type) {
		getHitResult()->type = type;
	}
	std::vector<Actor*> getRuntimeActorList() {
		std::vector<Actor*> listOut;
		MemoryUtils::CallVFunc<294, decltype(&listOut)>(this, &listOut);
		return listOut;
	}
};
