#include "code.h"
#include "input.h"
#include <iomanip>
#include <utility>
#include <sstream>

namespace csgo {
namespace settings {

constexpr auto triggerbot_button = input::up;

auto enable = true;
auto tgr = true;
auto aim = true;

}  // namespace settings

namespace color {

const auto friendly = Color(130, 200, 245);
const auto hostile = Color(245, 50, 0);

}  // namespace color

Code::Code() {
  hud_font_ = surface->CreateFont();
  surface->SetFontGlyphSet(hud_font_, "Segoe UI", 20, 2, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
  esp_font_ = surface->CreateFont();
  surface->SetFontGlyphSet(esp_font_, "DejaVu Sans", 12, 6, 0, 0, FONTFLAG_DROPSHADOW);
  punch_[0] = { 0.0f, 0.0f, 0.0f };
  punch_[1] = { 0.0f, 0.0f, 0.0f };
}

static std::vector<std::array<float, 2>> load() {
  // text.svg 100x100 pixel canvas with text
  //std::string_view src = "M37.035,32.249L35.397,30.815L33.451,29.586L30.994,28.562L28.741,28.102L26.437,27.999L24.082,27.999L21.727,28.358L19.32,28.972L16.965,30.15L15.173,31.481L13.688,32.914L12.306,34.758L11.384,36.857L10.719,38.854L10.104,41.209L9.848,43.564L9.695,45.663L9.643,48.121L9.797,50L9.951,52.063L10.155,54.06L10.616,55.801L11.179,57.49L11.896,59.078L12.664,60.46L13.791,61.791L14.917,62.969L16.402,64.146L17.887,64.966L19.627,65.631L21.573,66.092L23.314,66.399L25.72,66.45L27.615,66.246L29.816,65.785L32.12,65.068L34.168,64.044L35.653,64.095L37.138,65.324L38.52,66.553L40.107,67.577L41.695,68.652L43.231,69.574L44.613,70.393L46.303,71.161L35.96,62.764L37.394,61.279L38.469,59.538L39.442,57.9L40.261,55.75L40.773,53.292L41.029,50.578L41.08,48.07L41.131,45.612L40.978,43.257L40.619,40.799L40.107,38.188L39.442,36.345L38.264,34.143L56.031,28.153L56.031,30.457L55.979,32.761L55.979,34.809L56.031,37.369L56.031,44.69L55.979,47.199L55.979,49.247L55.928,51.756L55.928,53.958L55.979,56.262L55.928,58.361L55.877,60.921L55.928,63.327L55.928,65.273L55.979,66.758L69.803,63.839L71.749,64.914L73.643,65.529L75.691,66.143L77.842,66.399L79.839,66.502L82.245,66.194L84.754,65.375L86.853,64.249L88.287,62.61L89.362,60.972L89.823,59.026L89.925,56.364L89.618,54.214L88.594,52.268L87.263,50.732L85.727,49.452L83.781,48.121L81.631,46.994L79.736,45.919L77.637,44.742L76.101,43.666L74.667,42.386L73.387,40.953L72.466,39.366L71.8,37.266L71.903,35.014L72.466,32.914L73.49,31.122L75.128,29.638L76.767,28.818L78.712,28.255L80.555,28.102L82.808,28.306L84.549,28.511L86.239,29.023L87.672,29.638L89.208,30.406L56.594,24.006L57.208,23.34L56.287,22.879L55.775,23.545";
  //std::string_view src = "M25.341,27.862L20.887,28.425L17.303,29.859L13.924,32.829L12.132,35.593L10.596,39.382L9.725,43.273L9.674,47.421L9.879,51.005L10.289,54.281L11.313,57.712L13.002,60.579L15.357,63.395L18.02,65.136L21.911,66.313L25.905,66.467L29.898,65.955L33.277,64.521L35.94,62.627L38.193,60.118L39.319,57.763L40.343,54.998L41.06,51.67L41.521,48.086L41.265,44.809L40.753,40.201L39.677,37.232L38.449,34.006L36.247,31.395L33.277,29.501L29.898,28.169L26.263,27.862L35.735,63.753L37.578,65.75L39.78,67.286L41.879,68.925L44.081,70.102L46.487,71.177L56.727,66.109L56.727,63.753L56.778,61.04L56.778,58.429L56.727,55.92L56.727,53.001L56.778,50.493L56.778,47.779L56.727,44.4L56.778,41.584L56.727,38.512L56.778,36.054L56.829,33.033L56.778,29.859L56.727,27.657L88.778,30.013L85.45,28.528L81.969,27.913L78.589,27.965L76.439,28.477L73.981,30.064L72.036,32.47L71.473,35.542L71.831,38.768L73.674,41.993L76.593,44.144L80.74,46.294L84.119,48.035L87.498,50.595L90.109,53.411L90.621,56.585L89.905,60.067L88.573,62.576L86.269,64.573L83.505,65.853L81.252,66.518L78.845,66.467L75.722,66.109L72.957,65.392L71.063,64.47L69.22,63.19L55.997,22.55L57.021,22.422L56.573,21.718";
  std::string_view src = "M25.341,27.862L20.887,28.425L17.303,29.859L13.924,32.829L12.132,35.593L10.596,39.382L9.725,43.273L9.674,47.421L9.879,51.005L10.289,54.281L11.313,57.712L13.002,60.579L15.357,63.395L18.02,65.136L21.911,66.313L25.905,66.467L29.898,65.955L33.277,64.521L35.94,62.627L38.193,60.118L39.319,57.763L40.343,54.998L41.06,51.67L41.521,48.086L41.265,44.809L40.753,40.201L39.677,37.232L38.449,34.006L36.247,31.395L33.277,29.501L29.898,28.169L26.263,27.862L35.735,63.753L37.578,65.75L39.78,67.286L41.879,68.925L44.081,70.102L46.487,71.177L56.727,66.109L56.727,63.753L56.778,61.04L56.778,58.429L56.727,55.92L56.727,53.001L56.778,50.493L56.778,47.779L56.727,44.4L56.778,41.584L56.727,38.512L56.778,36.054L56.829,33.033L56.778,29.859L56.727,27.657L88.778,30.013L85.45,28.528L81.969,27.913L78.589,27.965L76.439,28.477L73.981,30.064L72.036,32.47L71.473,35.542L71.831,38.768L73.674,41.993L76.593,44.144L80.74,46.294L84.119,48.035L87.498,50.595L90.109,53.411L90.621,56.585L89.905,60.067L88.573,62.576L86.269,64.573L83.505,65.853L81.252,66.518L78.845,66.467L75.722,66.109L72.957,65.392L71.063,64.47L69.22,63.19L2,22L2,20L4,20L96,80L98,80L98,78L2,78L2,80L4,80L98,22L98,20L96,20";
  std::size_t beg = 0;
  std::vector<std::array<float, 2>> image;
  image.push_back({ 0.0f, -9.0f });
  while (true) {
    const auto end = src.find_first_of("ML", beg + 1);
    if (end == std::string_view::npos) {
      break;
    }
    const auto type = src[beg];
    const auto text = src.substr(beg + 1, end - beg - 1);
    const auto part = text.find(',');
    if (part == std::string_view::npos) {
      break;
    }
    float x = 0.0f;
    float y = 0.0f;
    if (std::sscanf(text.data(), "%f,%f", &x, &y) != 2) {
      break;
    }
    image.push_back({ x / 10.0f - 5.0f, -(y / 10.0f - 5.0f) });
    beg = end;
  }
  return image;
}

void Code::Qis(CUserCmd* cmd, int id, Player* player, Weapon* weapon) {
  constexpr auto start = 23;
  static auto shoot = false;
  static auto angle = QAngle();
  static auto image = load();
  if (weapon->GetId() != weapon_negev) {
    shoot = false;
    qis_ = false;
    return;
  }
  cmd->buttons |= IN_ATTACK;
  if (!shoot || player->GetShotsFired() < start) {
    if (!shoot) {
      shoot = true;
      angle = cmd->viewangles;
    }
    cmd->viewangles.y = angle.y - image[0][0] * 4.0f;
    cmd->viewangles.x = angle.x - image[0][1] * 4.0f;
    return;
  }
  if (weapon->GetNextPrimaryAttack() - globals->curtime < 0.0f) {
    shoot = false;
    qis_ = false;
    return;
  }
  if (player->GetShotsFired() - start >= static_cast<int>(image.size())) {
    cmd->buttons &= ~IN_ATTACK;
    shoot = false;
    qis_ = false;
    return;
  }
  const auto index = player->GetShotsFired() - start;
  cmd->viewangles.y = angle.y - image[index][0] * 4.0f;
  cmd->viewangles.x = angle.x - image[index][1] * 4.0f;
}

void Code::Aim(CUserCmd* cmd, int id, Player* player, Weapon* weapon) {
  // Compensate recoil.
  auto recoil_compensation = false;
  switch (weapon->GetType()) {
  //case weapon_type_pistol:
  //  recoil_compensation = true;
  //  break;
  case weapon_type_assault_rifle:
  case weapon_type_zoom_rifle:
  case weapon_type_auto_sniper:
  case weapon_type_machine_pistol:
    recoil_compensation = player->GetShotsFired() > 0;
    break;
  case weapon_type_machine_gun:
    recoil_compensation = player->GetShotsFired() > 3;
    break;
  }
  if (recoil_compensation && punch_[0].IsValid() && punch_[1].IsValid()) {
    cmd->viewangles.x -= (punch_[0].x - punch_[1].x) * 2.0f;
    cmd->viewangles.y -= (punch_[0].y - punch_[1].y) * 1.2f;
    Math::NormalizeAngles(cmd->viewangles);
    Math::ClampAngles(cmd->viewangles);
  }
}

void Code::Move(CUserCmd* cmd, int id, Player* player, Weapon* weapon) {
  if (speed_ < 100.f || cmd->buttons & IN_ATTACK || cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT) {
    return;
  }
  const auto move_type = player->GetMoveType();
  if (move_type == MOVETYPE_LADDER || move_type == MOVETYPE_NOCLIP) {
    return;
  }
  const auto flags = player->GetFlags();

  // Strafe.
  QAngle angles;
  engine->GetViewAngles(angles);
  static auto side_switch = false;
  if (!(flags & FL_ONGROUND)) {
    const auto yaw = 30.0f * fabsf(30.0f / speed_);
    if (side_switch || cmd->mousedx > 1) {
      angles.y += yaw;
      cmd->sidemove = 450.0f;
    } else if (!side_switch || cmd->mousedx < 1) {
      angles.y -= yaw;
      cmd->sidemove = -450.0f;
    }
    side_switch = !side_switch;
  }
  Math::NormalizeAngles(angles);
  Math::ClampAngles(angles);
  Math::CorrectMovement(angles, cmd, cmd->forwardmove, cmd->sidemove);

  // Jump.
  static bool last_jumped = false;
  static bool should_fake = false;
  if (!last_jumped && should_fake) {
    should_fake = false;
    cmd->buttons |= IN_JUMP;
  } else if (cmd->buttons & IN_JUMP) {
    if (player->GetFlags() & FL_ONGROUND) {
      last_jumped = true;
      should_fake = true;
    } else {
      cmd->buttons &= ~IN_JUMP;
      last_jumped = false;
    }
  } else {
    last_jumped = false;
    should_fake = false;
  }
}

void Code::Reveal(CUserCmd* cmd, int id, Player* player, Weapon* weapon) {
  if (!(cmd->buttons & IN_SCORE)) {
    return;
  }
  float msg[3] = { 0.0f, 0.0f, 0.0f };
  MsgFunc_ServerRankRevealAll(msg);
}

void Code::Trigger(CUserCmd* cmd, int id, Player* player, Weapon* weapon) {
  static auto shoot = false;
  if (!settings::tgr || punch_offset_ > 3 || cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2) {
    return;
  }

  auto trigger = false;
  switch (weapon->GetType()) {
  case weapon_type_pistol:
    if (shoot) {
      cmd->buttons &= ~IN_ATTACK;
    }
    shoot = false;
    [[fallthrough]];
  case weapon_type_assault_rifle:
  case weapon_type_zoom_rifle:
  case weapon_type_auto_sniper:
  case weapon_type_machine_gun:
  case weapon_type_machine_pistol:
  case weapon_type_sniper:
  case weapon_type_shotgun:
    trigger = true;
    break;
  }

  if (!trigger) {
    if (shoot) {
      cmd->buttons &= ~IN_ATTACK;
      shoot = false;
    }
    return;
  }

  const auto next = weapon->GetNextPrimaryAttack() - globals->curtime;
  
  if (shoot) {
    const auto shot = player->GetShotsFired();
    if ((shot > 0 && shot < 3) && (next > 0.0f && next < 0.5f)) {
      cmd->buttons |= IN_ATTACK;
      return;
    }
    cmd->buttons &= ~IN_ATTACK;
    shoot = false;
  }

  if (weapon->GetId() == weapon_deagle && next >= -0.4f) {
    return;
  }

  QAngle src = cmd->viewangles + punch_[0] * 2.0f;
  const auto eyes = Math::ExtrapolatePosition(player, player->GetEyePosition());

  Vector target;
  Math::AngleVectors(src, target);
  target = eyes + target * 8192.0f;

  CTraceFilter filter;
  filter.pSkip = player;
  trace_t tr;
  //Ray_t ray;
  //ray.Init(eyes, target);
  //trace->TraceRay(ray, 0x46004003, &filter, &tr);

  auto trace_length = 0.0f;
  auto pos = eyes;
  for (auto penetration = 0; penetration < 4; penetration++) {
    Ray_t ray;
    ray.Init(pos, target);
    trace->TraceRay(ray, 0x46004003, &filter, &tr);
    if (!tr.m_pEnt) {
      break;
    }
    if (const auto entity_class = tr.m_pEnt->GetClientClass()) {
      if (entity_class->m_ClassID == ClassId::CCSPlayer) {
        break;
      }
    }
    pos = tr.endpos;
    filter.pSkip = tr.m_pEnt;
  }


  if (auto entity = static_cast<Player*>(tr.m_pEnt)) {
    const auto entity_class = entity->GetClientClass();
    if (!entity_class) {
      return;
    }
    const auto entity_class_id = entity_class->m_ClassID;
    if (entity_class_id != ClassId::CCSPlayer) {
      return;
    }
    if (entity->GetTeam() == player->GetTeam()) {
      cmd->buttons &= ~IN_ATTACK;
      return;
    }
    if (input::button[input::buttons::left]) {
      return;
    }
    if (tr.hitgroup && tr.hitgroup < HITGROUP_GEAR) {
      if (weapon->GetId() == weapon_ssg08 || (weapon->GetType() == weapon_type_pistol && weapon->GetId() != weapon_revolver)) {
        if (tr.hitgroup != HITGROUP_HEAD) {
          return;
        }
      }
      if (next < 0.1) {
        cmd->buttons |= IN_ATTACK;
        shoot = true;
      }
      if (settings::aim && shoot && tr.hitgroup == HITGROUP_HEAD) {
        const auto time = engine->GetLastTimeStamp();
        matrix3x4_t bones[128];
        if (entity->SetupBones(bones, 128, 0x00000100, time)) {
          const auto head = Vector(bones[Bone::Head][0][3], bones[Bone::Head][1][3], bones[Bone::Head][2][3]);
          cmd->viewangles = Math::CalcAngle(eyes, Math::ExtrapolatePosition(entity, head));
          Math::NormalizeAngles(cmd->viewangles);
          Math::ClampAngles(cmd->viewangles);
        }
      }
    }
  }
}

void Code::CreateMove(float sample_input_frametime, CUserCmd* cmd) {
  if (!settings::enable) {
    return;
  }
  const auto id = engine->GetLocalPlayer();
  const auto player = static_cast<Player*>(entities->GetClientEntity(id));
  if (!player || !player->IsAlive()) {
    return;
  }
  const auto weapon = player->GetActiveWeapon();
  if (!weapon) {
    return;
  }

  // Store punch for use by aim and paint functions.
  if (const auto punch = player->GetAimPunchAngle()) {
    punch_[1] = std::exchange(punch_[0], *punch);
  } else {
    punch_[0] = { 0.0f, 0.0f, 0.0f };
    punch_[1] = { 0.0f, 0.0f, 0.0f };
  }

  // Store speed for use by move and paint functions.
  auto velocity = player->GetVelocity();
  velocity.z = 0.0f;
  speed_ = velocity.Length();

  if (qis_) {
    Qis(cmd, id, player, weapon);
    //cmd->viewangles.x -= (punch_[0].x - punch_[1].x) * 2.0f;
    //cmd->viewangles.y -= (punch_[0].y - punch_[1].y) * 2.0f;
    Math::NormalizeAngles(cmd->viewangles);
    Math::ClampAngles(cmd->viewangles);
  } else {
    Aim(cmd, id, player, weapon);
    Move(cmd, id, player, weapon);
    Reveal(cmd, id, player, weapon);
    Trigger(cmd, id, player, weapon);
  }

  // Pistol spray.
  if (weapon->GetType() == weapon_type_pistol && weapon->GetNextPrimaryAttack() - globals->curtime > 0.0f) {
    switch (weapon->GetId()) {
    case weapon_revolver:
      cmd->buttons &= ~IN_ATTACK2;
      break;
    default:
      cmd->buttons &= ~IN_ATTACK;
      break;
    }
  }

  // Use secondary fire with revolvers.
  if (cmd->buttons & IN_ATTACK && weapon->GetId() == weapon_revolver) {
    cmd->buttons &= ~IN_ATTACK;
    cmd->buttons |= IN_ATTACK2;
  }
}

void Code::PaintPlayer(int id, Player* player) {
  if (!player->IsAlive() || player->IsDormant()) {
    return;
  }
  matrix3x4_t bones[128];
  if (!player->SetupBones(bones, 128, 0x00000100, time_)) {
    return;
  }
  const auto head = Vector(bones[Bone::Head][0][3], bones[Bone::Head][1][3], bones[Bone::Head][2][3]);
  const auto feet = Vector(bones[Bone::Feet][0][3], bones[Bone::Feet][1][3], bones[Bone::Feet][2][3]);
  Vector screen_head;
  Vector screen_feet;
  if (!Math::WorldToScreen(head, screen_head) || !Math::WorldToScreen(feet, screen_feet)) {
    return;
  }
  const auto color = player->GetTeam() != team_ ? color::hostile : color::friendly;
  const auto is_crouched = (player->GetFlags() & static_cast<int>(EntityFlags::FL_DUCKING)) != 0;
  surface->DrawSetColor(color);
  const auto cy = static_cast<int>(std::abs(screen_head.y - screen_feet.y));
  const auto cx = static_cast<int>(cy * (is_crouched ? 0.65f : 0.50f));
  const auto sx = static_cast<int>(screen_head.x - cx / 2.0f);
  const auto sy = static_cast<int>(screen_head.y);
  surface->DrawOutlinedRect(sx, sy + cy / 8, sx + cx, sy + cy);
  const auto head_x = static_cast<int>(screen_head.x);
  const auto head_y = static_cast<int>(screen_head.y);
  surface->DrawOutlinedCircle(head_x, head_y, cy / 8, 24);
  surface->DrawLine(sx + cx - 6, sy + cy / 8, sx + cx - 6, sy + cy);
  surface->DrawLine(sx + cx - 5, sy + cy / 8 + 5, sx + cx - 1, sy + cy / 8 + 5);

  const auto helmet = player->HasHelmet();
  const auto health = player->GetHealth();
  const auto armor = player->GetArmor();
  const auto max = (sy + cy) - (sy + cy / 8) - 2 - 5;
  const auto cur = health * max / 100;
  const auto arm = armor * cur / 100;
  surface->DrawSetColor(Color::IndianRed());
  surface->DrawFilledRect(sx + cx - 5, sy + cy - 1 - cur, sx + cx - 1, sy + cy - 1);
  surface->DrawSetColor(Color::YellowGreen());
  surface->DrawFilledRect(sx + cx - 5, sy + cy - 1 - arm, sx + cx - 1, sy + cy - 1);
  surface->DrawSetColor(helmet ? Color::DodgerBlue() : Color::Black());
  surface->DrawFilledRect(sx + cx - 5, sy + cy / 8 + 1, sx + cx - 1, sy + cy / 8 + 5);

  player_info_t player_info = {};
  if (engine->GetPlayerInfo(id, &player_info)) {
    static std::wstring wname;
    const auto beg = player_info.szName;
    const auto end = std::find(beg, beg + sizeof(player_info.szName), '\0');
    const auto size = static_cast<int>(std::distance(beg, end));
    wname.resize(MultiByteToWideChar(CP_UTF8, 0, beg, size, nullptr, 0) + 1);
    wname.resize(MultiByteToWideChar(CP_UTF8, 0, beg, size, wname.data(), static_cast<int>(wname.size())));
    int text_cx = 0;
    int text_cy = 0;
    surface->GetTextSize(esp_font_, wname.data(), text_cx, text_cy);
    surface->DrawSetTextColor(color);
    surface->DrawSetTextPos(head_x - text_cx / 2, head_y - cy / 8 - text_cy - 2);
    surface->DrawText(wname.data());
  }
}

void Code::PaintNade(ClientClass* client, ClientEntity* entity) {
  auto color = Color::White();
  switch (client->m_ClassID) {
  case CSmokeGrenadeProjectile:
    color = Color::Gray();  // smoke
    break;
  case CMolotovProjectile:
    color = Color::Orange();  // fire
    break;
  default:
    if (const auto index = entity->GetModelIndex()) {
      switch (*index) {
      case 53:
        color = Color::Red();  // he
        break;
      case 54:
        color = Color::DodgerBlue();  // flash
        break;
      }
    }
    break;
  }
  Vector screen;
  if (!Math::WorldToScreen(entity->GetAbsOrigin(), screen)) {
    return;
  }
  const auto x = static_cast<int>(screen.x);
  const auto y = static_cast<int>(screen.y);
  const auto r = 5;
  surface->DrawSetColor(color);
  surface->DrawOutlinedCircle(x, y, r, 8);
}

void Code::PaintBomb(ClientEntity* entity, bool planted, std::optional<Color> defusing) {
  const auto game = GetGame();
  if (!game || (!planted && !game->IsBombDropped())) {
    return;
  }
  Vector screen;
  if (!Math::WorldToScreen(entity->GetAbsOrigin(), screen)) {
    return;
  }
  const auto x = static_cast<int>(screen.x);
  const auto y = static_cast<int>(screen.y);
  const auto bomb = planted ? static_cast<PlantedC4*>(entity) : nullptr;
  const auto timer = bomb ? bomb->GetBombTime() - globals->curtime : 0.0f;

  static Color color;
  static std::wstring text;
  if (planted && bomb && !bomb->IsBombDefused() && timer > 0.0f) {
    static std::wostringstream oss;
    oss.str(L"");
    oss.clear();
    oss << std::fixed << std::showpoint << std::setprecision(1) << timer << L"s";
    color = color::hostile;
    text = oss.str();
    if (defusing) {
      constexpr auto text = L"DEFUSING";
      int cx = 0;
      int cy = 0;
      surface->GetTextSize(esp_font_, text, cx, cy);
      surface->DrawSetTextColor(defusing.value());
      surface->DrawSetTextPos(x - cx / 2, y + cy / 3 + cy);
      surface->DrawText(text);
    }
  } else {
    color = Color::White();
    text = L"BOMB";
  }

  int cx = 0;
  int cy = 0;
  surface->GetTextSize(esp_font_, text.data(), cx, cy);
  surface->DrawSetTextColor(color);
  surface->DrawSetTextPos(x - cx / 2, y - cy / 3);
  surface->DrawText(text.data());
}

void Code::PaintStatus() {
  constexpr auto cx = 353;
  constexpr auto cy = 38;
  constexpr auto pv = 8;
  auto x = cx_ / 2 - cx / 2;
  auto y = cy_ - cy - 10;

  surface->DrawSetColor(Color(0, 0, 0, 220));
  surface->DrawFilledRect(x, y, x + cx, y + cy);

  x += 10;
  y += pv;

  surface->DrawSetTextFont(hud_font_);

  surface->DrawSetTextPos(x, y);
  surface->DrawSetTextColor(Color::White());
  surface->DrawText(L"TGR:");
  x += 35;

  surface->DrawSetTextColor(settings::tgr ? Color::LimeGreen() : Color::IndianRed());
  surface->DrawSetTextPos(x, y);
  surface->DrawText(settings::tgr ? L"ON" : L"OFF");
  x += 29;

  surface->DrawSetTextPos(x, y);
  surface->DrawSetTextColor(Color::White());
  surface->DrawText(L"AIM:");
  x += 35;

  surface->DrawSetTextPos(x, y);
  surface->DrawSetTextColor(settings::aim ? Color::LimeGreen() : Color::IndianRed());
  surface->DrawText(settings::aim ? L"ON" : L"OFF");
  x += 30;

  surface->DrawSetTextPos(x, y);
  surface->DrawSetTextColor(Color::White());
  surface->DrawText(L"SPD:");
  x += 36;
  y += 3;

  constexpr auto speed_max = 330 / 2;
  constexpr auto speed_run = 250 / 2 - 1;
  constexpr auto speed_pv = 1;
  constexpr auto speed_cy = 16;
  constexpr auto speed_cx = speed_max + speed_pv * 2;
  auto speed = static_cast<int>(std::round(speed_ / 2.0f));
  surface->DrawSetColor(Color(255, 255, 255, 20));
  surface->DrawFilledRect(x, y, x + speed_cx, y + speed_cy);
  surface->DrawSetColor(Color(200, 200, 200));
  surface->DrawFilledRect(x + speed_pv, y + speed_pv, x + speed_pv + std::min(speed, speed_run), y + speed_cy - speed_pv);
  if (speed > speed_run + 1) {
    surface->DrawSetColor(Color::IndianRed());
    surface->DrawFilledRect(x + speed_pv + speed_run + 1, y + speed_pv, x + speed_pv + speed_run + 1 + std::min(speed - speed_run, speed_max - speed_run), y + speed_cy - speed_pv);
  }
  surface->DrawSetColor(Color(20, 20, 20));
  surface->DrawLine(x + speed_pv + speed_run, y, x + speed_pv + speed_run, y + speed_cy);
}

void Code::PaintCursor() {
  if (!self_->IsAlive() || !std::isfinite(punch_[0].x) || !std::isfinite(punch_[0].y)) {
    return;
  }
  const auto sx = cx_ / 2;
  const auto sy = cy_ / 2;
  const auto dx = cx_ / 90.0f;
  const auto dy = cy_ / 90.0f;
  const auto x = static_cast<int>(sx - (dx * punch_[0].y));
  const auto y = static_cast<int>(sy + (dy * punch_[0].x));
  punch_offset_ = (std::abs(sx - x) + std::abs(sy - y)) / 2;
  surface->DrawSetColor(Color(255, 0, 0, 150));
  surface->DrawFilledRect(x - 1, y - 1, x + 2, y + 2);
}

void Code::Paint(int mode) {
  if (!settings::enable || !engine->IsInGame() || !engine->IsConnected() || engine->IsTakingScreenshot()) {
    return;
  }
  const auto id = engine->GetLocalPlayer();
  self_ = static_cast<Player*>(entities->GetClientEntity(id));
  if (!self_) {
    return;
  }
  team_ = self_->GetTeam();
  time_ = engine->GetLastTimeStamp();
  surface->DrawSetTextFont(esp_font_);

  std::optional<Color> defusing;
  ClientEntity* bomb = nullptr;
  bool planted = false;

#if 0
  for (auto i = 1, max = engine->GetMaxClients(); i < max; i++) {
    if (i == id) {
      continue;
    }
    const auto player = static_cast<Player*>(entities->GetClientEntity(i));
    if (!player) {
      continue;
    }
    const auto client = player->GetClientClass();
    if (!client || client->m_ClassID != CCSPlayer) {
      continue;
    }
    PaintPlayer(i, static_cast<Player*>(player));
    if (!defusing && static_cast<Player*>(player)->IsDefusing()) {
      defusing = static_cast<Player*>(player)->HasDefuser() ? Color::Green() : Color::Orange();
    }
  }
#else
  for (int i = 1, max = entities->GetHighestEntityIndex(); i < max; i++) {
    if (i == id) {
      continue;
    }
    const auto entity = entities->GetClientEntity(i);
    if (!entity) {
      continue;
    }
    const auto client = entity->GetClientClass();
    if (!client) {
      continue;
    }
    switch (client->m_ClassID) {
    case CCSPlayer:
      PaintPlayer(i, static_cast<Player*>(entity));
      if (!defusing && static_cast<Player*>(entity)->IsDefusing()) {
        defusing = static_cast<Player*>(entity)->HasDefuser() ? Color::Green() : Color::Orange();
      }
      break;
    case CMolotovProjectile:
    case CSmokeGrenadeProjectile:
    case CBaseCSGrenadeProjectile:
      PaintNade(client, static_cast<ClientEntity*>(entity));
      break;
    case CC4:
    case CPlantedC4:
      planted = client->m_ClassID == CPlantedC4;
      bomb = static_cast<ClientEntity*>(entity);
      break;
    }
  }
#endif

  if (bomb) {
    PaintBomb(bomb, planted, defusing);
  }

  PaintStatus();
  PaintCursor();
}

#if 0
void Code::PaintPlayers(int id, Player* player) {
  const auto team = player->GetTeam();
  const auto time = engine->GetLastTimeStamp();
  player_info_t player_info = {};
  surface->DrawSetTextFont(esp_font_);
  int defusing = 0;
  for (auto i = 1, max = engine->GetMaxClients(); i < max; i++) {
    if (i == id) {
      continue;
    }
    const auto entity = static_cast<Player*>(entities->GetClientEntity(i));
    if (!entity || !entity->IsAlive() || entity->IsDormant()) {
      continue;
    }
    const auto client_class = entity->GetClientClass();
    if (!client_class || client_class->m_ClassID != ClassId::CCSPlayer) {
      continue;
    }
    matrix3x4_t bones[128];
    if (!entity->SetupBones(bones, 128, 0x00000100, time)) {
      continue;
    }
    const auto head = Vector(bones[Bone::Head][0][3], bones[Bone::Head][1][3], bones[Bone::Head][2][3]);
    const auto feet = Vector(bones[Bone::Feet][0][3], bones[Bone::Feet][1][3], bones[Bone::Feet][2][3]);
    Vector screen_head;
    Vector screen_feet;
    if (!Math::WorldToScreen(head, screen_head) || !Math::WorldToScreen(feet, screen_feet)) {
      continue;
    }
    const auto color = entity->GetTeam() != team ? color::hostile : color::friendly;
    const auto is_crouched = (entity->GetFlags() & static_cast<int>(EntityFlags::FL_DUCKING)) != 0;
    surface->DrawSetColor(color);
    const auto cy = static_cast<int>(std::abs(screen_head.y - screen_feet.y));
    const auto cx = static_cast<int>(cy * (is_crouched ? 0.65f : 0.50f));
    const auto sx = static_cast<int>(screen_head.x - cx / 2.0f);
    const auto sy = static_cast<int>(screen_head.y);
    surface->DrawOutlinedRect(sx, sy + cy / 8, sx + cx, sy + cy);
    const auto head_x = static_cast<int>(screen_head.x);
    const auto head_y = static_cast<int>(screen_head.y);
    surface->DrawOutlinedCircle(head_x, head_y, cy / 8, 24);

    if (!defusing && entity->IsDefusing()) {
      defusing = entity->HasDefuser() ? 1 : 2;
    }

    if (engine->GetPlayerInfo(i, &player_info)) {
      static std::wstring wname;
      const auto beg = player_info.szName;
      const auto end = std::find(beg, beg + sizeof(player_info.szName), '\0');
      const auto size = static_cast<int>(std::distance(beg, end));
      wname.resize(MultiByteToWideChar(CP_UTF8, 0, beg, size, nullptr, 0) + 1);
      wname.resize(MultiByteToWideChar(CP_UTF8, 0, beg, size, wname.data(), static_cast<int>(wname.size())));
      int text_cx = 0;
      int text_cy = 0;
      surface->GetTextSize(esp_font_, wname.data(), text_cx, text_cy);
      surface->DrawSetTextColor(color);
      surface->DrawSetTextPos(head_x - text_cx / 2, head_y - cy / 8 - text_cy - 2);
      surface->DrawText(wname.data());
    }
  }
  defusing_ = defusing;
}

void Code::PaintObjects(int id, Player* player) {
  const auto team = player->GetTeam();
  const auto time = engine->GetLastTimeStamp();
  surface->DrawSetTextFont(esp_font_);
  for (int i = 1, max = entities->GetHighestEntityIndex(); i < max; i++) {
    auto entity = entities->GetClientEntity(i);
    if (!entity) {
      continue;
    }
    auto client = entity->GetClientClass();
    if (!client) {
      continue;
    }
    if (client->m_ClassID == CC4 || client->m_ClassID == CPlantedC4) {
      auto bomb = static_cast<PlantedC4*>(entity);
      if (!bomb || !game || bomb->IsBombDefused() ||(client->m_ClassID == CC4 && !game->IsBombDropped())) {
        continue;
      }
      Vector screen;
      if (!Math::WorldToScreen(entity->GetAbsOrigin(), screen)) {
        continue;
      }
      const auto x = static_cast<int>(screen.x);
      const auto y = static_cast<int>(screen.y);
      const auto timer = bomb->GetBombTime() - globals->curtime;
      static Color color;
      static std::wstring text;
      if (game->IsBombPlanted() && !bomb->IsBombDefused() && timer > 0.0f) {
        static std::wostringstream oss;
        oss.str(L"");
        oss.clear();
        oss << std::fixed << std::showpoint << std::setprecision(1) << timer << L"s";
        color = color::hostile;
        text = oss.str();
        if (defusing_) {
          constexpr auto text = L"DEFUSING";
          int cx = 0;
          int cy = 0;
          surface->GetTextSize(esp_font_, text, cx, cy);
          surface->DrawSetTextColor(defusing_ == 1 ? Color::Green() : Color::Orange());
          surface->DrawSetTextPos(x - cx / 2, y + cy / 3 + cy);
          surface->DrawText(text);
        }
      } else {
        color = Color::White();
        text = L"BOMB";
      }
      int cx = 0;
      int cy = 0;
      surface->GetTextSize(esp_font_, text.data(), cx, cy);
      surface->DrawSetTextColor(color);
      surface->DrawSetTextPos(x - cx / 2, y - cy / 3);
      surface->DrawText(text.data());
      continue;
    }
  }
}
#endif

void Code::SetFlashMaxAlpha() {
  const auto id = engine->GetLocalPlayer();
  const auto player = static_cast<Player*>(entities->GetClientEntity(id));
  if (!player) {
    return;
  }
  *player->GetFlashMaxAlpha() = 128.0f;
}

void Code::EnablePostProcessing(bool enable) {
  if (!s_bOverridePostProcessingDisable) {
    warn("s_bOverridePostProcessingDisable not found");
    return;
  }
  *s_bOverridePostProcessingDisable = !enable;
}

void Code::FrameStageNotify(ClientFrameStage stage) {
  if (!engine->IsInGame()) {
    return;
  }
  switch (stage) {
  case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
    SetFlashMaxAlpha();
    break;
  case FRAME_RENDER_START:
    EnablePostProcessing(false);
    break;
  }
}

void Code::OnKey(int key, bool down) {
  if (down) {
    switch (key) {
    case VK_F9:
      settings::enable = !settings::enable;
      break;
    case 'O':
      qis_ = !qis_;
      break;
    }
  }
}

void Code::OnButton(int button, bool down) {
  if (down) {
    switch (button) {
    case input::buttons::up:
      settings::tgr = !settings::tgr;
      break;
    case input::buttons::down:
      settings::aim = !settings::aim;
      break;
    }
  }
}

}  // namespace csgo