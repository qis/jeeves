#pragma once

namespace csgo {
namespace input {

enum buttons : int {
  left = 0,
  right,
  middle,
  down,
  up,
};

extern bool button[5];
extern bool key[0xFF];

}  // namespace input
}  // namespace