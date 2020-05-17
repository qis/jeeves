#include "code.h"
#include "input.h"
#include "loader.h"
#include <config.h>
#include <fstream>
#include <vector>

namespace csgo {

WNDPROC proc = nullptr;

std::unique_ptr<Code> code;
std::unique_ptr<hook> client_hook;
std::unique_ptr<hook> engine_vgui_hook;
std::unique_ptr<hook> client_mode_hook;

using PaintProc = void(__thiscall*)(IEngineVGui* self, int mode);
using CreateMoveProc = bool(__thiscall*)(IClientMode* self, float sample_input_frametime, CUserCmd* cmd);
using FrameStageNotifyProc = void(__stdcall*)(ClientFrameStage stage);

PaintProc PaintOrig = nullptr;
CreateMoveProc CreateMoveOrig = nullptr;
FrameStageNotifyProc FrameStageNotifyOrig = nullptr;

void __stdcall PaintHook(int mode) {
  PaintOrig(engine_vgui, mode);
  if (mode & PAINT_UIPANELS) {
    StartDrawing(surface);
    code->Paint(mode);
    FinishDrawing(surface);
  }
}

bool __stdcall CreateMoveHook(float sample_input_frametime, CUserCmd* cmd) {
  CreateMoveOrig(client_mode, sample_input_frametime, cmd);
  code->CreateMove(sample_input_frametime, cmd);
  engine->SetViewAngles(cmd->viewangles);
  return false;
}

void __stdcall FrameStageNotifyHook(ClientFrameStage stage) {
  code->FrameStageNotify(stage);
  FrameStageNotifyOrig(stage);
}

DWORD WINAPI code_create(LPVOID param);
DWORD WINAPI code_delete();

void OnClose() {
  code_delete();
}

void OnError(const char* text, const char* title) {
  std::wstring wtext;
  wtext.resize(MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0) + 1);
  wtext.resize(MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext.data(), static_cast<int>(wtext.size())));
  std::wstring wtitle;
  wtitle.resize(MultiByteToWideChar(CP_UTF8, 0, title, -1, nullptr, 0) + 1);
  wtitle.resize(MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle.data(), static_cast<int>(wtitle.size())));
  MessageBox(window, wtext.data(), wtitle.data(), MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
}

inline void OnKey(UINT_PTR index, bool down) {
  if (index < 0xFF) {
    input::key[index] = down;
    if (code) {
      code->OnKey(static_cast<int>(index), down);
    }
  }
}

inline void OnButton(int index, USHORT flags, USHORT mask_down, USHORT mask_up) {
  if (flags & mask_down) {
    input::button[index] = true;
    if (code) {
      code->OnButton(index, true);
    }
  } else if (flags & mask_up) {
    input::button[index] = false;
    if (code) {
      code->OnButton(index, false);
    }
  }
};

inline void OnInput(HRAWINPUT input) {
  static std::vector<BYTE> buffer;
  if (code) {
    UINT size = 0;
    GetRawInputData(input, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
    if (size > buffer.size()) {
      buffer.resize(size);
    }
    GetRawInputData(input, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));
    const auto raw = reinterpret_cast<LPRAWINPUT>(buffer.data());
    if (raw->header.dwType == RIM_TYPEMOUSE) {
      const auto flags = raw->data.mouse.usButtonFlags;
      OnButton(0, flags, RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_1_UP);
      OnButton(1, flags, RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_2_UP);
      OnButton(2, flags, RI_MOUSE_BUTTON_3_DOWN, RI_MOUSE_BUTTON_3_UP);
      OnButton(3, flags, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP);
      OnButton(4, flags, RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP);
    }
  }
}

static LRESULT __stdcall WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_CSGO_CLOSE:
    OnClose();
    return 0;
  case WM_CSGO_ERROR:
    OnError(reinterpret_cast<const char*>(wparam), reinterpret_cast<const char*>(lparam));
    return 0;
  case WM_KEYUP:
  case WM_KEYDOWN:
    OnKey(static_cast<UINT_PTR>(wparam), msg == WM_KEYDOWN);
    break;
  case WM_INPUT:
    OnInput(reinterpret_cast<HRAWINPUT>(lparam));
    break;
  }
  return CallWindowProc(proc, hwnd, msg, wparam, lparam);
}

DWORD WINAPI code_create(LPVOID param) {
  if (!initialize()) {
    Sleep(100);
    return 1;
  }
  proc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc)));
  info("version %d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  code = std::make_unique<Code>();
  client_hook = std::make_unique<hook>(client, true);
  engine_vgui_hook = std::make_unique<hook>(engine_vgui, true);
  client_mode_hook = std::make_unique<hook>(client_mode, true);
  PaintOrig = engine_vgui_hook->Hook(14, reinterpret_cast<PaintProc>(PaintHook));
  CreateMoveOrig = client_mode_hook->Hook(24, reinterpret_cast<CreateMoveProc>(CreateMoveHook));
  FrameStageNotifyOrig = client_hook->Hook(36, reinterpret_cast<FrameStageNotifyProc>(FrameStageNotifyHook));
  return 0;
}

DWORD WINAPI code_delete() {
  client_mode_hook.reset();
  engine_vgui_hook.reset();
  client_hook.reset();
  code.reset();
  if (auto orig = std::exchange(proc, nullptr)) {
    SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(orig));
  }
  window = nullptr;
  return 0;
}

}  // namespace csgo

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
  switch (reason) {
  case DLL_QUERY_HMODULE:
    if (reserved) {
      *reinterpret_cast<HMODULE*>(reserved) = instance;
    }
    break;
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(instance);
    CreateThread(nullptr, 0, csgo::code_create, instance, 0, nullptr);
    break;
  case DLL_PROCESS_DETACH:
    csgo::code_delete();
    break;
  case DLL_THREAD_ATTACH:
    break;
  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}