#pragma once
#include "ItemStackBase.h"

class ItemStack : public ItemStackBase {
public:
	BUILD_ACCESS(this, void*, tag, 0x10);
public:
	uint8_t stackNetId; //0x0088
	ItemStack()
	{
		memset(this, 0, sizeof(ItemStack));
	}
	bool isValid() {
		return (this->getItemPtr() != nullptr);
	}
	float getDamageValue() {
		if (!this->isValid()) return 0;
		using func_t = short(__fastcall*)(Item*, void*);
		static func_t func = reinterpret_cast<func_t>(findSig(Sigs::Itemstack::getDamageValue));
		return (float)func(this->getItemPtr(), this->tag);
	}
	float getMaxDamage() {
		using func_t = short(__fastcall*)(ItemStack*);
		static func_t func = reinterpret_cast<func_t>(findSig(Sigs::Itemstack::getMaxDamage));
		return (float)func(this);
	}
};