#pragma once

#if EDITOR

class ControllerServer;

// app is a crow::SimpleApp* — crow types are hidden from headers to avoid WinSock conflicts.
void RegisterRoutes(void* app, ControllerServer* server);

#endif
