#pragma once
#include "../../../../SDK/Classes/Actor.h"
#include "../../../../Utils/MemoryUtils.h"

#include "../../../Client.h"
#include <intrin.h>
#include "../../../../Utils/ActorModelUtils.h"

class ActorModelHook {
protected:
	using func_t = void(__fastcall*)(__int64, __int64, __int64, float, int);
	static inline func_t func;

	static void ActorModelCallback(__int64 a1, __int64 a2, __int64 a3, float a4, int a5) {
		auto a = *(uintptr_t**)((uintptr_t)(a2)+0x8);
		Actor* ent = reinterpret_cast<Actor*>(a) + 0x0CF0;
		if (mc.getClientInstance() != nullptr) {
			if (*reinterpret_cast<int*>(a1 + 0x2F4) != 2 && a5 != 2) {
				// Check if we are getting head rot
				char* bodyPart = reinterpret_cast<char*>(a3 + 0x10);
				if (ent != nullptr && (__int64)ent != 0x2B48EC8CFD8 && (__int64)ent != 0x1FE160E33D8) {
					//Log("%s", bodyPart);
					if (strcmp(bodyPart, "head") == 0) ActorModelUtils::actorModels[ent].models[0] = reinterpret_cast<ActorPartModel*>(a3 + 0xEC);
					else if (strcmp(bodyPart, "leftarm") == 0) ActorModelUtils::actorModels[ent].models[1] = reinterpret_cast<ActorPartModel*>(a3 + 0xEC);
					else if (strcmp(bodyPart, "rightarm") == 0) ActorModelUtils::actorModels[ent].models[2] = reinterpret_cast<ActorPartModel*>(a3 + 0xEC);
					else if (strcmp(bodyPart, "leftleg") == 0) ActorModelUtils::actorModels[ent].models[3] = reinterpret_cast<ActorPartModel*>(a3 + 0xEC);
					else if (strcmp(bodyPart, "rightleg") == 0) ActorModelUtils::actorModels[ent].models[4] = reinterpret_cast<ActorPartModel*>(a3 + 0xEC);
					else if (strcmp(bodyPart, "body") == 0) ActorModelUtils::actorModels[ent].models[5] = reinterpret_cast<ActorPartModel*>(a3 + 0xEC);
				}
			}
		}

		func(a1, a2,a3,a4,a5);
	}

public:
	static void init() {
		uintptr_t address = findSig(Sigs::hook::ActorModelHook);
		MemoryUtils::CreateHook("ActorModelHook", address, (void*)&ActorModelHook::ActorModelCallback, (void*)&func);
	}
};