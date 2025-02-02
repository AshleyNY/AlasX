#include "PopCounter.h"

using namespace std;
PopCounter::PopCounter() : Module("PopCham", "Counter Pop", Category::COMBAT) {
	addBoolCheck("SendChat", "NULL", &sendchat);
	addColorPicker("Color", "NULL", &headcolor);
	addColorPicker("CirColor", "NULL", &bodycolor);
	//addColorPicker("LColor", "NULL", &limbcolor);
	addSlider<float>("Duration", "NULL", ValueType::FLOAT_T, &Dura, 0.8f, 2.f);
	//addSlider<float>("Ylock", "NULL", ValueType::FLOAT_T, &YLock, 0.f, 100.f);
	//addSlider<float>("Fade", "NULL", ValueType::FLOAT_T, &Fade, 0.f, 255);
	//registerFloatSetting("NotiDurationTime", &Dura, Dura, 2.f, 0.8f);
//	addBoolCheck("totem", "null", &totem);
	//addBoolCheck("Render", "null", &render);
}


std::string PopCounter::getModName() {
	return names;
}





static std::vector<Actor*> playerlist;


void PopCounter::onNormalTick(Actor* actor) {
	playerlist.clear();

	LocalPlayer* player = mc.getLocalPlayer();
	PlayerInventory* plrInv = player->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	GameMode* gm = mc.getGameMode();
	BlockSource* region = player->dimension->blockSource;
	Level* level = player->getLevel();
	if (!level) return;
	if (player == nullptr)
		return;


	if (!player->isAlive()) {
		popcount = 0;
	}

	for (Actor* actor : level->getRuntimeActorList()) {
		if (TargetUtils::isTargetValid(actor, false)) {
			float dist = actor->getPosition()->dist(*player->getPosition());
			if (dist < 50) playerlist.push_back(actor);
		}
	}

	if (!playerlist.empty()) {
		if (popcount > 37) {
			popcount = 0;
		}
		//ItemStack *current = i->getOffhandSlot()->item;
		if (playerlist[0]->getOffhandSlot()->item == nullptr) {
			if (!totem) {
				ItemStack* stack = playerlist[0]->getArmor(1);
				if (stack->item != nullptr) {
					popcount += 1;

					mc.DisplayClientMessage("[%s%s%s] %s%s %sPopped %s%i%s Totems!", LIGHT_PURPLE, "Alas", WHITE, DARK_AQUA, playerlist[0]->getNameTag()->c_str(), WHITE, RED, popcount, WHITE);

					std::string messagebox = playerlist[0]->getNameTag()->c_str() + std::to_string(popcount) + " totems!";
					Notifications::addNotifBox(messagebox, Dura);
					totem = true;
					render = true;
				}
			}
		}
		else {
			totem = false;
		};

	}

}
void PopCounter::onRender(MinecraftUIRenderContext* ctx)
{

	LocalPlayer* player = mc.getLocalPlayer();
	if (player == nullptr)
		return;
	PlayerInventory* plrInv = player->getPlayerInventory();
	Inventory* inv = plrInv->inventory;
	GameMode* gm = mc.getGameMode();
	Level* level = player->getLevel();
	UIColor SBColor = UIColor(headcolor.r, headcolor.g, headcolor.b, Fade);

	if (!level) return;

	if (!playerlist.empty()) {
		if (render && Fade > 0) {
			YLock += 0.01;
			Fade -= 1;

				Vec3<float> Pos = playerlist[0]->stateVectorComponent->pos.add(0, YLock, 0);
				RenderUtils::drawBoxCustom(Pos, 0.25, 0.23f, 0.23f, SBColor, SBColor, 0.5f, true, true);
				RenderUtils::drawBoxCustom(Pos.add(0, -0.6f, 0), 0.2f, 0.37f, 0.26f, SBColor, SBColor, 0.5f, true, true);
				RenderUtils::drawBoxCustom(Pos.add(0, -0.5f, 0.39), 0.2f, 0.27f, 0.13f, SBColor, SBColor, 0.5f, true, true);
				RenderUtils::drawBoxCustom(Pos.add(0, -0.5f, -0.39), 0.2f, 0.27f, 0.13f, SBColor, SBColor, 0.5f, true, true);
				RenderUtils::drawBoxCustom(Pos.add(0, -1.27f, 0.13), 0.2f, 0.27f, 0.13f, SBColor, SBColor, 0.5f, true, true);
				RenderUtils::drawBoxCustom(Pos.add(0, -1.27f, -0.13), 0.2f, 0.27f, 0.13f, SBColor, SBColor, 0.5f, true, true);
			
		}
		else {
			YLock = 0;
			Fade = headcolor.a;
			render = false;
		}
	}
}

void PopCounter::onImGuiRender(ImDrawList* d)
{
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	UIColor SBColor = UIColor(bodycolor.r, bodycolor.g, bodycolor.b, Fade);
	if (localPlayer == nullptr) return;
	if (!mc.canUseMoveKeys()) return;
	Level* level = localPlayer->getLevel();
	if (level == nullptr) return;
	if (!playerlist.empty()) {
		if (render) {
				Vec3<float> lpPos = playerlist[0]->stateVectorComponent->pos.add(0, YLock, 0);
				//if (mc.cameraPerspectiveMode == 0) lpPos = mc.getClientInstance()->getLevelRenderer()->levelRendererPlayer->cameraPos1;
				std::vector<Vec2<float>> pointsList;
				for (int i = 0; i < 360; i += 4) {
					float calcYaw = (i + 90) * (PI / 180);
					float x = cos(calcYaw) * 0.4;
					float z = sin(calcYaw) * 0.4;
					static Vec2<float> pointsVec2;
					if (ImGuiUtils::worldToScreen(lpPos.add(x, 0.4f, z), pointsVec2)) {
						pointsList.push_back(pointsVec2);
					}
				}
				for (int i = 0; i < pointsList.size(); i++) {
					int next = i + 1;
					if (next >= pointsList.size()) next = 0;
					d->AddLine(pointsList[i].toImVec2(), pointsList[next].toImVec2(), SBColor.toImColor(), 2.f);
				}
			
		}
	}
}