// ISC License
//
// Copyright (c) 2017 Alexej Harm
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS AL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#pragma once
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <filesystem>
#include <iterator>
#include <string>
#include <system_error>
#include <utility>

namespace os {

class process {
public:
  using handle = HANDLE;

  process() noexcept = default;

  process(DWORD access, BOOL inherit_handle, DWORD id) {
    open(access, inherit_handle, id);
  }

  process(process&& other) noexcept : handle_(std::exchange(other.handle_, invalid_handle_value())) {
  }

  process& operator=(process&& other) noexcept {
    close();
    handle_ = std::exchange(other.handle_, invalid_handle_value());
    return *this;
  }

  ~process() {
    close();
  }

  operator bool() const noexcept {
    return handle_ != invalid_handle_value();
  }

  operator handle() const noexcept {
    return handle_;
  }

  void open(DWORD access, BOOL inherit_handle, DWORD id, std::error_code& ec) noexcept {
    close();
    handle_ = OpenProcess(access, inherit_handle, id);
    ec = std::error_code(GetLastError(), std::system_category());
  }

  void open(DWORD access, BOOL inherit_handle, DWORD id) {
    std::error_code ec;
    open(access, inherit_handle, id, ec);
    if (ec) {
      throw std::system_error(ec, "os::process::open");
    }
  }

  void close() noexcept {
    const auto handle = std::exchange(handle_, invalid_handle_value());
    if (handle != invalid_handle_value()) {
      CloseHandle(handle);
    }
  }

  std::filesystem::path path(std::error_code& ec) const noexcept {
    DWORD size = 0;
    std::wstring path;
    do {
      path.resize(path.size() + MAX_PATH);
      size = GetProcessImageFileName(handle_, path.data(), static_cast<DWORD>(path.size()));
      if (size == 0) {
        const auto ev = GetLastError();
        if (ev != ERROR_INSUFFICIENT_BUFFER) {
          ec = { static_cast<int>(ev), std::system_category() };
          return {};
        }
      }
    } while (size == 0);
    path.resize(size);
    return std::filesystem::absolute(path);
  }

  std::filesystem::path path() const {
    std::error_code ec;
    auto ret = path(ec);
    if (ec) {
      throw std::system_error(ec, "os::process::path");
    }
    return ret;
  }

  static constexpr handle invalid_handle_value() noexcept {
    return nullptr;
  }

private:
  handle handle_ = invalid_handle_value();
};

class process_snapshot {
public:
  class iterator {
  public:
    using value_type = PROCESSENTRY32;
    using iterator_category = std::input_iterator_tag;
    using pointer = const value_type*;
    using reference = const value_type&;

    iterator() noexcept = default;

    explicit iterator(process_snapshot* snapshot) noexcept : snapshot_(snapshot) {
    }

    reference operator*() const noexcept {
      return snapshot_->entry_;
    }

    iterator& operator++() noexcept {
      if (!snapshot_->next()) {
        snapshot_ = nullptr;
      }
      return *this;
    }

    iterator operator++(int) noexcept {
      return ++iterator(*this);
    }

    bool operator==(const iterator& rhs) noexcept {
      return snapshot_ == rhs.snapshot_;
    }

    bool operator!=(const iterator& rhs) noexcept {
      return snapshot_ != rhs.snapshot_;
    }

  private:
    process_snapshot* snapshot_ = nullptr;
  };

  process_snapshot() noexcept {
    entry_.dwSize = sizeof(entry_);
  }

  process_snapshot(DWORD flags, DWORD id) : process_snapshot() {
    open(flags, id);
  }

  process_snapshot(process_snapshot&& other) noexcept : handle_(std::exchange(other.handle_, invalid_handle_value())), entry_(other.entry_) {
  }

  process_snapshot& operator=(process_snapshot&& other) noexcept {
    close();
    handle_ = std::exchange(other.handle_, invalid_handle_value());
    entry_ = other.entry_;
    return *this;
  }

  ~process_snapshot() {
    close();
  }

  operator bool() const noexcept {
    return handle_ != invalid_handle_value();
  }

  void open(DWORD flags, DWORD id, std::error_code& ec) noexcept {
    close();
    handle_ = CreateToolhelp32Snapshot(flags, id);
    ec = std::error_code(GetLastError(), std::system_category());
  }

  void open(DWORD flags, DWORD id) {
    std::error_code ec;
    open(flags, id, ec);
    if (ec) {
      throw std::system_error(ec, "os::process_snapshot::open");
    }
  }

  void close() noexcept {
    const auto handle = std::exchange(handle_, invalid_handle_value());
    if (handle != invalid_handle_value()) {
      CloseHandle(handle);
    }
  }

  iterator begin() noexcept {
    return first() ? iterator(this) : iterator();
  }

  iterator end() noexcept {
    return {};
  }

  constexpr static HANDLE invalid_handle_value() noexcept {
    return INVALID_HANDLE_VALUE;
  }

private:
  BOOL first() noexcept {
    return Process32First(handle_, &entry_);
  }

  BOOL next() noexcept {
    return Process32Next(handle_, &entry_);
  }

  HANDLE handle_ = invalid_handle_value();
  PROCESSENTRY32 entry_ = {};
};

}  // namespace os