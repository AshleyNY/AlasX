#pragma once
#include <iostream>
#include <random>

#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <unordered_set>
#include "../SDK/Classes/Actor.h"
#include "../SDK/Classes/ActorModel.h"
class ActorModelUtils {
public:
	static class std::map<Actor*, ActorModel> actorModels;
    //Vec2<float> GetRotations(Vec3<float> playerEyePos, Vec3<float> targetPos);
};