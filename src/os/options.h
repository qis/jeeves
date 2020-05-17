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
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

namespace os {

class options {
public:
  options(int argc, char** argv, std::string_view format = {}) {
    parse(argc, argv, format);
  }

#ifdef _WIN32
  options(PCWSTR cmd, std::string_view format = {}) {
    auto argc = 0;
    auto argv = CommandLineToArgvW(cmd, &argc);
    for (auto i = 0; i < argc; i++) {
      std::string arg;
      arg.resize(WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, nullptr, 0, nullptr, nullptr) + 1);
      arg.resize(WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, arg.data(), static_cast<int>(arg.size()), nullptr, nullptr));
      args_.push_back(std::move(arg));
    }
    std::vector<char*> ptr;
    for (auto& e : args_) {
      ptr.push_back(e.data());
    }
    parse(argc, ptr.data(), format);
  }
#endif

  bool has(char c) const {
    return std::find(flags_.begin(), flags_.end(), c) != flags_.end() || options_.find(c) != options_.end();
  }

  std::string get(char c) const {
    const auto it = options_.find(c);
    if (it != options_.end()) {
      return std::string(it->second);
    }
    throw std::runtime_error("Missing command line option: -" + std::string(&c, 1));
  }

  std::string get(char c, std::string_view value) const {
    const auto it = options_.find(c);
    if (it != options_.end()) {
      return std::string(it->second);
    }
    return std::string(value);
  }

  std::size_t size() const {
    return arguments_.size();
  }

  std::string operator[](std::size_t index) const {
    if (index >= arguments_.size()) {
      throw std::runtime_error("Missing " + index_name(index) + " command line argument.");
    }
    return std::string(arguments_[index]);
  }

private:
  void parse(int argc, char** argv, std::string_view format) {
    int i = 1;
    for (; i < argc; i++) {
      std::string_view s(argv[i]);
      if (s.empty() || s[0] != '-') {
        break;
      }
      if (s.size() == 2 && s[1] == '-') {
        i++;
        break;
      }
      for (std::size_t j = 1; j < s.size(); j++) {
        const auto c = s[j];
        const auto pos = format.find(c);
        if (pos == std::string::npos) {
          throw std::runtime_error("Unknown command line option: -" + std::string(&c, 1));
        }
        if (pos + 1 < format.size() && format[pos + 1] == ':') {
          if (j + 1 < s.size()) {
            options_[c] = s.substr(j + 1);
          } else if (i + 1 < argc) {
            options_[c] = argv[++i];
          } else {
            throw std::runtime_error("Missing command line option value: -" + std::string(&c, 1));
          }
          break;
        }
        flags_.push_back(c);
      }
    }
    for (; i < argc; i++) {
      arguments_.emplace_back(argv[i]);
    }
  }

  static std::string index_name(std::size_t index) {
    const auto str = std::to_string(++index);
    if (index < 10 || index > 20) {
      switch (index % 10) {
      case 1: return str + "st";
      case 2: return str + "nd";
      case 3: return str + "rd";
      }
    }
    return str + "th";
  }

  std::vector<char> flags_;
  std::map<char, std::string_view> options_;
  std::vector<std::string_view> arguments_;
#ifdef _WIN32
  std::vector<std::string> args_;
#endif
};

}  // namespace os