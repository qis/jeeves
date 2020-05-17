// CS:GO SDK based on https://github.com/alliedmodders/hl2sdk/tree/csgo

#pragma once
#include <windows.h>
#ifdef DrawText
#undef DrawText
#endif
#include <string>
#include <string_view>
#include <cstddef>
#include <cstdio>
#include "sdk/Common.h"
#include "sdk/CRC.h"
#include "sdk/Vector.h"
#include "sdk/Vector2D.h"
#include "sdk/Vector4D.h"
#include "sdk/QAngle.h"
#include "sdk/CHandle.h"
#include "sdk/CGlobalVarsBase.h"
#include "sdk/ClientClass.h"
#include "sdk/Color.h"
#include "sdk/IBaseClientDll.h"
#include "sdk/IClientEntity.h"
#include "sdk/IClientEntityList.h"
#include "sdk/IClientNetworkable.h"
#include "sdk/IClientRenderable.h"
#include "sdk/IClientThinkable.h"
#include "sdk/IClientUnknown.h"
#include "sdk/IEngineVGui.h"
#include "sdk/IPanel.h"
#include "sdk/ISurface.h"
#include "sdk/IVEngineClient.h"
#include "sdk/IEngineTrace.h"
#include "sdk/PlayerInfo.h"
#include "sdk/Recv.h"
#include "sdk/VMatrix.h"
#include "sdk/IClientMode.h"
#include "sdk/CInput.h"
#include "sdk/ICvar.h"
#include "xorstr.h"

#define WM_CSGO_CLOSE (WM_APP + 0xB000)
#define WM_CSGO_ERROR (WM_APP + 0xB001)

namespace csgo {

class GameRules {
public:
  template <class T>
  T* GetFieldPointer(std::ptrdiff_t offset) {
    return reinterpret_cast<T*>(reinterpret_cast<DWORD>(this) + offset);
  }

  template <class T>
  inline T GetFieldValue(std::ptrdiff_t offset) {
    return *GetFieldPointer<T>(offset);
  }

  bool IsBombDropped();
  bool IsBombPlanted();
};

class ClientEntity : public IClientEntity {
public:
  template <class T>
  T* GetFieldPointer(std::ptrdiff_t offset) {
    return reinterpret_cast<T*>(reinterpret_cast<DWORD>(this) + offset);
  }

  template <class T>
  inline T GetFieldValue(std::ptrdiff_t offset) {
    return *GetFieldPointer<T>(offset);
  }

  int*                GetModelIndex();
  float               GetAnimTime();
  float               GetSimulationTime();
  int                 GetTeam();
  Vector              GetVecOrigin();
  MoveType            GetMoveType();
  ICollideable*       GetCollideable();
  bool*               GetSpotted();
};

class Weapon : public ClientEntity {
public:
  WeaponId            GetId();
  WeaponType          GetType();
  const char*         GetName();
  bool                IsHeldByPlayer();
  bool                GetPinPulled();
  float               GetThrowTime();
  float               GetThrowStrength();
  float               GetNextPrimaryAttack();
};

class PlantedC4 : public ClientEntity {
public:
  template <class T>
  T* GetFieldPointer(std::ptrdiff_t offset) {
    return reinterpret_cast<T*>(reinterpret_cast<DWORD>(this) + offset);
  }

  template <class T>
  inline T GetFieldValue(std::ptrdiff_t offset) {
    return *GetFieldPointer<T>(offset);
  }

  bool IsBombTicking();
  float GetBombTime();
  bool IsBombDefused();
  int GetBombDefuser();
};

class Player : public ClientEntity {
public:
  QAngle*             GetViewPunchAngle();
  QAngle*             GetAimPunchAngle();
  Vector              GetVecViewOffset();
  unsigned int        GetTickBase();
  Vector              GetVelocity();
  int                 GetHealth();
  unsigned char       GetLifeState();
  int                 GetFlags();
  void*               GetObserverTarget();
  void*               GetViewModel();
  const char*         GetLastPlaceName();
  int                 GetShotsFired();
  QAngle*             GetEyeAngles();
  int                 GetMoney();
  int                 GetHits();
  int                 GetArmor();
  int                 HasDefuser();
  bool                IsDefusing();
  bool                IsGrabbingHostage();
  bool                IsScoped();
  bool                GetImmune();
  bool                IsRescuing();
  int                 HasHelmet();
  float               GetFlashDuration();
  float*              GetFlashMaxAlpha();
  float*              GetLowerBodyYawTarget();
  Weapon*             GetActiveWeapon();
  int*                GetWeapons();
  int*                GetWearables();
  bool                IsAlive();
  Vector              GetEyePosition();
};

// ====================================================================================================================

extern HWND window;
extern ICvar* cvar;
extern IBaseClientDLL* client;
extern IVEngineClient* engine;
extern IEngineVGui* engine_vgui;
extern IClientMode* client_mode;
extern IPanel* vgui2_panel;
extern IClientEntityList* entities;
extern ISurface* surface;
extern IEngineTrace* trace;
extern CGlobalVarsBase* globals;

extern void(__thiscall*StartDrawing)(void*);
extern void(__thiscall*FinishDrawing)(void*);
extern bool(__cdecl*MsgFunc_ServerRankRevealAll)(float*);
extern bool* s_bOverridePostProcessingDisable;

GameRules* GetGame();
bool initialize();

#define info(s, ...) cvar->ConsoleColorPrintf(Color(120, 255, 90), xorstr("[jeeves] " ## s ## "\n"), __VA_ARGS__)
#define warn(s, ...) cvar->ConsoleColorPrintf(Color(255, 90, 90), xorstr("[jeeves] " ## s ## "\n"), __VA_ARGS__)

}  // namespace csgo