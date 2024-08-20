#pragma once

#if API_GX

#include <gccore.h>
#include <stdint.h>
#include "EngineTypes.h"

#define GX_DYNAMIC_LIGHT_SCALE 1.0f

class World;

struct LightingState
{
    bool mEnabled = false;
    bool mColorChannel = false;
    uint8_t mMaterialSrc = GX_SRC_REG;
    uint8_t mLightMask = 0;
    uint8_t mDiffuseFunc = GX_DF_CLAMP;
    uint8_t mAttenuationFunc = GX_AF_SPOT;

    LightingState()
    {

    }

    LightingState(bool enable, bool colorChannel, uint8_t matSrc, uint8_t lightMask, uint8_t diffFunc, uint8_t attenFunc) :
        mEnabled(enable),
        mColorChannel(colorChannel),
        mMaterialSrc(matSrc),
        mLightMask(lightMask),
        mDiffuseFunc(diffFunc),
        mAttenuationFunc(attenFunc)
    {

    }

    bool operator==(const LightingState& other) const
    {
        return
            mEnabled == other.mEnabled &&
            mColorChannel == other.mColorChannel &&
            mMaterialSrc == other.mMaterialSrc &&
            mLightMask == other.mLightMask &&
            mDiffuseFunc == other.mDiffuseFunc &&
            mAttenuationFunc == other.mAttenuationFunc;
    }

    bool operator!=(const LightingState& other) const
    {
        return !operator==(other);
    }
};

struct GxContext
{
    void* mGpFifo = nullptr;
    World* mWorld = nullptr;

    // Lighting/Channel control
    LightingState mLighting;
    uint8_t mSceneLightMask = 0;
    uint8_t mSceneNumLights = 0;
    LightData mLightData[MAX_LIGHTS_PER_DRAW];

    // Fog
    bool mApplyFog = true;
    uint8_t mFogType = 0;
    float mFogStartZ = 0.0f;
    float mFogEndZ = 0.0f;
    float mFogNearZ = 0.0f;
    float mFogFarZ = 0.0f;
    GXColor mFogColor = {0,0,0,0};
};

#endif