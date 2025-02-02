#include "ItemStackBase.h"
#include "ScreenContext.h"
#include "Minecraft.h"
#include "ClientInstance.h"
#include "../../Utils/MemoryUtils.h"
#include "../../Utils/Math.h"

class ItemRenderer;
class BaseActorRenderContext {
public:
	char pad[4096];
public:
	BaseActorRenderContext(ScreenContext* screenCtx, ClientInstance* clientInstance, MinecraftGame* minecraft) {
		memset(this, 0, sizeof(BaseActorRenderContext));
		using func_t = void(__fastcall*)(BaseActorRenderContext*, ScreenContext*, ClientInstance*, MinecraftGame*);
		static func_t func = reinterpret_cast<func_t>(findSig(Sigs::hook::baseActorRenderContext));
		func(this, screenCtx, clientInstance, minecraft);
	}
	BUILD_ACCESS(this, ItemRenderer*, itemRenderer, 0x58);
};

class ItemRenderer {
public:
	void renderGuiItem(BaseActorRenderContext* baseActorCtx, ItemStack* item, int mode, Vec2<float> pos, float opacity, float scale, bool isEnchanted) {
		using func_t = void(__fastcall*)(ItemRenderer*, BaseActorRenderContext*, ItemStack*, int, float, float, bool, float, float, float);
		static func_t func = reinterpret_cast<func_t>(findSig(Sigs::Itemstack::itemRenderer));
		func(this, baseActorCtx, item, mode, pos.x, pos.y, isEnchanted, 1, opacity, scale);
	}
};
