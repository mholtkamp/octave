#pragma once

#include "Components/Component.h"
#include "ObjectRef.h"
#include "ScriptableFuncPointer.h"
#include "NetFunc.h"

#include "NetworkManager.h"

#include <set>

struct AnimEvent;
class Button;
class Selector;
class TextField;

typedef std::unordered_map<std::string, ScriptNetFunc> ScriptNetFuncMap;

class ScriptComponent : public Component
{
public:

    DECLARE_COMPONENT(ScriptComponent, Component);

    ScriptComponent();
    virtual ~ScriptComponent();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void BeginPlay() override;
    virtual void EndPlay() override;
    virtual void Tick(float deltaTime);

    virtual void SetOwner(Actor* owner);

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    void AppendScriptProperties(std::vector<Property>& outProps);

    void SetFile(const char* filename);
    const std::string& GetFile() const;
    const std::string& GetScriptClassName() const;
    const std::string& GetTableName() const;

    void StartScript();
    void RestartScript();
    void StopScript();

    bool LoadScriptFile(const std::string& fileName);
    bool ReloadScriptFile(const std::string& fileName, bool restartScript = true);

    bool ShouldHandleEvents() const;

    std::vector<ScriptNetDatum>& GetReplicatedData();

    void InvokeNetFunc(const char* name, std::vector<Datum>& params);
    ScriptNetFunc* FindNetFunc(const char* funcName);
    ScriptNetFunc* FindNetFunc(uint16_t index);
    void ExecuteNetFunc(uint16_t index, uint32_t numParams, std::vector<Datum>& params);

    void BeginOverlap(PrimitiveComponent* thisComp, PrimitiveComponent* otherComp);
    void EndOverlap(PrimitiveComponent* thisComp, PrimitiveComponent* otherComp);
    void OnCollision(
        PrimitiveComponent* thisComp,
        PrimitiveComponent* otherComp,
        glm::vec3 impactPoint,
        glm::vec3 impactNormal,
        btPersistentManifold* manifold);

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

    static ScriptComponent* GetExecutingScriptComponent();
    static const char* GetExecutingScriptTableName();
    static bool OnRepHandler(Datum* datum, const void* newValue);

    static std::string GetClassNameFromFileName(const std::string& fileName);
    static void SetEmbeddedScripts(EmbeddedFile* embeddedScripts, uint32_t numEmbeddedScripts);
    static EmbeddedFile* FindEmbeddedScript(const std::string& className);

protected:

    static bool HandlePropChange(Datum* datum, const void* newValue);
    static bool HandleScriptPropChange(Datum* datum, const void* newValue);
    static bool HandleForeignScriptPropChange(Datum* datum, const void* newValue);

    void CreateScriptInstance();
    void DestroyScriptInstance();

    void UploadScriptProperties();
    void GatherScriptProperties();
    void GatherReplicatedData();
    void RegisterNetFuncs();
    void GatherNetFuncs(std::vector<ScriptNetFunc>& outFuncs);
    void DownloadReplicatedData();

    bool DownloadDatum(lua_State* L, Datum& datum, int tableIdx, const char* varName);
    void UploadDatum(Datum& datum, const char* varName);

    bool LuaFuncCall(int numArgs, int numResults = 0);
    void CallTick(float deltaTime);

    bool CheckIfFunctionExists(const char* funcName);

    static std::set<std::string> sLoadedLuaFiles;
    static std::unordered_map<std::string, ScriptNetFuncMap> sScriptNetFuncMap;
    static EmbeddedFile* sEmbeddedScripts;
    static uint32_t sNumEmbeddedScripts;
    static uint32_t sNumScriptInstances;
    static ScriptComponent* sExecutingScript;

    std::string mFileName;
    std::string mClassName;
    std::string mTableName;
    std::vector<Property> mScriptProps;
    std::vector<ScriptNetDatum> mReplicatedData;
    bool mTickEnabled = false;
    bool mHandleBeginOverlap = false;
    bool mHandleEndOverlap = false;
    bool mHandleOnCollision = false;
};

