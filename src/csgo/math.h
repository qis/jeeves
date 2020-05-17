#pragma once
#include "sdk.h"

#define M_PI		3.14159265358979323846
#define M_PI_F	3.14159265358979323846f
#define M_PHI		1.61803398874989484820

#ifndef RAD2DEG
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI_F) )
#endif

#ifndef DEG2RAD
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI_F / 180.f) )
#endif

namespace csgo {
namespace Math {

enum {
  PITCH = 0,  // up / down
  YAW   = 1,  // left / right
  ROLL  = 2,  // fall over
};

inline Vector GetEntityBone(IClientEntity* entity, Bone bone) {
  matrix3x4_t matrix[128];
  if (entity->SetupBones(matrix, 128, 0x00000100, engine->GetLastTimeStamp())) {
    return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
  }
  return {};
}

inline bool ScreenTransform(const Vector& point, Vector& screen) {
  const auto& matrix = engine->WorldToScreenMatrix();
  screen.x = matrix.m[0][0] * point.x + matrix.m[0][1] * point.y + matrix.m[0][2] * point.z + matrix.m[0][3];
  screen.y = matrix.m[1][0] * point.x + matrix.m[1][1] * point.y + matrix.m[1][2] * point.z + matrix.m[1][3];
  screen.z = 0.0f;
  const auto w = matrix.m[3][0] * point.x + matrix.m[3][1] * point.y + matrix.m[3][2] * point.z + matrix.m[3][3];
  if (w < 0.001f) {
    screen.x *= 100000;
    screen.y *= 100000;
    return true;
  }
  float invw = 1.0f / w;
  screen.x *= invw;
  screen.y *= invw;
  return false;
}

inline bool WorldToScreen(const Vector& origin, Vector& screen) {
  if (!ScreenTransform(origin, screen)) {
    auto cx = 0;
    auto cy = 0;
    engine->GetScreenSize(cx, cy);
    screen.x = (cx / 2.0f) + (screen.x * cx) / 2;
    screen.y = (cy / 2.0f) - (screen.y * cy) / 2;
    return true;
  }
  return false;
}

inline void Normalize(Vector& angle) {
  while (angle.x > 89.0f) {
    angle.x -= 180.f;
  }
  while (angle.x < -89.0f) {
    angle.x += 180.f;
  }
  while (angle.y > 180.f) {
    angle.y -= 360.f;
  }
  while (angle.y < -180.f) {
    angle.y += 360.f;
  }
}

inline void ClampAngles(Vector& angles) {
  if (angles.y > 180.0f) {
    angles.y = 180.0f;
  } else if (angles.y < -180.0f) {
    angles.y = -180.0f;
  }
  if (angles.x > 89.0f) {
    angles.x = 89.0f;
  } else if (angles.x < -89.0f) {
    angles.x = -89.0f;
  }
  angles.z = 0;
}

inline bool Clamp(Vector& angles) {
  auto a = angles;
  Normalize(a);
  ClampAngles(a);
  if (isnan(a.x) || isinf(a.x) || isnan(a.y) || isinf(a.y) || isnan(a.z) || isinf(a.z)) {
    return false;
  }
  angles = a;
  return true;
}

inline void SinCos(float angle, float *s, float *c) {
  _asm {
    fld     angle
    fsincos
    mov     eax, c
    fstp    dword ptr[eax]
    mov     eax, s
    fstp    dword ptr[eax]
  }
}

inline void AngleVectors(const Vector &angles, Vector &forward) {
  float sp, sy, cp, cy;
  SinCos(DEG2RAD(angles[YAW]), &sy, &cy);
  SinCos(DEG2RAD(angles[PITCH]), &sp, &cp);
  forward.x = cp * cy;
  forward.y = cp * sy;
  forward.z = -sp;
}

inline void NormalizeAngles(Vector& angle) {
  while (angle.x > 89.0f) {
    angle.x -= 180.f;
  }
  while (angle.x < -89.0f) {
    angle.x += 180.f;
  }
  while (angle.y > 180.f) {
    angle.y -= 360.f;
  }
  while (angle.y < -180.f) {
    angle.y += 360.f;
  }
}

inline void CorrectMovement(Vector vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove) {
  // side/forward move correction
  float deltaView;
  float f1;
  float f2;
  if (vOldAngles.y < 0.f) {
    f1 = 360.0f + vOldAngles.y;
  } else {
    f1 = vOldAngles.y;
  }
  if (pCmd->viewangles.y < 0.0f) {
    f2 = 360.0f + pCmd->viewangles.y;
  } else {
    f2 = pCmd->viewangles.y;
  }
  if (f2 < f1) {
    deltaView = abs(f2 - f1);
  } else {
    deltaView = 360.0f - abs(f1 - f2);
  }
  deltaView = 360.0f - deltaView;
  pCmd->forwardmove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
  pCmd->sidemove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
}

inline float GetFov(const Vector &viewAngle, const Vector &aimAngle) {
  Vector delta = aimAngle - viewAngle;
  NormalizeAngles(delta);
  return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
}

inline void VectorAngles(const Vector &forward, Vector &angles) {
  if (forward[1] == 0.0f && forward[0] == 0.0f) {
    angles[0] = (forward[2] > 0.0f) ? 270.0f : 90.0f; // pitch (up/down)
    angles[1] = 0.0f;  // yaw (left/right)
  } else {
    angles[0] = atan2(-forward[2], forward.Length2D()) * -180 / M_PI_F;
    angles[1] = atan2(forward[1], forward[0]) * 180 / M_PI_F;
    if (angles[1] > 90) {
      angles[1] -= 180;
    } else if (angles[1] < 90) {
      angles[1] += 180;
    } else if (angles[1] == 90) {
      angles[1] = 0;
    }
  }
  angles[2] = 0.0f;
}

inline Vector CalcAngle(Vector src, Vector dst) {
  Vector angles;
  VectorAngles(src - dst, angles);
  return angles;
}

inline Vector ExtrapolatePosition(Player* player, const Vector& pos) {
  return pos + (player->GetVelocity() * globals->interval_per_tick);
}

}  // namespace Math
}  // namespace csgo