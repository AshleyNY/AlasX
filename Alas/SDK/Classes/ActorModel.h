#pragma once
#include <cstring>
#include "../../Utils/Math.h"
struct ActorPartModel {
    Vec3<float> rpos;
    Vec3<float> rot;
    Vec3<float> rsize;
};

struct ActorModel {
    ActorPartModel* models[6];

    ActorModel() {
        memset(this, 0, sizeof(ActorModel));
    }

    __forceinline bool isValid() {
        for (int i = 0; i < 6; i++) {
            if (models[i] == nullptr) return false;
        }
        return true;
    }
};