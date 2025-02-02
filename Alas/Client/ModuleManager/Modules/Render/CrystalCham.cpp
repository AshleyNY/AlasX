#include "CrystalCham.h"
#include "../../../../Utils/ActorModelUtils.h"

CrystalCham::CrystalCham() : Module("CrystalCham", "CrystalCham Enities.", Category::RENDER) {
	addColorPicker("Color", "NULL", &color);
	addColorPicker("CirColor", "NULL", &CirColor);
	addBoolCheck("Extra", "T", &CaCham);
	addBoolCheck("Circle", "T", &circle);
	addBoolCheck("CircleFill", "T", &circlefill);
	addSlider<float>("Yspeed", "Change", ValueType::FLOAT_T, &movingY, 1.f, 1000);
	addSlider<float>("Yheight", "Change", ValueType::FLOAT_T, &movingT, 1.f, 5);
	addSlider<float>("CSize", "Change", ValueType::FLOAT_T, &size, 0.f, 2);
	addSlider<float>("CHeight", "Change", ValueType::FLOAT_T, &height, 0.f, 2);
	addSlider<float>("CSpeed", "Change", ValueType::FLOAT_T, &Speed, 1.f, 1000);
}







void CrystalCham::onRender(MinecraftUIRenderContext* ctx) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();
	if (localPlayer == nullptr) return;
	if (!mc.canUseMoveKeys()) return;
	Level* level = localPlayer->getLevel();
	if (level == nullptr) return;
	CrystalList.clear();
	for (Actor* ent : level->getRuntimeActorList()) {
		if (ent->getEntityTypeId() != 71)
			continue;
		bool isValid = TargetUtils::isTargetValid(ent, true);
		float dist = ent->stateVectorComponent->pos.dist(localPlayer->stateVectorComponent->pos);
		float rangeCheck = 15;
		//mc.DisplayClientMessage("%f",dist);
		if (dist < rangeCheck) {
			if (isValid)
				CrystalList.push_back(ent);
		}
	}
	for (Actor* ent : CrystalList) {
		static  float anim = 0;
		anim += movingY / 1000;
		float height2 = ent->aabbComponent->aabb.upper.y - ent->aabbComponent->aabb.lower.y;
		const float coolAnim = (height2 / movingT) + (height2 / movingT) * sin(((float)anim / 60.f) * PI * 0.8f);

		Vec3<float> lpPos = ent->stateVectorComponent->pos.add(0, coolAnim + 0.5, 0);
		//if (mc.cameraPerspectiveMode == 0) lpPos = mc.getClientInstance()->getLevelRenderer()->levelRendererPlayer->cameraPos1;
		std::vector<Vec2<float>> Plist1 = { 0 };
		std::vector<Vec2<float>> Plist2 = { 0 };
		std::vector<Vec2<float>> Plist3 = { 0 };
		std::vector<Vec2<float>> Plist4 = { 0 };

		for (int i = 0; i < 360; i += 1) {
			static Vec2<float> PV1 = { 1,1 };
			static Vec2<float> PV2 = { 1,1 };
			static Vec2<float> PV3 = { 1,1 };
			static Vec2<float> PV4 = { 1,1 };
			float calcYaw = (i + 90) * (PI / 180);
			float x = cos(calcYaw) * size;
			float z = sin(calcYaw) * size;
			//if (PV1 != nullptr && !Plist2.empty() && !Plist3.empty() && !Plist4.empty()
			//if (PV1.x > 0 && PV1.y > 0 && PV2.x > 0 && PV2.y > 0 && PV3.x > 0 && PV3.y > 0 && PV4.x > 0 && PV4.y > 0) {
			RenderUtils::worldToScreen(lpPos.add(x, height, z), PV1);
				Plist1.push_back(PV1);
			
			RenderUtils::worldToScreen(lpPos.add(-x, 0, z), PV2);
				Plist2.push_back(PV2);
			
			RenderUtils::worldToScreen(lpPos.add(x, 0, -z), PV3);
				Plist3.push_back(PV3);
			
			RenderUtils::worldToScreen(lpPos.add(-x, -height, -z), PV4);
				Plist4.push_back(PV4);
			

		}
		x1 += Speed / 1000;
		if (x1 > 358) {
			x1 = 0;
		}
		y1 += Speed / 1000;
		if (y1 > 358) {
			y1 = 0;
		}
		z1 += Speed / 1000;
		if (z1 > 358) {
			z1 = 0;
		}
		x2 += Speed / 1000;
		if (x2 > 358) {
			x2 = 0;
		}
		if (CaCham) {
			ex1 += Speed / 1000;
			if (ex1 > 358) {
				ex1 = 0;
			}
			ex2 += Speed / 1000;
			if (ex2 > 358) {
				ex2 = 0;
			}
			ex3 += Speed / 1000;
			if (ex3 > 358) {
				ex3 = 0;
			}
			ex4 += Speed / 1000;
			if (ex4 > 358) {
				ex4 = 0;
			}
		}
		//color

		if (!Plist1.empty() && !Plist2.empty() && !Plist3.empty() && !Plist4.empty()) {

			RenderUtils::setColor(color.toMC_Color());
			RenderUtils::drawQuad(Plist1[x1], Plist2[x1], Plist3[x1], Plist4[x1]);
			RenderUtils::drawQuad(Plist4[x1], Plist3[x1], Plist2[x1], Plist1[x1]);
			RenderUtils::drawQuad(Plist1[y1], Plist2[y1], Plist3[y1], Plist4[y1]);
			RenderUtils::drawQuad(Plist4[y1], Plist3[y1], Plist2[y1], Plist1[y1]);
			RenderUtils::drawQuad(Plist1[z1], Plist2[z1], Plist3[z1], Plist4[z1]);
			RenderUtils::drawQuad(Plist4[z1], Plist3[z1], Plist2[z1], Plist1[z1]);
			RenderUtils::drawQuad(Plist1[x2], Plist2[x2], Plist3[x2], Plist4[x2]);
			RenderUtils::drawQuad(Plist4[x2], Plist3[x2], Plist2[x2], Plist1[x2]);
			/*if (CaCham) {
				RenderUtils::drawQuad(Plist1[ex1], Plist2[ex1], Plist3[ex1], Plist4[ex1]);
				RenderUtils::drawQuad(Plist4[ex1], Plist3[ex1], Plist2[ex1], Plist1[ex1]);
				RenderUtils::drawQuad(Plist1[ex2], Plist2[ex2], Plist3[ex2], Plist4[ex2]);
				RenderUtils::drawQuad(Plist4[ex2], Plist3[ex2], Plist2[ex2], Plist1[ex2]);
				RenderUtils::drawQuad(Plist1[ex3], Plist2[ex3], Plist3[ex3], Plist4[ex3]);
				RenderUtils::drawQuad(Plist4[ex3], Plist3[ex3], Plist2[ex3], Plist1[ex3]);
				RenderUtils::drawQuad(Plist1[ex4], Plist2[ex4], Plist3[ex4], Plist4[ex4]);
				RenderUtils::drawQuad(Plist4[ex4], Plist3[ex4], Plist2[ex4], Plist1[ex4]);
			}*/
		}

	}
	
}
void CrystalCham::onImGuiRender(ImDrawList* d) {
	LocalPlayer* localPlayer = mc.getLocalPlayer();

	if (localPlayer == nullptr) return;
	if (!mc.canUseMoveKeys()) return;
	Level* level = localPlayer->getLevel();
	if (level == nullptr) return;
	CrystalList.clear();
	for (Actor* ent : level->getRuntimeActorList()) {
		if (ent->getEntityTypeId() != 71)
			continue;
		bool isValid = TargetUtils::isTargetValid(ent, true);
		if (isValid) CrystalList.push_back(ent);
	}
	if(circle){
		for (Actor* ent : CrystalList) {
			Vec3<float> lpPos = ent->stateVectorComponent->pos.add(0, 1.6, 0);
			//if (mc.cameraPerspectiveMode == 0) lpPos = mc.getClientInstance()->getLevelRenderer()->levelRendererPlayer->cameraPos1;
			std::vector<Vec2<float>> pointsList;
			std::vector<Vec2<float>> pointsList2;
			for (int i = 0; i < 360; i += 1) {
				float calcYaw = (i + 90) * (PI / 180);
				float x = cos(calcYaw) * 0.5;
				float z = sin(calcYaw) * 0.5;
				static Vec2<float> pointsVec2;
				static Vec2<float> pointsVec22;
				if (ImGuiUtils::worldToScreen(lpPos.add(x, -1.6f, z), pointsVec2)) {
					pointsList.push_back(pointsVec2);
				}
				if (ImGuiUtils::worldToScreen(lpPos.add(-x, -1.6f, -z), pointsVec22)) {
					pointsList2.push_back(pointsVec22);
				}
			}
			for (int i = 0; i < pointsList.size(); i++) {
				int next = i + 1;
				if (next >= pointsList.size()) next = 0;
				if (circlefill) {
					d->AddLine(pointsList[i].toImVec2(), pointsList2[i].toImVec2(), CirColor.toImColor(), 5.f);
				}
				d->AddLine(pointsList[i].toImVec2(), pointsList[next].toImVec2(), CirColor.toImColor(), 2.f);
			}
		}
	}
}
void CrystalCham::onLevelRender()
{
	

}

void CrystalCham::onEnable()
{

}

void CrystalCham::onDisable()
{
	CrystalList.clear();
	 x1 = 0;
	 y1 = 90;
	 z1 = 180;
	 x2 = 270;
	 ex1 = 45;
	 ex2 = 135;
	 ex3 = 225;
	 ex4 = 315;
}
