#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"
#include "../../../../Utils/DamageUtils.h"

struct AnchorFade {
	Vec3<float> lastPos;
	float fadeTimer;
	float fadeDuration;
};

struct AnchorStruct {
public:
	float targetDmg;
	float localDmg;
	Actor* entity;
};

class AnchorPlace : public AnchorStruct {
public:
	Vec3<int> placePos;
	AnchorPlace(Vec3<int> _placePos, Actor* _entity) {
		placePos = _placePos;
		targetDmg = DamageUtils::getExplosionDamage(_placePos.toFloat(), _entity, 0.f, 66063);
		localDmg = DamageUtils::getExplosionDamage(_placePos.toFloat(), mc.getLocalPlayer(), 0.f, 66063);
		entity = _entity;
	}
};

class AnchorAura : public Module {
public:
	std::vector<AnchorPlace> placeList;
	std::vector<AnchorPlace> explodeList;
	std::vector<Actor*> targetList;
	std::vector<Actor*> entityList;
	std::vector<AnchorFade> fadeList; // AA
public:
	// Place
	bool placeAnchor = true;
	bool airplace = false;
	float placeRange = 6.f;
	float placeProximity = 12.f;
	float localDamagePlace = 20.f;
	float enemyDamagePlace = 4.f;
	int anchorDelay = 0;
	int gsDelay = 0;
	int iAnchorDelay = 0;
	int iGsDelay = 0;
	// Explode
	bool explodeAnchor = true;
	float explodeRange = 6.f;
	float explodeProximity = 12.f;
	float localDamageExplode = 20.f;
	float enemyDamageExplode = 4.f;
	int explodeDelay = 0;
	int iExplodeDelay = 0;
	// Logic
	bool eatStop = true;
	bool self = false;
	int trapMode = 0;
	bool antiBlocker = false;
	float targetRange = 12.f;
	// Render
	bool renderDamage = true;
	bool render = true;
	bool render2d = false;
	float fadeDur = 1.f;
	int alpha = 50;
	int lineAlpha = 70;
	bool fade = false;
public:
	static bool sortByDamage(AnchorStruct a1, AnchorStruct a2);
	bool isPlaceValid(const Vec3<float>& placePos, Actor* target);
	bool isExplodeValid(const Vec3<float>& explodePos, Actor* target);
	void getPlaceList(Actor* target);
	void getExplodeList(Actor* target);
	int getItem(int id);
	void placeAnchors(GameMode* gm);
	void igniteAnchors(GameMode* gm);
	void explodeAnchors(GameMode* gm);
	void mineBlocker(Actor* actor);
	void trap(Actor* actor);
public:
	AnchorAura();
	virtual void onNormalTick(Actor* actor) override;
	virtual void onRender(MinecraftUIRenderContext* renderCtx) override;
	virtual void onEnable();
	virtual void onDisable();
};