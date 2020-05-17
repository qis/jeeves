#pragma once
#include <windows.h>
#include <csgo/sdk.h>
#include <csgo/hook.h>
#include <csgo/math.h>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace csgo {

class Code {
public:
  Code();

  void Qis(CUserCmd* cmd, int id, Player* player, Weapon* weapon);
  void Aim(CUserCmd* cmd, int id, Player* player, Weapon* weapon);
  void Move(CUserCmd* cmd, int id, Player* player, Weapon* weapon);
  void Reveal(CUserCmd* cmd, int id, Player* player, Weapon* weapon);
  void Trigger(CUserCmd* cmd, int id, Player* player, Weapon* weapon);
  void CreateMove(float sample_input_frametime, CUserCmd* cmd);

  void PaintPlayer(int id, Player* player);
  void PaintNade(ClientClass* client, ClientEntity* entity);
  void PaintBomb(ClientEntity* entity, bool planted, std::optional<Color> defusing);
  void PaintStatus();
  void PaintCursor();
  void Paint(int mode);

  void SetFlashMaxAlpha();
  void EnablePostProcessing(bool enable);
  void FrameStageNotify(ClientFrameStage stage);

  void OnKey(int key, bool down);
  void OnButton(int button, bool down);

private:
  HFont hud_font_;
  HFont esp_font_;
  Player* self_ = nullptr;
  int team_ = 0;
  float time_ = 0.0f;
  int cx_ = 2560;
  int cy_ = 1080;
  std::array<QAngle, 2> punch_;
  int punch_offset_ = 0;
  float speed_ = 0.0f;
  bool qis_ = false;
};

}  // namespace csgo