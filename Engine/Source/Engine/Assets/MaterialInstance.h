#pragma once

#include "Assets/Material.h"

class MaterialInstance : public Material
{
public:

    DECLARE_ASSET(MaterialInstance, Material);

    static MaterialInstance* New(const Material* src = nullptr);

    MaterialInstance();
    ~MaterialInstance();

    virtual bool IsTransient() const override;
    virtual bool IsMaterialInstance() const override;

    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream);

};
