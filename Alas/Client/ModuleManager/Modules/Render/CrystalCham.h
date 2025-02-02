#pragma once
#include "../Module.h"

class CrystalCham : public Module {
private:
	std::vector<Actor*> CrystalList;

	bool circle = true;
	bool circlefill = false;
	UIColor color = UIColor(82, 138, 240, 200);
	UIColor CirColor = UIColor(112, 0, 255, 255);
public:

	float x1 = 0;
	float y1 = 90;
	float z1 = 180;
	float x2 = 270;
	float ex1 = 45;
	float ex2 = 135;
	float ex3 = 225;
	float ex4 = 315;
	float size = 0.35f;
	float movingY = 100;
	float movingT = 4;
	float Speed = 400;
	float height = 0.40f;
	bool CaCham = false;
	CrystalCham();
	Vec3<float> sb = (0,0,0);
	virtual void onRender(MinecraftUIRenderContext* ctx) override;
	virtual void onImGuiRender(ImDrawList* d);
	virtual void onLevelRender()override;
	virtual void onEnable()override;
	virtual void onDisable()override;
};