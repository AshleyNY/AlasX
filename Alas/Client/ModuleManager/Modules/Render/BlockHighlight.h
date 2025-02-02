#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"

class BlockHighlight : public Module {
public:
	int alpha = 50;
	int lineAlpha = 70;
	int mode = 0;
public:
	BlockHighlight();
	virtual void onRender(MinecraftUIRenderContext* renderCtx) override;
};