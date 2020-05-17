#pragma once
#include <windows.h>
#include <stdio.h>
#include <map>

namespace csgo {

class hook {
public:
  hook(void* base, bool replace) : base_(reinterpret_cast<PDWORD*>(base)), replace_(replace) {
    if (replace) {
      table_old_ = *reinterpret_cast<PDWORD*>(base);
      const auto length = CalculateLength();
      table_new_ = new DWORD[length];
      std::memcpy(table_new_, table_old_, length * sizeof(DWORD));
      DWORD old = 0;
      VirtualProtect(base_, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old);
      *base_ = table_new_;
      VirtualProtect(base_, sizeof(DWORD), old, &old);
    } else {
      table_old_ = *reinterpret_cast<PDWORD*>(base);
      table_new_ = *reinterpret_cast<PDWORD*>(base);
    }
  }

  hook(const hook& other) = delete;
  hook& operator=(const hook& other) = delete;

  ~hook() {
    RestoreTable();
    if (replace_ && table_new_) {
      delete[] table_new_;
    }
  }

  void RestoreTable() {
    if (replace_) {
      DWORD old;
      VirtualProtect(base_, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &old);
      *base_ = table_old_;
      VirtualProtect(base_, sizeof(DWORD), old, &old);
    } else {
      for (auto& pair : hooked_indexes_) {
        Unhook(pair.first);
      }
    }
  }

  template <class T>
  T Hook(uint32_t index, T function) {
    const auto old = table_old_[index];
    table_new_[index] = reinterpret_cast<DWORD>(function);
    hooked_indexes_.insert(std::make_pair(index, old));
    return reinterpret_cast<T>(old);
  }

  void Unhook() {
    for (auto& e : hooked_indexes_) {
      table_new_[e.first] = e.second;
    }
    hooked_indexes_.clear();
  }

  void Unhook(uint32_t index) {
    auto it = hooked_indexes_.find(index);
    if (it != hooked_indexes_.end()) {
      table_new_[index] = it->second;
      hooked_indexes_.erase(it);
    }
  }

  template <class T>
  T GetOriginal(uint32_t index) {
    return reinterpret_cast<T>(table_old_[index]);
  }

private:
  uint32_t CalculateLength() {
    uint32_t index = 0;
    if (!table_old_) {
      return 0;
    }
    for (index = 0; table_old_[index]; index++) {
      if (IsBadCodePtr(reinterpret_cast<FARPROC>(table_old_[index]))) {
        break;
      }
    }
    return index;
  }

private:
  std::map<uint32_t, DWORD> hooked_indexes_;

  PDWORD* base_ = nullptr;
  PDWORD table_old_ = nullptr;
  PDWORD table_new_ = nullptr;
  bool replace_ = true;
};

}  // namespace csgo