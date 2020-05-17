#include "module.h"
#include <os/application.h>
#include <os/process.h>
#include <csgo/xorstr.h>
#include <config.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#define WM_CSGO_CLOSE (WM_APP + 0xB000)

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR cmd, int show) {
  try {
    const auto window = FindWindowA(xorstr("Valve001"), nullptr);
    if (!window) {
      throw std::runtime_error(xorstr("Could not find CS:GO window."));
    }
    SendMessage(window, WM_CSGO_CLOSE, 0, 0);
    
    const auto path = os::application::path().parent_path() / xorstr("code.dll");
    if (!std::filesystem::exists(path)) {
      throw std::runtime_error(xorstr("could not find file: ") + path.u8string());
    }

    std::string buffer;
    std::ifstream is(path, std::ios::binary | std::ios::ate);
    buffer.resize(static_cast<std::size_t>(is.tellg()));
    is.seekg(0, std::ios::beg);
    if (!is.read(buffer.data(), buffer.size())) {
      throw std::runtime_error(xorstr("could not read file: ") + path.u8string());
    }
    is.close();
    const auto data = reinterpret_cast<std::uint8_t*>(buffer.data());
    const auto size = buffer.size();

    HANDLE token = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
      throw std::system_error({ static_cast<int>(GetLastError()), std::system_category() }, xorstr("open process token"));
    }

    os::process process;
    for (const auto& pe : os::process_snapshot(TH32CS_SNAPPROCESS, 0)) {
      if (std::wstring_view(pe.szExeFile) == L"csgo.exe") {
        process = { PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pe.th32ProcessID };
        break;
      }
    }
    if (!process) {
      throw std::runtime_error(xorstr("could not find process"));
    }

#ifndef _DEBUG
    const auto thread = LoadRemoteLibraryR(process, data, size, nullptr);
#else
    auto dll = path.u8string();
    auto load_library = reinterpret_cast<decltype(&LoadLibraryA)>(GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA"));
    if (!load_library) {
      throw std::runtime_error("could not get LoadLibraryA address");
    }
    auto memory = VirtualAllocEx(process, nullptr, dll.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!memory) {
      throw std::runtime_error("could not allocate remote memory");
    }
    if (!WriteProcessMemory(process, memory, dll.data(), dll.size() + 1, nullptr)) {
      throw std::runtime_error("could not write remote memory");
    }
    const auto thread = CreateRemoteThread(process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(load_library), memory, 0, nullptr);
#endif
    if (!thread) {
      throw std::system_error({ static_cast<int>(GetLastError()), std::system_category() }, path.filename().u8string());
    }
    CloseHandle(thread);
  }
  catch (const std::exception& e) {
    std::wstring msg;
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, nullptr, 0) + 1);
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, msg.data(), static_cast<int>(msg.size())));
    MessageBox(nullptr, msg.data(), TEXT(PROJECT" Error"), MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    return 1;
  }
  return 0;
}