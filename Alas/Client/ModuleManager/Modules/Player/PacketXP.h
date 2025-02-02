#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class PacketXP : public Module {
public:
    int speed = 1;
    int rotation = 0;
    int mode = 0;
    int swap = 0;
    bool whileEating = false;
    bool feetPlace = false;
    int clickMode = 0;
public:
    int getItem(int id);
public:
    virtual void onNormalTick(Actor* actor) override;
    PacketXP();
};