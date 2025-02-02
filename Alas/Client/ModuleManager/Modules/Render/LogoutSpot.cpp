#include "LogoutSpot.h"

LogoutSpot::LogoutSpot() : Module("LogoutSpot", "Renders where an enemy logs off", Category::RENDER) {
	addBoolCheck("Clear Data", "Clear logout spots", &clearData);
}

void LogoutSpot::onImGuiRender(ImDrawList* d) {
	if (!mc.getLocalPlayer()) return;
	if (!mc.getClientInstance()) return;
	if (!mc.getLocalPlayer()->getLevel()) return;
	if (!mc.getGameMode()) return;
	if (clearData) {
		logoutData.clear();
		mc.DisplayClientMessage("%sAurora%s >> %sCleared LogoutSpot data!", AQUA, GOLD, GREEN);
		clearData = false;
	}
	if (mc.getLocalPlayer() == nullptr) return;
	if (mc.getClientInstance() == nullptr) return;
	if (mc.getLocalPlayer()->getLevel() == nullptr) return;
	for (Actor* actor : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (actor == nullptr) continue;
		if (actor == mc.getLocalPlayer()) continue;
		int entId = actor->getEntityTypeId();
		if (entId == 0) continue;
		if (entId == 64) continue;
		if (entId == 66) continue;
		if (entId == 69) continue;
		if (entId == 70) continue;
		if (entId == 95) continue;
		if (entId == 4194372) continue;
		if (entId == 4194390) continue;
		if (entId == 4194405) continue;
		if (entId == 4194391) continue;
		if (entId == 12582992) continue;
		if (!actor->isPlayer()) continue;
		if (!actor->isAlive()) {
			Vec3<float> blockPos = actor->getPosition()->floor(); blockPos.y -= 1.f;
			LogoutStruct logout = LogoutStruct(*actor->getNameTag(), blockPos.toInt());
			logoutData.push_back(logout);
		}
	}
	if (logoutData.empty()) return;
	for (Actor* actor : mc.getLocalPlayer()->getLevel()->getRuntimeActorList()) {
		if (!TargetUtils::isTargetValid(actor, false)) continue;
		for (int i = 0; i < logoutData.size(); i++) {
			if (*actor->getNameTag() == logoutData[i].name) {
				logoutData.erase(logoutData.begin() + i);
				break;
			}
		}
	}
	for (LogoutStruct data : logoutData) {
		std::string name = data.name;
		Vec3<int> blockPos = data.logoutPos;
		AABB playerAABB = AABB(blockPos.toFloat(), blockPos.add(0.6f, 1.8f, 0.6f).toFloat());
ImGuiUtils::drawBox(playerAABB, UIColor(255, 0, 0, 5), UIColor(255, 0, 0, 7), 0.3f, true, false);
	
		Vec2<float> pos;
		if (ImGuiUtils::worldToScreen(blockPos.add(0.f, 1.10f, 0.f).toFloat(), pos)) {
			float textWidth = ImGuiUtils::getTextWidth(name, 1.f);
			float textHeight = ImGuiUtils::getTextHeight(1.f);
			Vec2<float> textPos = Vec2<float>(pos.x - textWidth / 2.f, pos.y - textHeight / 2.f);
			ImGuiUtils::drawText(textPos, name, UIColor(180, 180, 180, 255), 1.f);
		}
	}
}