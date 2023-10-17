#pragma once

#include "Utilities.h"

#ifdef GetClassName
#undef GetClassName
#endif

#define DECLARE_FACTORY_MANAGER(Base) \
    static std::vector<Factory*>& GetFactoryList(); \
    static TypeId RegisterFactory(Factory* factory, uint32_t typeIdMod = 0); \
    static Base* CreateInstance(const char* typeName); \
    static Base* CreateInstance(TypeId typeId);

#define DEFINE_FACTORY_MANAGER(Base) \
    std::vector<Factory*>& Base::GetFactoryList() \
    { \
        static std::vector<Factory*> sFactoryList; \
        return sFactoryList; \
    } \
    \
    TypeId Base::RegisterFactory(Factory* factory, uint32_t typeIdMod) \
    { \
        std::vector<Factory*>& factoryList = GetFactoryList(); \
        const char* name = factory->GetClassName(); \
        TypeId typeId = (OctHashString(name) + typeIdMod); \
        if (typeId == 0) { typeId++; } \
        for (uint32_t i = 0; i < factoryList.size(); ++i) { \
            if (strncmp(factoryList[i]->GetClassName(), name, MAX_PATH_SIZE) == 0) { \
                LogError("Conflicting class name found in factory's RegisterClass() - %s", name); OCT_ASSERT(0); typeId = 0; break; } \
            if (factoryList[i]->GetType() == typeId) { \
                LogError("Conflicting TypeId %x encountered in " #Base " factory manager's RegisterClass() - [%s] and [%s]", (uint32_t)typeId, factoryList[i]->GetClassName(), name); \
                LogError("Use special case of XXXXX_FACTORY() with hash add number to avoid conflict."); OCT_ASSERT(0); typeId = 0; break; } \
        } \
        if (typeId != 0) { factoryList.push_back(factory); } \
        return typeId; \
    } \
    \
    Base* Base::CreateInstance(const char* typeName) \
    { \
        std::vector<Factory*>& factoryList = GetFactoryList(); \
        Base* retObject = nullptr; \
        for (uint32_t i = 0; i < factoryList.size(); ++i) { \
            if (strncmp(factoryList[i]->GetClassName(), typeName, MAX_PATH_SIZE) == 0) { \
                retObject = (Base*) factoryList[i]->Create(); break; } \
        } \
        return retObject; \
    }\
    \
    Base* Base::CreateInstance(TypeId typeId) \
    { \
        std::vector<Factory*>& factoryList = GetFactoryList(); \
        Base* retObject = nullptr; \
        for (uint32_t i = 0; i < factoryList.size(); ++i) { \
            if (factoryList[i]->GetType() == typeId) { \
                retObject = (Base*) factoryList[i]->Create(); break; } \
        } \
        return retObject; \
    }

class Factory
{
public:
    Factory()
    {

    }

    TypeId GetType()
    {
        return mType;
    }

    virtual void* Create()
    {
        return nullptr;
    }

    virtual const char* GetClassName() const
    {
        return "Class";
    }

protected:
    TypeId mType = 0;
};

// In the future, DECLARE_FACTORY might do more, but right now it just creates a GetType() func
// The "override" qualifier is left out so that the base class can also use this macro.
#define DECLARE_FACTORY(Class, BaseClass) \
    class Factory_##Class; \
    virtual TypeId GetType() const; \
    virtual const char* GetClassName() const; \
    static TypeId GetStaticType();

#define DEFINE_FACTORY_EX(Class, BaseClass, TypeMod) \
    class Factory_##Class : public Factory \
    { \
        public: \
        Factory_##Class() { mType = BaseClass::RegisterFactory(this, TypeMod); } \
        virtual void* Create() override { return new Class(); } \
        virtual const char* GetClassName() const override { return #Class; } \
    }; \
    static Factory_##Class sFactory_##Class; \
    TypeId Class::GetType() const { return sFactory_##Class.GetType(); } \
    const char* Class::GetClassName() const { return sFactory_##Class.GetClassName(); } \
    TypeId Class::GetStaticType() { return sFactory_##Class.GetType(); }

#define DEFINE_FACTORY(Class, BaseClass) DEFINE_FACTORY_EX(Class, BaseClass, 0)
