#include "BlockHighlight.h"
#include "../../../Client.h"

int outlineType = 0;
BlockHighlight::BlockHighlight() : Module("BlockHighlight", "Highlights blocks ur pointing at", Category::RENDER) {
	addEnumSetting("Mode", "NULL", { "3D", "2D" }, &mode);
	addEnumSetting("Outline", "NULL", { "Classic", "Hologram" }, &outlineType);
	addSlider<int>("Alpha", "Alpha of fill render", ValueType::INT_T, &alpha, 0, 255);
	addSlider<int>("Line Alpha", "Alpha of outline", ValueType::INT_T, &lineAlpha, 0, 255);
}

void BlockHighlight::onRender(MinecraftUIRenderContext* renderCtx) {
	if (!mc.getLocalPlayer()) return;
	if (!mc.getLocalPlayer()->getLevel()) return;
	if (!mc.canUseMoveKeys()) return;
	Colors* colorMod = (Colors*)client->moduleMgr->getModule("Colors");
	UIColor mainColor = colorMod->getColor();
	Vec3<int> blockPos = mc.getLocalPlayer()->getLevel()->getPointingBlockPos().toInt();
	AABB blockAABB = mc.getLocalPlayer()->makeAABB(&blockPos);
	if (mode == 0) RenderUtils::drawBox(blockAABB, UIColor(mainColor.r, mainColor.g, mainColor.b, alpha), UIColor(mainColor.r, mainColor.g, mainColor.b, lineAlpha), 0.3f, true, (bool)outlineType);
	else {
		int face = mc.getLocalPlayer()->getLevel()->getHitResult()->selectedFace;
		if (face == 0) blockAABB = AABB(blockPos.toFloat(), blockPos.toFloat().add(1.f).add(0.f, -0.99f, 0.f));
		else if (face == 1) blockAABB = AABB(blockPos.toFloat().add(0.f, 0.99f, 0.f), blockPos.toFloat().add(1.f));
		else if (face == 2) blockAABB = AABB(blockPos.toFloat(), blockPos.toFloat().add(1.f, 1.f, 0.f));
		else if (face == 3) blockAABB = AABB(blockPos.toFloat().add(0.f, 0.f, 1.f), blockPos.toFloat().add(1.f, 1.f, 1.f));
		else if (face == 4) blockAABB = AABB(blockPos.toFloat(), blockPos.toFloat().add(0.f, 1.f, 1.f));
		else if (face == 5) blockAABB = AABB(blockPos.toFloat().add(1.f, 0.f, 0.f), blockPos.toFloat().add(1.f, 1.f, 1.f));
		RenderUtils::drawBox(blockAABB, UIColor(mainColor.r, mainColor.g, mainColor.b, alpha), UIColor(mainColor.r, mainColor.g, mainColor.b, lineAlpha), 0.3f, true, (bool)outlineType);
	}
}