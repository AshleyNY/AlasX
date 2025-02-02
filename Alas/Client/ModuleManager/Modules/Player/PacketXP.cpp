#include "PacketXP.h"
int expDelay = 0;
int IexpDelay = 0;
PacketXP::PacketXP() : Module("PacketXP", "Uses packets to exp very fast ong", Category::PLAYER) {
    addSlider<int>("Waste Amount", "Amount of exp we waste every tick", ValueType::INT_T, &speed, 1, 5);
    addSlider<int>("Delay", "NULL", ValueType::INT_T, &expDelay, 0, 20);
    addBoolCheck("Feet", "Rotate to feet", &feetPlace);
    addEnumSetting("Mode", "NULL", { "Auto", "MidClick", "RightClick" }, &clickMode);
    addEnumSetting("Swap", "NULL", { "None", "Spoof", "Silent" }, &swap);
}

int PacketXP::getItem(int id) {
    PlayerInventory* plrInv = mc.getLocalPlayer()->getPlayerInventory();
    Inventory* inv = plrInv->inventory;
    for (int i = 0; i < 9; i++) {
        ItemStack* itemStack = inv->getItemStack(i);
        if (itemStack->isValid() && itemStack->getItemPtr()->gettexture_name().find("bottle") != std::string::npos) return i;
    }
    return plrInv->selectedSlot;
}

void PacketXP::onNormalTick(Actor* actor) {
    LocalPlayer* localPlayer = (LocalPlayer*)actor;
    GameMode* gm = mc.getGameMode();
    if (clickMode == 2 && !mc.isRightClickDown()) return;
    if (clickMode == 1 && !mc.getHIDController()->wheelDown) return;
    if (!mc.canUseMoveKeys()) return;
    PlayerInventory* plrInv = localPlayer->getPlayerInventory();
    Inventory* inv = plrInv->inventory;
    int oldSlot = plrInv->selectedSlot;
    if (swap == 1) plrInv->selectedSlot = getItem(508);
    else if (swap == 2) {
        MobEquipmentPacket pk(localPlayer->getRuntimeID(), localPlayer->getPlayerInventory()->inventory->getItemStack(getItem(508)), getItem(508), getItem(508));
        mc.getLocalPlayer()->sendNetworkPacket(pk);
        mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
    }
    ItemStack* stack = inv->getItemStack(plrInv->selectedSlot);
    if (IexpDelay >= expDelay) {
        if (stack->item != nullptr && (*stack->item)->itemId != 0 && stack->getItemPtr()->gettexture_name().find("bottle") != std::string::npos) {
            if (feetPlace) localPlayer->rotationComponent->Set(Vec2<float>(90, 0));
            for (int i = 0; i < speed; i++) {
                gm->useItem(*stack);
            }
        }
        IexpDelay = 0;
    }
    else IexpDelay++;
    if (swap == 1) plrInv->selectedSlot = oldSlot;
    else if (swap == 2) {
        MobEquipmentPacket pk(localPlayer->getRuntimeID(), localPlayer->getPlayerInventory()->inventory->getItemStack(oldSlot), oldSlot, oldSlot);
        mc.getLocalPlayer()->sendNetworkPacket(pk);
        mc.getClientInstance()->loopbackPacketSender->sendToServer(&pk);
    }
}