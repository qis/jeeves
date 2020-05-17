#include "sdk.h"
#include "netvars.h"
#include <psapi.h>
#include <string>
#include <vector>

namespace csgo {
namespace {

#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) (INRANGE((x&(~0x20)), 'A', 'F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x, '0', '9') ? x - '0' : 0))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))

DWORD FindSignature(HMODULE module, const char* signature) {
  MODULEINFO mi = {};
  GetModuleInformation(GetCurrentProcess(), module, &mi, sizeof(MODULEINFO));
  const auto begin = reinterpret_cast<DWORD>(mi.lpBaseOfDll);
  const auto end = begin + mi.SizeOfImage;
  auto pat = signature;
  auto ret = DWORD(0);
  for (auto cur = begin; cur < end; cur++) {
    if (!*pat) {
      return ret;
    }
    if (*(PBYTE)pat == '\?' || *(BYTE*)cur == getByte(pat)) {
      if (!ret) {
        ret = cur;
      }
      if (!pat[2]) {
        return ret;
      }
      if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') {
        pat += 3;
      } else {
        pat += 2;
      }
    } else {
      pat = signature;
      ret = 0;
    }
  }
  return 0;
}

#undef getByte
#undef getBits
#undef INRANGE

template <typename T>
T* GetInstanceFromFactory(const char* module_name, const char* instance_name) {
  if (const auto module = GetModuleHandleA(module_name)) {
    if (const auto factory = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module, xorstr("CreateInterface")))) {
      return reinterpret_cast<T*>(factory(instance_name, nullptr));
    }
  }
  return nullptr;
}

template <typename T>
T* GetValueFromSignature(const char* module_name, const char* signature, DWORD offset = 0) {
  if (const auto module = GetModuleHandleA(module_name)) {
    if (const auto location = FindSignature(module, signature)) {
      if (const auto address = reinterpret_cast<DWORD*>(location + offset)) {
        return reinterpret_cast<T*>(*address);
      }
    }
  }
  return nullptr;
}

template <typename T>
T* GetInstanceFromSignature(const char* module_name, const char* signature, DWORD offset = 0) {
  if (const auto address = GetValueFromSignature<T*>(module_name, signature, offset)) {
    return *address;
  }
  return nullptr;
}

template <typename T>
T GetFunctionFromSignature(const char* module_name, const char* signature, DWORD offset = 0) {
  if (const auto module = GetModuleHandleA(module_name)) {
    if (const auto location = FindSignature(module, signature)) {
      return reinterpret_cast<T>(location + offset);
    }
  }
  return nullptr;
}

DWORD FindPattern(char* pattern, char* mask, DWORD start, DWORD end, DWORD offset) {
  int patternLength = strlen(mask);
  auto found = false;
  // For each byte from start to end.
  for (DWORD i = start; i < end - patternLength; i++) {
    found = true;
    // For each byte in the pattern.
    for (int idx = 0; idx < patternLength; idx++) {
      if (mask[idx] == 'x' && pattern[idx] != *(char*)(i + idx)) {
        found = false;
        break;
      }
    }
    if (found) {
      return i + offset;
    }
  }
  return NULL;
}

}  // namespace

// ====================================================================================================================

bool GameRules::IsBombDropped() {
  return GetFieldValue<bool>(netvars::m_bBombDropped);
}

bool GameRules::IsBombPlanted() {
  return GetFieldValue<bool>(netvars::m_bBombPlanted);
}

// ====================================================================================================================

int* ClientEntity::GetModelIndex() {
  return GetFieldPointer<int>(netvars::m_nModelIndex);
}

float ClientEntity::GetAnimTime() {
  return GetFieldValue<float>(netvars::m_flAnimTime);
}

float ClientEntity::GetSimulationTime() {
  return GetFieldValue<float>(netvars::m_flSimulationTime);
}

int ClientEntity::GetTeam() {
  return GetFieldValue<int>(netvars::m_iTeamNum);
}

Vector ClientEntity::GetVecOrigin() {
  return GetFieldValue<Vector>(netvars::m_vecOrigin);
}

MoveType ClientEntity::GetMoveType() {
  return GetFieldValue<MoveType>(netvars::m_MoveType);
}

ICollideable* ClientEntity::GetCollideable() {
  return GetFieldPointer<ICollideable>(netvars::m_Collision);
}

bool* ClientEntity::GetSpotted() {
  return GetFieldPointer<bool>(netvars::m_bSpotted);
}

// ====================================================================================================================

WeaponId Weapon::GetId() {
  //return CallVFunction<WeaponId(__thiscall*)(void*)>(this, 458)(this);
  return *GetFieldPointer<WeaponId>(netvars::m_iItemDefinitionIndex);
}

WeaponType Weapon::GetType() {
  switch (GetId()) {
  case weapon_deagle:
  case weapon_elite:
  case weapon_fiveseven:
  case weapon_glock:
  case weapon_tec9:
  case weapon_hkp2000:
  case weapon_p250:
  case weapon_usp_silencer:
  case weapon_cz75a:
  case weapon_revolver:
    return weapon_type_pistol;
  case weapon_ak47:
  case weapon_famas:
  case weapon_galilar:
  case weapon_m4a1:
  case weapon_m4a1_silencer:
    return weapon_type_assault_rifle;
  case weapon_aug:
  case weapon_sg556:
    return weapon_type_zoom_rifle;
  case weapon_awp:
  case weapon_ssg08:
    return weapon_type_sniper;
  case weapon_g3sg1:
  case weapon_scar20:
    return weapon_type_auto_sniper;
  case weapon_m249:
  case weapon_negev:
    return weapon_type_machine_gun;
  case weapon_mac10:
  case weapon_p90:
  case weapon_ump:
  case weapon_bizon:
  case weapon_mp7:
  case weapon_mp9:
    return weapon_type_machine_pistol;
  case weapon_xm1014:
  case weapon_mag7:
  case weapon_sawedoff:
  case weapon_nova:
    return weapon_type_shotgun;
  case weapon_flashbang:
  case weapon_hegrenade:
  case weapon_smokegrenade:
  case weapon_molotov:
  case weapon_decoy:
  case weapon_incgrenade:
    return weapon_type_grenade;
  case weapon_taser:
    return weapon_type_taser;
  case weapon_knife:
  case weapon_knife_t:
  case weapon_bayonet:
  case weapon_knife_flip:
  case weapon_knife_gut:
  case weapon_knife_karambit:
  case weapon_knife_m9_bayonet:
  case weapon_knife_tactical:
  case weapon_knife_falchion:
  case weapon_knife_survival_bowie:
  case weapon_knife_butterfly:
  case weapon_knife_push:
    return weapon_type_knife;
  case weapon_c4:
    return weapon_type_bomb;
  }
  return weapon_type_unknown;
}

const char* Weapon::GetName() {
  return CallVFunction<const char* (__thiscall*)(void*)>(this, 378)(this);
}

bool Weapon::IsHeldByPlayer() {
  return GetFieldValue<bool>(netvars::m_bIsHeldByPlayer);
}

bool Weapon::GetPinPulled() {
  return GetFieldValue<bool>(netvars::m_bPinPulled);
}

float Weapon::GetThrowTime() {
  return GetFieldValue<float>(netvars::m_fThrowTime);
}

float Weapon::GetThrowStrength() {
  return GetFieldValue<float>(netvars::m_flThrowStrength);
}

float Weapon::GetNextPrimaryAttack() {
  return GetFieldValue<float>(netvars::m_flNextPrimaryAttack);
}

// ====================================================================================================================

bool PlantedC4::IsBombTicking() {
  return GetFieldValue<bool>(netvars::m_bBombTicking);
}

float PlantedC4::GetBombTime() {
  return GetFieldValue<float>(netvars::m_flC4Blow);
}

bool PlantedC4::IsBombDefused() {
  return GetFieldValue<bool>(netvars::m_bBombDefused);
}

int PlantedC4::GetBombDefuser() {
  return GetFieldValue<int>(netvars::m_hBombDefuser);
}

// ====================================================================================================================

QAngle* Player::GetViewPunchAngle() {
  return GetFieldPointer<QAngle>(netvars::m_viewPunchAngle);
}

QAngle* Player::GetAimPunchAngle() {
  return GetFieldPointer<QAngle>(netvars::m_aimPunchAngle);
}

Vector Player::GetVecViewOffset() {
  return GetFieldValue<Vector>(netvars::m_vecViewOffset);
}

unsigned int Player::GetTickBase() {
  return GetFieldValue<unsigned int>(netvars::m_nTickBase);
}

Vector Player::GetVelocity() {
  return GetFieldValue<Vector>(netvars::m_vecVelocity);
}

int Player::GetHealth() {
  return GetFieldValue<int>(netvars::m_iHealth);
}

unsigned char Player::GetLifeState() {
  return GetFieldValue<unsigned char>(netvars::m_lifeState);
}

int Player::GetFlags() {
  return GetFieldValue<int>(netvars::m_fFlags);
}

void* Player::GetObserverTarget() {
  return GetFieldPointer<void>(netvars::m_hObserverTarget);
}

void* Player::GetViewModel() {
  return GetFieldPointer<void>(netvars::m_hViewModel);
}

const char* Player::GetLastPlaceName() {
  return GetFieldPointer<const char>(netvars::m_szLastPlaceName);
}

int Player::GetShotsFired() {
  return GetFieldValue<int>(netvars::m_iShotsFired);
}

QAngle* Player::GetEyeAngles() {
  return GetFieldPointer<QAngle>(netvars::m_angEyeAngles);
}

int Player::GetMoney() {
  return GetFieldValue<int>(netvars::m_iAccount);
}

int Player::GetHits() {
  return GetFieldValue<int>(netvars::m_totalHitsOnServer);
}

int Player::GetArmor() {
  return GetFieldValue<int>(netvars::m_ArmorValue);
}

int Player::HasDefuser() {
  return GetFieldValue<int>(netvars::m_bHasDefuser);
}

bool Player::IsDefusing() {
  return GetFieldValue<bool>(netvars::m_bIsDefusing);
}

bool Player::IsGrabbingHostage() {
  return GetFieldValue<bool>(netvars::m_bIsGrabbingHostage);
}

bool Player::IsScoped() {
  return GetFieldValue<bool>(netvars::m_bIsScoped);
}

bool Player::GetImmune() {
  return GetFieldValue<bool>(netvars::m_bGunGameImmunity);
}

bool Player::IsRescuing() {
  return GetFieldValue<bool>(netvars::m_bIsRescuing);
}

int Player::HasHelmet() {
  return GetFieldValue<int>(netvars::m_bHasHelmet);
}

float Player::GetFlashDuration() {
  return GetFieldValue<float>(netvars::m_flFlashDuration);
}

float* Player::GetFlashMaxAlpha() {
  return GetFieldPointer<float>(netvars::m_flFlashMaxAlpha);
}

float* Player::GetLowerBodyYawTarget() {
  return GetFieldPointer<float>(netvars::m_flLowerBodyYawTarget);
}

Weapon* Player::GetActiveWeapon() {
  const auto handle = GetFieldValue<CHandle<IClientEntity>>(netvars::m_hActiveWeapon);
  return static_cast<Weapon*>(entities->GetClientEntityFromHandle(handle));
}

int* Player::GetWeapons() {
  return GetFieldPointer<int>(netvars::m_hMyWeapons);
}

int* Player::GetWearables() {
  return GetFieldPointer<int>(netvars::m_hMyWearables);
}

bool Player::IsAlive() {
  return this->GetHealth() > 0 && this->GetLifeState() == LIFE_ALIVE;
}

Vector Player::GetEyePosition() {
  return this->GetVecOrigin() + this->GetVecViewOffset();
}

// ====================================================================================================================

HWND window = nullptr;
ICvar* cvar = nullptr;
IBaseClientDLL* client = nullptr;
IVEngineClient* engine = nullptr;
IEngineVGui* engine_vgui = nullptr;
IClientMode* client_mode = nullptr;
IPanel* vgui2_panel = nullptr;
IClientEntityList* entities = nullptr;
ISurface* surface = nullptr;
IEngineTrace* trace = nullptr;
CGlobalVarsBase* globals = nullptr;

void(__thiscall*StartDrawing)(void*) = nullptr;
void(__thiscall*FinishDrawing)(void*) = nullptr;
bool(__cdecl*MsgFunc_ServerRankRevealAll)(float*) = nullptr;
bool* s_bOverridePostProcessingDisable = nullptr;

#define check(variable, command)                    \
  if (!(variable = command)) {                      \
    if (cvar) {                                     \
      warn("could not get instance: " # variable);  \
      return false;                                 \
    }                                               \
  }

GameRules* GetGame() {
  static GameRules* game = nullptr;
  if (!game) {
    game = GetInstanceFromSignature<GameRules>(xorstr("client.dll"), xorstr("A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 B8 ? ? ? ? ? 0F 84 ? ? ? ? 0F 10 05"), 1);
  }
  return game;
}

bool initialize() {
  check(cvar, GetInstanceFromFactory<ICvar>(xorstr("vstdlib.dll"), xorstr("VEngineCvar007")));
  for (auto i = 0; i < 30 && !window; i++) {
    window = FindWindowA(xorstr("Valve001"), nullptr);
    if (!window) {
      Sleep(100);
    }
  }
  if (!window) {
    return false;
  }
  check(client, GetInstanceFromFactory<IBaseClientDLL>(xorstr("client.dll"), xorstr("VClient018")));
  check(engine, GetInstanceFromFactory<IVEngineClient>(xorstr("engine.dll"), xorstr("VEngineClient014")));
  check(engine_vgui, GetInstanceFromFactory<IEngineVGui>(xorstr("engine.dll"), xorstr("VEngineVGui001")));
  check(client_mode, GetInstanceFromSignature<IClientMode>(xorstr("client.dll"), xorstr("8B 0D ? ? ? ? 8B 01 5D FF"), 2));
  check(vgui2_panel, GetInstanceFromFactory<IPanel>(xorstr("vgui2.dll"), xorstr("VGUI_Panel009")));
  check(entities, GetInstanceFromFactory<IClientEntityList>(xorstr("client.dll"), xorstr("VClientEntityList003")));
  check(surface, GetInstanceFromFactory<ISurface>(xorstr("vguimatsurface.dll"), xorstr("VGUI_Surface031")));
  check(trace, GetInstanceFromFactory<IEngineTrace>(xorstr("engine.dll"), xorstr("EngineTraceClient004")));
  auto client_base = reinterpret_cast<PDWORD>(*reinterpret_cast<PDWORD>(client));
  auto client_init = static_cast<DWORD>(client_base[0]);
  for (DWORD i = 0; i <= 0xFF; i++) {
    if (*reinterpret_cast<PBYTE>(client_init + i) == 0xA3) {
      globals = reinterpret_cast<CGlobalVarsBase*>(*reinterpret_cast<PDWORD>(*reinterpret_cast<PDWORD>(client_init + i + 1)));
      break;
    }
  }
  if (!globals) {
    warn("could not get instance: globals");
    return false;
  }
  check(StartDrawing, GetFunctionFromSignature<decltype(StartDrawing)>(xorstr("vguimatsurface.dll"), xorstr("55 8B EC 83 E4 C0 83 EC 38")));
  check(FinishDrawing, GetFunctionFromSignature<decltype(FinishDrawing)>(xorstr("vguimatsurface.dll"), xorstr("8B 0D ? ? ? ? 56 C6 05")));
  check(MsgFunc_ServerRankRevealAll, GetFunctionFromSignature<decltype(MsgFunc_ServerRankRevealAll)>(xorstr("client.dll"), xorstr("55 8B EC 8B 0D ? ? ? ? 68")));
  check(s_bOverridePostProcessingDisable, GetValueFromSignature<bool>(xorstr("client.dll"), xorstr("80 3D ? ? ? ? ? 53 56 57 0F 85"), 2));

  //netvars::initialize();
  //std::ofstream os(xorstr("C:\\Workspace\\jeeves\\res\\netvars.txt"), std::ios::binary);
  //netvars::dump(os);
  //return false;

  return netvars::initialize();
}

}  // namespace csgo
