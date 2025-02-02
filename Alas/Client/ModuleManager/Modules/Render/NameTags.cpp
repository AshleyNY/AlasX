#include "NameTags.h"
#include "../../../Client.h"

float tagSize = 0.70f;
UIColor friendColor = (0, 255, 255, 255);
NameTags::NameTags() : Module("NameTags", "Better nametags.", Category::RENDER) {
	addSlider<float>("Size", "NULL", ValueType::FLOAT_T, &tagSize, 0.f, 1.f);
	addSlider<float>("Opacity", "NULL", ValueType::FLOAT_T, &opacity, 0.f, 1.f);
	addBoolCheck("Self", "Render urself", &showSelf);
	addBoolCheck("Underline", "Show a line under", &underline);
	addBoolCheck("Dynamic Size", "NULL", &dynamicSize);
	addColorPicker("Friend", "NULL", &friendColor);
}

void NameTags::onImGuiRender(ImDrawList* d) {
	LocalPlayer* lp = mc.getLocalPlayer();
	if (!lp) return;
	if (!lp->getLevel()) return;
	if (!mc.canUseMoveKeys()) return;
	if (lp->getLevel()->getRuntimeActorList().empty()) return;
	if (mc.getClientInstance() == nullptr) return;
	if (mc.getGameMode() == nullptr) return;
	static Colors* colorsMod = (Colors*)client->moduleMgr->getModule("Colors");
	UIColor mainColor = colorsMod->getColor();
	for (Actor* actor : lp->getLevel()->getRuntimeActorList()) {
		if (TargetUtils::isTargetValid(actor, false) || (actor == lp && showSelf)) {
			Vec2<float> pos;
			if (ImGuiUtils::worldToScreen(actor->getEyePos().add(0.f, 0.75f, 0.f), pos)) {
				std::string name = *actor->getNameTag();
				float textSize = 1.f * tagSize;
				float textWidth = ImGuiUtils::getTextWidth(name, textSize);
				float textHeight = ImGuiUtils::getTextHeight(textSize);
				float textPadding = 1.f * textSize;
				Vec2<float> textPos = Vec2<float>(pos.x - textWidth / 2.f, pos.y - textHeight / 2.f);
				Vec4<float> rectPos = Vec4<float>(textPos.x - textPadding * 2.f,
					textPos.y - textPadding,
					textPos.x + textWidth + textPadding * 2.f,
					textPos.y + textHeight + textPadding);
				Vec4<float> underlineRect = Vec4<float>(rectPos.x, rectPos.w - 1.f * textSize, rectPos.z, rectPos.w);
				ImGuiUtils::fillRectangle(rectPos, UIColor(0, 0, 0, (int)(255 * opacity)));
				if (underline) ImGuiUtils::fillRectangle(underlineRect, UIColor(255, 255, 255, 255));
ImGuiUtils::drawText(textPos, name, UIColor(255, 255, 255, 255), textSize, true);
		
			}
		}
	}
}
int waitForInit = 0;
void NameTags::onRender(MinecraftUIRenderContext* renderCtx) {
	if (!mc.getLocalPlayer()) return;
	if (!mc.getLocalPlayer()->getLevel()) return;
	if (!mc.canUseMoveKeys()) return;
	if (mc.getLocalPlayer()->getLevel()->getRuntimeActorList().empty()) return;
	if (mc.getClientInstance() == nullptr) return;
	if (mc.getGameMode() == nullptr) return;
	if (waitForInit >= 20) {
		for (Actor* actor : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
			if (TargetUtils::isTargetValid(actor, false) || (actor == mc.getLocalPlayer() && showSelf)) {
				Vec2<float> pos;
				if (RenderUtils::worldToScreen(actor->getEyePos().add(0.f, 0.75f, 0.f), pos)) {
					std::string name = *actor->getNameTag();
					float textSize = 1.f * tagSize;
					float textWidth = ImGuiUtils::getTextWidth(name, textSize);
					float textHeight = ImGuiUtils::getTextHeight(textSize);
					float textPadding = 1.f * textSize;
					Vec2<float> textPos = Vec2<float>(pos.x - textWidth / 2.f, pos.y - textHeight / 2.f);
					Vec4<float> rectPos = Vec4<float>(textPos.x - textPadding * 2.f,
						textPos.y - textPadding,
						textPos.x + textWidth + textPadding * 2.f,
						textPos.y + textHeight + textPadding);
					if (showItems) {
						float scale = tagSize * 0.75f;
						float spacing = scale + 15.f;
						int x = 0;
						int y = rectPos.y - 5.f;
						if (actor->getOffhandSlot() != nullptr && actor->getOffhandSlot()->isValid()) x += scale * spacing;
						for (int i = 0; i < 4; i++) {
							ItemStack* armor = actor->getArmor(i);
							if (armor->isValid() && armor != nullptr) {
								x += scale * spacing;
							}
						}
						ItemStack* selected = actor->getCarriedItem();
						if (selected->isValid() && selected != nullptr) x += scale * spacing;
						float xReal = (rectPos.x + rectPos.z - x) / 2;
						if (actor->getOffhandSlot()->isValid() && actor->getOffhandSlot() != nullptr) {
							ItemStack* offhand = actor->getOffhandSlot();
							RenderUtils::drawItem(offhand, Vec2<float>(xReal, y), 1.f, scale);
							RenderUtils::drawItemDurability(offhand, Vec2<float>(xReal, y), scale, 1.f, 2);
							if (offhand->stackCount > 1) {
								RenderUtils::drawCenteredText(Vec2<float>(xReal + 10.f, y + 1.5f), std::to_string((int)offhand->stackCount), MC_Color(1.f, 1.f, 1.f), scale, 1.f);
							}
							xReal += scale * spacing;
						}
						for (int i = 0; i < 4; i++) {
							ItemStack* armor = actor->getArmor(i);
							if (armor->isValid() && armor != nullptr) {
								RenderUtils::drawItem(armor, Vec2<float>(xReal, y), 1.f, scale);
								RenderUtils::drawItemDurability(armor, Vec2<float>(xReal, y), scale, 1.f, 2);
								if (armor->stackCount > 1) {
									RenderUtils::drawCenteredText(Vec2<float>(xReal + 10.f, y + 1.5f), std::to_string((int)armor->stackCount), MC_Color(255.f, 255.f, 255.f), scale, 1.f);
								}
								xReal += scale * spacing;
							}
						}
						if (selected->isValid() && selected != nullptr) {
							RenderUtils::drawItem(selected, Vec2<float>(xReal, y), 1.f, scale);
							RenderUtils::drawItemDurability(selected, Vec2<float>(xReal, y), scale, 1.f, 2);
							if (selected->stackCount > 1) {
								RenderUtils::drawText(Vec2<float>(xReal + 10.f, y + 1.5f), std::to_string((int)selected->stackCount), MC_Color(255.f, 255.f, 255.f), scale, 1.f);
							}
						}
					}
				}
			}
		}
	}
	else waitForInit++;
}