#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class Killaura : public Module {
public:
    int packetID = -1;
    bool packetAttack = false;
    bool shouldChangeID = false;
public:
    std::vector<Actor*> targetList;
    Vec2<float> angle;
public:
    int attackPackets = 1;
    int hitAttempts = 1;
    int attackDelay = 0;
    int attackIncr = 0;
    int maxTargets = 0;
    float alpha = 0.25f;
    float targetRange = 12.f;
    float attackRange = 6.f;
    int switchType = 0;
    int rot = 0;
    int strafeType = 0;
protected:
    void forEachEntity();
protected:
    void attack(Actor* target);
    int getItemSlot();
    void attackByPacket(Actor* target);
public:
    Killaura();
    virtual void onNormalTick(Actor* actor);
    virtual void onSendPacket(Packet* packet, bool& shouldCancel);
    virtual void onRender(MinecraftUIRenderContext* renderCtx) override;
    virtual void onEnable();
    virtual void onDisable();
};