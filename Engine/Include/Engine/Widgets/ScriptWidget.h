#pragma once

#include "Widget.h"

class ScriptWidget : public Widget
{
public:

    DECLARE_FACTORY(ScriptWidget, Widget);

    ScriptWidget();
    ScriptWidget(const char* scriptName);

    void SetFile(const char* filename);
    const std::string& GetFile();
    const std::string& GetTableName();

    virtual void Update() override;

    void StartScript();
    void RestartScript();
    void StopScript();

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


protected:

    void CreateScriptInstance();
    void DestroyScriptInstance();

    std::string mFileName;
    std::string mClassName;
    std::string mTableName;
};
