#if EDITOR

#include "NetworkModule.h"
#include "../JsonSettings.h"
#include "../PreferencesManager.h"
#include "ControllerServer/ControllerServer.h"

#include "document.h"
#include "imgui.h"
#include "Log.h"

#include <algorithm>

// Windows winspool.h defines SetPort as SetPortA/SetPortW — undefine to avoid collision
#ifdef SetPort
#undef SetPort
#endif

DEFINE_PREFERENCES_MODULE(NetworkModule, "Network", "")

NetworkModule* NetworkModule::sInstance = nullptr;

NetworkModule::NetworkModule()
{
    sInstance = this;
}

NetworkModule::~NetworkModule()
{
    if (sInstance == this)
    {
        sInstance = nullptr;
    }
}

NetworkModule* NetworkModule::Get()
{
    return sInstance;
}

void NetworkModule::Render()
{
    bool changed = false;
    ControllerServer* server = ControllerServer::Get();

    ImGui::Text("Controller Server");
    ImGui::Spacing();

    if (ImGui::Checkbox("Enabled (Auto-start)", &mControllerServerEnabled))
    {
        changed = true;

        if (server)
        {
            if (mControllerServerEnabled)
            {
                server->Start(mPort);
            }
            else
            {
                server->Stop();
            }
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Automatically start the controller server when the editor launches.");

    ImGui::Spacing();

    int port = mPort;
    if (ImGui::InputInt("Port", &port))
    {
        port = std::clamp(port, 1024, 65535);
        if (port != mPort)
        {
            mPort = port;
            changed = true;
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("HTTP port for the controller server. Changes apply on restart.");

    ImGui::Spacing();

    if (ImGui::Checkbox("Log Requests", &mLogRequests))
    {
        changed = true;
        if (server)
        {
            server->SetLogRequests(mLogRequests);
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Log each incoming REST request to the debug log.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Status display
    bool running = server && server->IsRunning();
    if (running)
    {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Status: Running (port %d)", mPort);
    }
    else
    {
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Status: Stopped");
    }

    ImGui::Spacing();

    // Control buttons
    ImGui::BeginDisabled(running);
    if (ImGui::Button("Start"))
    {
        if (server)
        {
            server->Start(mPort);
            server->SetLogRequests(mLogRequests);
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!running);
    if (ImGui::Button("Stop"))
    {
        if (server)
        {
            server->Stop();
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Restart"))
    {
        if (server)
        {
            server->Restart(mPort);
            server->SetLogRequests(mLogRequests);
        }
    }

    if (changed)
    {
        SetDirty(true);
    }
}

void NetworkModule::SetControllerServerEnabled(bool enabled)
{
    mControllerServerEnabled = enabled;
    SetDirty(true);
}

void NetworkModule::SetPort(int port)
{
    mPort = std::clamp(port, 1024, 65535);
    SetDirty(true);
}

void NetworkModule::SetLogRequests(bool log)
{
    mLogRequests = log;
    SetDirty(true);
}

void NetworkModule::LoadSettings(const rapidjson::Document& doc)
{
    mControllerServerEnabled = JsonSettings::GetBool(doc, "controllerServerEnabled", false);
    mPort = JsonSettings::GetInt(doc, "port", 7890);
    mLogRequests = JsonSettings::GetBool(doc, "logRequests", false);

    mPort = std::clamp(mPort, 1024, 65535);
}

void NetworkModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetBool(doc, "controllerServerEnabled", mControllerServerEnabled);
    JsonSettings::SetInt(doc, "port", mPort);
    JsonSettings::SetBool(doc, "logRequests", mLogRequests);
}

#endif
