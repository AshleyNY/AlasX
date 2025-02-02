#pragma once
#include "../Module.h"
#include "../../ModuleManager.h"
#include <unordered_map>

struct LogoutStruct {
public:
	std::string name;
	Vec3<int> logoutPos;
	LogoutStruct(std::string _name, Vec3<int> _logoutPos) {
		name = name;
		logoutPos = _logoutPos;
	}
};

class LogoutSpot : public Module {
public:
	std::vector<LogoutStruct> logoutData;
public:
	bool clearData = false;
public:
	LogoutSpot();
	virtual void onImGuiRender(ImDrawList* d) override;
};