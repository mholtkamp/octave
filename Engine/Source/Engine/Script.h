#pragma once

#include "Nodes/Node.h"
#include "ScriptFunc.h"
#include "NetFunc.h"
#include "Object.h"

#include "NetworkManager.h"

#include <set>
#include <unordered_map>

struct AnimEvent;
struct Node_Lua;

struct AutoProperty
{
    std::string mVarName;
    std::string mDisplayName;
    DatumType mType;
    Datum mDefaultValue;
    bool mIsArray = false;
    std::vector<Datum> mArrayValues;
};

typedef std::unordered_map<std::string, ScriptNetFunc> ScriptNetFuncMap;

class Script : public Object
{
public:

    DECLARE_OBJECT(Script, Object);

    Script(Node* owner);
    virtual ~Script();

    Node* GetOwner();

    virtual void Tick(float deltaTime);

    void AppendScriptProperties(std::vector<Property>& outProps);

    void SetFile(const char* filename);
    const std::string& GetFile() const;
    const std::string& GetScriptClassName() const;

    void StartScript();
    void RestartScript();
    void StopScript();

    bool IsActive() const;

    void SetWorld(World* world);

    bool ReloadScriptFile(const std::string& fileName, bool restartScript = true);

    std::vector<ScriptNetDatum>& GetReplicatedData();

    bool InvokeNetFunc(const char* name, uint32_t numParams, const Datum** params);
    ScriptNetFunc* FindNetFunc(const char* funcName);
    ScriptNetFunc* FindNetFunc(uint16_t index);
    void ExecuteNetFunc(uint16_t index, uint32_t numParams, const Datum** params);

    void BeginOverlap(Primitive3D* thisNode, Primitive3D* otherNode);
    void EndOverlap(Primitive3D* thisNode, Primitive3D* otherNode);
    void OnCollision(
        Primitive3D* thisNode,
        Primitive3D* otherNode,
        glm::vec3 impactPoint,
        glm::vec3 impactNormal,
        btPersistentManifold* manifold);

    bool HasFunction(const char* name) const;

    void CallFunction(const char* name);
    void CallFunction(const char* name, const Datum& param0);
    void CallFunction(const char* name, const Datum& param0, const Datum& param1);
    void CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2);
    void CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3);
    void CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4);
    void CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5);
    void CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6);
    void CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7);
    Datum CallFunctionR(const char* name);
    Datum CallFunctionR(const char* name, const Datum& param0);
    Datum CallFunctionR(const char* name, const Datum& param0, const Datum& param1);
    Datum CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2);
    Datum CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3);
    Datum CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4);
    Datum CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5);
    Datum CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6);
    Datum CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7);
    void CallFunction(const char* name, uint32_t numParams, const Datum** params, Datum* ret);
    Datum CallFunction(const char* name, const std::vector<Datum>& params);

    bool LuaFuncCall(int numArgs, int numResults = 0);

    Datum GetField(const char* key);
    void SetField(const char* key, const Datum& value);

    void SetArrayScriptPropCount(const std::string& name, uint32_t count);
    void UploadScriptProperties();
    const std::vector<Property>& GetScriptProperties() const;
    std::vector<Property>& GetScriptProperties();
    void SetScriptProperties(const std::vector<Property>& srcProps);

    // Auto property support
    void AddAutoProperty(const std::string& varName, const std::string& displayName, DatumType type, const Datum& defaultValue);
    void AddAutoPropertyArray(const std::string& varName, const std::string& displayName, DatumType type, const std::vector<Datum>& arrayValues);
    void ClearAutoProperties();
    void GatherAutoProperties();

    static bool OnRepHandler(Datum* datum, uint32_t index, const void* newValue);

protected:

    static bool HandleScriptPropChange(Datum* datum, uint32_t index, const void* newValue);
    static bool HandleForeignScriptPropChange(Datum* datum, uint32_t index, const void* newValue);

    void CreateScriptInstance();
    void DestroyScriptInstance();

    void GatherScriptProperties();
    void GatherReplicatedData();
    void RegisterNetFuncs();
    void GatherNetFuncs(std::vector<ScriptNetFunc>& outFuncs);
    void DownloadReplicatedData();

    bool DownloadDatum(lua_State* L, Datum& datum, int udIdx, const char* varName);
    void UploadDatum(Datum& datum, const char* varName);

    void CallTick(float deltaTime);

    bool CheckIfFunctionExists(const char* funcName);

    static std::unordered_map<std::string, ScriptNetFuncMap> sScriptNetFuncMap;

    Node* mOwner = nullptr;
    std::string mFileName;
    std::string mClassName;
    std::vector<Property> mScriptProps;
    std::vector<ScriptNetDatum> mReplicatedData;
    std::vector<AutoProperty> mAutoProperties;
    bool mActive = false;
    bool mTickEnabled = false;
    bool mHandleBeginOverlap = false;
    bool mHandleEndOverlap = false;
    bool mHandleOnCollision = false;
};

