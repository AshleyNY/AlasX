#pragma once
#include "../../../../SDK/Classes/Actor.h"
#include "../../../../Utils/MemoryUtils.h"

class ActorShouldRenderHook {
protected:
	using func_t = bool(__fastcall*)(Actor*);
	static inline func_t func;

	static bool Actor_ShouldRender(Actor* _this) {
		//return true;
		static AntiInvis* AntiInvisMod = (AntiInvis*)client->moduleMgr->getModule("AntiInvis");
		if (AntiInvisMod->isEnabled()) {
			return AntiInvisMod->isEnabled() ? true : func(_this);
		}
		static CrystalCham* CrystalChamMod = (CrystalCham*)client->moduleMgr->getModule("CrystalCham");
		if (CrystalChamMod->isEnabled()) {
			if (CrystalChamMod->isEnabled() && _this->getEntityTypeId()==71) 
				return false;
		}
		static PlayerCham* PlayerChamMod = (PlayerCham*)client->moduleMgr->getModule("PlayerCham");
		if (PlayerChamMod->isEnabled()) {
			if (PlayerChamMod->isEnabled() && _this->getEntityTypeId() == 319)
				return false;
		}
		return func(_this);
	}
public:
	static void init() {
		uintptr_t address = findSig(Sigs::hook::ShouldRender);
		MemoryUtils::CreateHook("Actor_shouldRenderHook", address, (void*)&ActorShouldRenderHook::Actor_ShouldRender, (void*)&func);
	}
};
