#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class Fall : public Module {
public:
	Fall();
	virtual void onNormalTick(Actor* actor) override;
};