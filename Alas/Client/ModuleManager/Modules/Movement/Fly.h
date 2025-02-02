#pragma once
#include "../Module.h"

class Fly : public Module {
public:
	float Speed = 4.f;
	float Speedy = 5.0f;
	float glideValue = 0.f;
	Fly();
	virtual void onNormalTick(Actor* actor) override;
};
