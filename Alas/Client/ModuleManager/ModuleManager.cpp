#include "ModuleManager.h"
#include "../Client.h"
#include "Modules/Misc/AutoTools.h"
#include "Modules/Player/AutoRespawn.h"
#include "Modules/Player/OffhandExploit.h"
#include "Modules/Combat/AutoMine.h"

ModuleManager::ModuleManager() {

	{ // Init modules
		//Combat
		moduleList.push_back(new Killaura());
		moduleList.push_back(new AutoCrystal());
		moduleList.push_back(new Reach());
		moduleList.push_back(new AutoAnvil());
		moduleList.push_back(new GravityAura());
		moduleList.push_back(new AutoTntSpammer());
		moduleList.push_back(new AnchorAura());
		moduleList.push_back(new AnchorAuraNeo());
		moduleList.push_back(new PopCounter());
		moduleList.push_back(new AutoTrap());
		moduleList.push_back(new CevBreaker());
		moduleList.push_back(new HoleFiller());
		moduleList.push_back(new Elevator());
		moduleList.push_back(new AutoMine());
		moduleList.push_back(new PistonAura());
		moduleList.push_back(new SelfTrap());
		moduleList.push_back(new HoleKicker());
		//Misc
		moduleList.push_back(new NoPacket());
		moduleList.push_back(new Timer()); 
		moduleList.push_back(new TestModule());
		moduleList.push_back(new Disabler());
		moduleList.push_back(new AntiCrystal());
		moduleList.push_back(new AntiInvis());
		moduleList.push_back(new InventoryCleaner());
		//moduleList.push_back(new AutoTools());
		//Render
		moduleList.push_back(new CameraNoClip());
		moduleList.push_back(new ConicalHat());
		moduleList.push_back(new CustomFov());
		moduleList.push_back(new Fullbright());
		moduleList.push_back(new HurtColor());
		moduleList.push_back(new NameTags());
		moduleList.push_back(new NoHurtcam());
		moduleList.push_back(new NoRender());
		moduleList.push_back(new Tracer());
		moduleList.push_back(new ESP());
		moduleList.push_back(new AutoEmote());
		moduleList.push_back(new CustomSky());
		moduleList.push_back(new CrystalCham());
		moduleList.push_back(new BlockHighlight());
		moduleList.push_back(new LogoutSpot());
		moduleList.push_back(new PlayerCham());
		//Movement
		moduleList.push_back(new AutoSprint());
		moduleList.push_back(new Burrow());
		moduleList.push_back(new Fall());
		moduleList.push_back(new Speed());
		moduleList.push_back(new Fly());
		moduleList.push_back(new Phase());
		moduleList.push_back(new Velocity());
		moduleList.push_back(new Clip());
		moduleList.push_back(new ElytraFly());
		moduleList.push_back(new NoSlowDown());
		//Player
		moduleList.push_back(new AutoEat());
		moduleList.push_back(new OffhandExploit());
		moduleList.push_back(new AutoRespawn());
		moduleList.push_back(new ChestStealler());
		moduleList.push_back(new AntiAnvil());
		moduleList.push_back(new BlockReach());
		moduleList.push_back(new FastEat());
		moduleList.push_back(new PacketMine());
		moduleList.push_back(new Swing());
		moduleList.push_back(new Surround());
		moduleList.push_back(new Scaffold());
		moduleList.push_back(new AutoOffhand());
		moduleList.push_back(new Regen());
		moduleList.push_back(new AntiNigga());
		moduleList.push_back(new PacketXP());
		//Client
		moduleList.push_back(new ArrayList());
		//moduleList.push_back(new Clients());
		moduleList.push_back(new Colors());
		moduleList.push_back(new CustomFont());
		moduleList.push_back(new ClickGui());
		moduleList.push_back(new HUD());
		moduleList.push_back(new Notifications());
		//Unknow
	}
	std::sort(moduleList.begin(), moduleList.end(), [](Module* lhs, Module* rhs) {
		return lhs->getModuleName() < rhs->getModuleName();
	});
	getModule("ClickGui")->setEnabled(false);
	getModule("Notifications")->setEnabled(true);
	getModule("ArrayList")->setEnabled(true);
	getModule("HUD")->setEnabled(true);
	this->initialized = true;
}

ModuleManager::~ModuleManager() {
	for (Module* mod : moduleList) {
		mod->setEnabled(false);
		delete mod;
	}
}

void ModuleManager::onSaveConfig(json* currentConfig) {
	((ClickGui*)getModule("ClickGui"))->setEnabled(false);
	for (Module* mod : moduleList) {
		mod->onSaveConfig(currentConfig);
	}
}

void ModuleManager::onLoadConfig(json* config) {
	for (Module* mod : moduleList) {
		mod->onLoadConfig(config);
	}
	((ClickGui*)getModule("ClickGui"))->setEnabled(false);
}

void ModuleManager::onKeyUpdate(int key, bool isDown) {
	if (!client->isInitialized()) return;
	for (Module* mod : moduleList) {
		mod->onKeyUpdate(key, isDown);
	}
}

void ModuleManager::onRender(MinecraftUIRenderContext* ctx) {
	if (!client->isInitialized()) return;
	for (Module* mod : moduleList) {
		if (mod->isEnabled()) {
			mod->onRender(ctx);
		}
	}
}

void ModuleManager::onImGuiRender(ImDrawList* drawlist) {
	if (!client->isInitialized()) return;
	for (Module* mod : moduleList) {
		if (mod->isEnabled()) {
			mod->onImGuiRender(drawlist);
		}
	}
}

void ModuleManager::onNormalTick(Actor* actor) {
	if (!client->isInitialized()) return;
	for (Module* mod : moduleList) {
		if (mod->isEnabled()) {
			mod->onNormalTick(actor);
		}
	}
}
void ModuleManager::onContainerScreenControllerTickEvent(ContainerScreenControllerTickEvent* events) {
	if (!client->isInitialized()) return;
	for (Module* mod : moduleList) {
		if (mod->isEnabled()) {
			mod->onContainerScreenControllerTickEvent(events);
		}
	}
}

void ModuleManager::onSendPacket(Packet* packet, bool& shouldCancel) {
	if (!client->isInitialized()) return;
	for (Module* mod : moduleList) {
		if (mod->isEnabled()) {
			mod->onSendPacket(packet, shouldCancel);
			if (shouldCancel) break;
		}
	}
}

void ModuleManager::onLevelRender() {
	if (!client->isInitialized()) return;
	for (Module* mod : moduleList) {
		if (mod->isEnabled()) {
			mod->onLevelRender();
		}
	}
}
