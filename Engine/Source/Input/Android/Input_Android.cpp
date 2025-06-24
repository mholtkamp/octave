#if PLATFORM_ANDROID

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"
#include "Log.h"

void INP_Initialize()
{
    InputInit();
}

void INP_Shutdown()
{
    InputShutdown();
}

void INP_Update()
{
    InputAdvanceFrame();

}

void INP_SetCursorPos(int32_t x, int32_t y)
{

}

void INP_ShowCursor(bool show)
{

}

void INP_LockCursor(bool lock)
{

}

void INP_TrapCursor(bool trap)
{

}

void INP_ShowSoftKeyboard(bool show)
{
    SystemState& system = GetEngineState()->mSystem;

#if 0
    // This call does nothing?
    ANativeActivity *nativeActivity = GetEngineState()->mSystem.mActivity;
    ANativeActivity_showSoftInput(nativeActivity, 0);
#else
    JNIEnv* env = nullptr;
    JavaVM* vm = system.mActivity->vm;
    vm->AttachCurrentThread(&env, nullptr);

    jobject octActivity = system.mActivity->clazz;
    jclass octClass = env->GetObjectClass(octActivity);

    if (show)
    {
        jmethodID showMethod = env->GetMethodID(octClass, "showSoftKeyboard", "()V");
        env->CallVoidMethod(octActivity, showMethod);
    }
    else
    {
        jmethodID hideMethod = env->GetMethodID(octClass, "hideSoftKeyboard", "()V");
        env->CallVoidMethod(octActivity, hideMethod);
    }

    vm->DetachCurrentThread();
#endif
}

bool INP_IsSoftKeyboardShown()
{
    bool shown = false;
    SystemState& system = GetEngineState()->mSystem;

    JNIEnv* env = nullptr;
    JavaVM* vm = system.mActivity->vm;
    vm->AttachCurrentThread(&env, nullptr);

    jobject octActivity = system.mActivity->clazz;
    jclass octClass = env->GetObjectClass(octActivity);

    jmethodID showMethod = env->GetMethodID(octClass, "isSoftKeyboardShown", "()Z");
    shown = (bool)env->CallBooleanMethod(octActivity, showMethod);

    return shown;
}

#endif