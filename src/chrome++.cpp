#include <windows.h>
#include <stdio.h>
#include <psapi.h>

HMODULE hInstance;

#define MAGIC_CODE 0x1603ABD9

#include "detours.h"
#include "version.h"

#include "hijack.h"
#include "utils.h"
#include "patch.h"
#include "config.h"
#include "tabbookmark.h"
#include "hotkey.h"
#include "portable.h"
#include "pakpatch.h"
#include "appid.h"
#include "green.h"

typedef int (*Startup)();
Startup ExeMain = nullptr;

void ChromePlus() {
  // Shortcut.
  //SetAppId();

  // Portable hajack patch.
  MakeGreen();

  // Enhancement of the address bar, tab, and bookmark.
  TabBookmark();

  // Patch the pak file.
  PakPatch();

  // Process the hotkey.
  GetHotkey();
}

void ChromePlusCommand(LPWSTR param) {
  if (!wcsstr(param, L"--portable")) {
    Portable(param);
  } else {
    ChromePlus();
  }
}

int Loader() {
  // Hard patch.
  // MakePatch();

  // Only main interface.
  LPWSTR param = GetCommandLineW();
  // DebugLog(L"param %s", param);
  if (!wcsstr(param, L"-type=")) {
    ChromePlusCommand(param);
  }

  // Return to the main function.
  return ExeMain();
}

void InstallLoader() {
  // Get the address of the original entry point of the main module.
  MODULEINFO mi;
  GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &mi,
                       sizeof(MODULEINFO));
  ExeMain = (Startup)mi.EntryPoint;

  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach((LPVOID*)&ExeMain, Loader);
  auto status = DetourTransactionCommit();
  if (status != NO_ERROR) {
    DebugLog(L"InstallLoader failed: %d", status);
  }
}

__declspec(dllexport) void portable() {}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID pv) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    hInstance = hModule;

    // Maintain the original function of system DLLs.
    LoadSysDll(hModule);

    InstallLoader();
  }
  return TRUE;
}
