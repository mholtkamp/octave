#include "Log.h"
#include "Renderer.h"
#include "Widgets/Console.h"

#include "System/System.h"

static bool sInitialized = false;
static MutexHandle sMutex = {};

void InitializeLog()
{
    sMutex = SYS_CreateMutex();
    sInitialized = true;
}

void ShutdownLog()
{
    sInitialized = false;
    SYS_DestroyMutex(sMutex);
}

void LockLog()
{
    OCT_ASSERT(sInitialized);
    SYS_LockMutex(sMutex);
}

void UnlockLog()
{
    OCT_ASSERT(sInitialized);
    SYS_UnlockMutex(sMutex);
}

void WriteConsoleMessage(const char* message, glm::vec4 color)
{
#if CONSOLE_ENABLED
    Renderer* renderer = Renderer::Get();
    Console* console = renderer ? renderer->GetConsoleWidget() : nullptr;

    if (console != nullptr)
    {
        console->WriteOutput(message, color);
    }
#endif
}
