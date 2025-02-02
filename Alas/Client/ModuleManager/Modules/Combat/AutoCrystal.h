#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

struct CrystalFade {
	Vec3<float> lastPos;
	float fadeTimer;
	float fadeDuration;
};

struct CrystalUtil {
public:
	Actor* targetEntity;
	float targetDamage;
	float localDamage;
protected:
	float getExplosionDamage(const Vec3<float>& crystalPos, Actor* target);
};

class PlaceUtils : public CrystalUtil {
public:
	Vec3<int> placePos;
	PlaceUtils(Vec3<int> pos, Actor* _target) {
		LocalPlayer* lp = mc.getLocalPlayer();
		placePos = pos;
		Vec3<float> crystalPos = pos.toFloat().add(0.5f, 1.f, 0.5f);
		targetDamage = getExplosionDamage(crystalPos, _target);
		localDamage = getExplosionDamage(crystalPos, lp);
		targetEntity = _target;
	}
};

class BreakUtils : public CrystalUtil {
public:
	Actor* endCrystal;
	BreakUtils(Actor* crystal, Actor* _target) {
		LocalPlayer* lp = mc.getLocalPlayer();
		endCrystal = crystal;
		Vec3<float> crystalPos = *crystal->getPosition();
		targetDamage = getExplosionDamage(crystalPos, _target);
		localDamage = getExplosionDamage(crystalPos, lp);
		targetEntity = _target;
	}
};
class AutoCrystal : public Module {
public:
	std::vector<PlaceUtils> placeList;
	std::vector<BreakUtils> breakList;
	std::vector<Actor*> targetList;
	std::vector<Actor*> entityList;
	std::vector<CrystalFade> fadeList;
public:
	bool placeCrystal = true;
	bool explodeCrystal = true;
	bool idPredict = false;
	bool eatCheck = true;
	bool antiWeakness = false;
	bool extrapolation = true;
	bool swing = true;
	bool rotate = false;
	bool setDead = false;
	int placeAmount = 1;
	float targetRange = 12.f;
	float maxY = 5.f;
	float placeRange = 6.f;
	float breakRange = 6.f;
	float placeProximity = 12.f;
	int protocol = 0;
	int switchType = 3;
	float localDamagePlace = 20.f;
	float enemyDamagePlace = 4.f;
	float localDamageBreak = 20.f;
	float enemyDamageBreak = 4.f;
	float extrapolateAmount = 0.5f;
	float fadeDur = 1.f;
	int breakType = 0;
	int placeDelay = 0;
	int explodeDelay = 0;
	int boostDelay = 0;
	int IplaceDelay = 0;
	int IexplodeDelay = 0;
	int IboostDelay = 0;
	int highestId = -1;
	bool shouldChange = false;
	int idPacket = 1;
	bool renderDamage = true;
	bool render = true;
	bool render2d = false;
	int alpha = 50;
	int lineAlpha = 70;
	bool fade = false;
	bool test = false;
public:
	int getEndCrystal();
	int getBestItem();
	static bool sortCrystal(CrystalUtil a1, CrystalUtil a2);
	bool isPlaceValid(const Vec3<int>& placePos, Actor* target);
	void generatePlacements(Actor* target);
	void getCrystalList(Actor* target);
	void placeEndCrystal(GameMode* gm);
	void explodeEndCrystal(GameMode* gm);
	void predictEndCrystal(GameMode* gm);
public:
	AutoCrystal();
	virtual std::string getModName() override;
	virtual void onNormalTick(Actor* randomActorIDFK) override;
	virtual void onSendPacket(Packet* packet, bool& shouldCancel) override;
	virtual void onRender(MinecraftUIRenderContext* renderCtx);
	virtual void onImGuiRender(ImDrawList* d) override;
	virtual void onEnable();
	virtual void onDisable();
};
