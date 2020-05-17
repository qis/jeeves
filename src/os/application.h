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
#include <filesystem>
#include <string>
#include <system_error>

namespace os::application {

std::filesystem::path path(std::error_code& ec) noexcept {
  DWORD size = 0;
  std::wstring path;
  do {
    path.resize(path.size() + MAX_PATH);
    size = GetModuleFileName(nullptr, path.data(), static_cast<DWORD>(path.size()));
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

std::filesystem::path path() {
  std::error_code ec;
  auto ret = path(ec);
  if (ec) {
    throw std::system_error(ec, "os::application::path");
  }
  return ret;
}

}  // namespace os::application