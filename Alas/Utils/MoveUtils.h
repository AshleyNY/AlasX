#pragma once
#include "../../SDK/GameData.h"

class MoveUtils {
public:
	static Vec2<float> getMotion(float speed);
	static void setSpeed(float speed);
	static bool isMoving(bool jumpCheck = false);
};