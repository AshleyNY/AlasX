#pragma once
#include <optional>
#include <memory>
#include <vector>
#include <algorithm>



#include "Modules/Module.h"

//Combat
#include "Modules/Combat/Killaura.h"
#include "Modules/Combat/AutoCrystal.h"
#include "Modules/Combat/Reach.h"
#include "Modules/Combat/GravityAura.h"
#include "Modules/Combat/AutoAnvil.h"
#include "Modules/Combat/AutoTntSpammer.h"
#include "Modules/Combat/AnchorAura.h"
#include "Modules/Combat/AnchorAuraNeo.h"
#include "Modules/Combat/PopCounter.h"
#include "Modules/Combat/AutoTrap.h"
#include "Modules/Combat/CevBreaker.h"
#include "Modules/Combat/HoleFiller.h"
#include "Modules/Combat/Elevator.h"
#include "Modules/Combat/AutoMine.h"
#include "Modules/Combat/PistonAura.h"
#include "Modules/Combat/SelfTrap.h"
#include "Modules/Combat/HoleKicker.h"
//Misc
#include "Modules/Misc/NoPacket.h"
#include "Modules/Misc/Timer.h"
#include "Modules/Misc/Disabler.h"
#include "Modules/Misc/Anticrystal.h"
#include "Modules/Misc/AntiInvis.h"
#include "Modules/Misc/InventoryCleaner.h"
#include "Modules/Misc/TestModule.h"
//Render
#include "Modules/Render/CameraNoClip.h"
#include "Modules/Render/ConicalHat.h"
#include "Modules/Render/CustomFov.h"
#include "Modules/Render/Fullbright.h"
#include "Modules/Render/HurtColor.h"
#include "Modules/Render/NameTags.h"
#include "Modules/Render/NoHurtcam.h"
#include "Modules/Render/NoRender.h"
#include "Modules/Render/Tracer.h"
#include "Modules/Render/ESP.h"
#include "Modules/Render/AutoEmote.h"
#include "Modules/Render/CustomSky.h"
#include "Modules/Render/CrystalCham.h"
#include "Modules/Render/BlockHighlight.h"
#include "Modules/Render/LogoutSpot.h"
#include "Modules/Render/PlayerCham.h"









//Movement
#include "Modules/Movement/AutoSprint.h"
#include "Modules/Movement/Speed.h"
#include "Modules/Movement/Phase.h"
#include "Modules/Movement/Velocity.h"
#include "Modules/Movement/Clip.h"
#include "Modules/Movement/Fly.h"
#include "Modules/Movement/ElytraFly.h"
#include "Modules/Movement/NoSlowDown.h"
#include "Modules/Movement/Burrow.h"
#include "Modules/Movement/Fall.h"
//Player
#include "Modules/Player/OffhandExploit.h"
#include "Modules/Player/AutoEat.h"
#include "Modules/Player/ChestStealler.h"
#include "Modules/Player/AntiAnvil.h"
#include "Modules/Player/BlockReach.h"
#include "Modules/Player/FastEat.h"
#include "Modules/Player/PacketMine.h"
#include "Modules/Player/Swing.h"
#include "Modules/Player/AntiNigga.h"
#include "Modules/Player/Scaffold.h"
#include "Modules/Player/Surround.h"
#include "Modules/Player/AutoOffhand.h"
#include "Modules/Player/Regen.h"
#include "Modules/Player/AutoRespawn.h"
#include "Modules/Player/PacketXP.h"
//Client
#include "Modules/Client/ArrayList.h"
#include "Modules/Client/ClickGui.h"
#include "Modules/Client/Colors.h"
#include "Modules/Client/CustomFont.h"
#include "Modules/Client/HUD.h"
#include "Modules/Client/Notifications.h"
#include "Modules/Client/Clients.h"
//Unknow

class ModuleManager {
private:
	std::vector<Module*> moduleList;
    bool initialized = false;
public:
	ModuleManager();
    ~ModuleManager();
public:
    inline std::vector<Module*>* getModuleList() { return &this->moduleList; }
    inline const bool isInitialized() { return this->initialized; }

	Module* getModule(const std::string& moduleName) {
        std::string nameLower(moduleName);
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        for (Module* mod : moduleList) {
            std::string modNameLower = mod->getModuleName();
            std::transform(modNameLower.begin(), modNameLower.end(), modNameLower.begin(), ::tolower);
            if (strcmp(nameLower.c_str(), modNameLower.c_str()) == 0) return mod;
        }
        return nullptr;
	}
    std::optional<Module*> getModuleByName(const std::string& name) {
        std::string nameLower = name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

        for (Module* mod : moduleList) {
            std::string modNameLower = mod->getModuleName();
            std::transform(modNameLower.begin(), modNameLower.end(), modNameLower.begin(), ::tolower);
            if (modNameLower == nameLower) {
                return mod;
            }
        }

        return std::nullopt;
    }

    


public:
    void onSaveConfig(json* currentConfig);
    void onLoadConfig(json* config);
    void onKeyUpdate(int key, bool isDown);
    void onRender(MinecraftUIRenderContext* ctx);
    void onImGuiRender(ImDrawList* drawlist);
    void onNormalTick(Actor* actor);
    void onContainerScreenControllerTickEvent(ContainerScreenControllerTickEvent* events);
    void onSendPacket(Packet* packet, bool& shouldCancel);
    void onLevelRender();
};
