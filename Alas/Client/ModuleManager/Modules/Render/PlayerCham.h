#pragma once
#include "../Module.h"

class PlayerCham : public Module {
private:
	std::vector<Actor*> PlayerChamList;

	bool mobs = false;
	int renderMode = 0;
	UIColor color = UIColor(0, 255, 255, 50);
	UIColor lineColor = UIColor(0, 255, 255, 255);
public:
	PlayerCham();

	virtual void onRender(MinecraftUIRenderContext* ctx) override;
};