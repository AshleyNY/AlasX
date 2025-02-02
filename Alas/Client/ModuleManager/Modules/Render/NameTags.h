#pragma once
#include "../Module.h"

class NameTags : public Module {
private:
	float opacity = 0.4f;
	bool showPops = false;
	bool showItems = true;
	bool showDura = true;
	bool showSelf = true;
	bool underline = true;
	bool dynamicSize = false;
public:
	NameTags();
	virtual void onImGuiRender(ImDrawList* d) override;
	virtual void onRender(MinecraftUIRenderContext* renderCtx) override;
};