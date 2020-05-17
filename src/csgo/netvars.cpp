#include "netvars.h"
#include "sdk.h"
#include "xorstr.h"
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>

namespace csgo {
namespace netvars {
namespace {

struct table {
  std::ptrdiff_t offset = 0;
  std::unordered_map<std::string, std::unique_ptr<table>> children;
  std::unordered_map<std::string, std::ptrdiff_t> props;
};

std::unordered_map<std::string, std::unique_ptr<table>> database;

std::unique_ptr<table> load(RecvTable* recv, std::ptrdiff_t offset) {
  auto ret = std::make_unique<table>();
  ret->offset = offset;
  for (auto i = 0; i < recv->m_nProps; i++) {
    auto prop = &recv->m_pProps[i];

    // Skip trash array items.
    if (!prop || isdigit(prop->m_pVarName[0])) {
      continue;
    }

    // Skip base classes.
    if (strcmp(prop->m_pVarName, "baseclass") == 0) {
      continue;
    }

    // Insert children and props.
    if (prop->m_pDataTable) {
      ret->children.emplace(prop->m_pVarName, load(prop->m_pDataTable, prop->m_Offset));
    } else {
      ret->props.emplace(prop->m_pVarName, prop->m_Offset);
    }
  }
  return ret;
}

std::ptrdiff_t get(const std::string& name, const std::initializer_list<std::string>& props) {
  auto it = database.find(name);
  if (it == database.end()) {
    return 0;
  }
  auto table = it->second.get();
  auto offset = table->offset;
  for (std::size_t i = 0; i < props.size(); i++) {
    const auto& name = *(props.begin() + i);
    auto child = table->children.find(name);
    if (i + 1 < props.size()) {
      if (child == table->children.end()) {
        return 0;
      }
      offset += child->second->offset;
      table = child->second.get();
    } else {
      if (child != table->children.end()) {
        return child->second->offset;
      }
      auto prop = table->props.find(name);
      if (prop == table->props.end()) {
        return 0;
      }
      offset += prop->second;
    }
  }
  return offset;
}

template <typename... Args>
std::ptrdiff_t get(const std::string& name, Args&&... args) {
  return get(name, { std::forward<Args>(args)... });
}

void dump(std::ostream& os, const std::string& path, const std::unordered_map<std::string, std::ptrdiff_t>& props) {
  for (const auto& e : props) {
    os << (path.empty() ? path : path + '.') << e.first << ": " << e.second << '\n';
  }
}

void dump(std::ostream& os, const std::string& path, const std::unordered_map<std::string, std::unique_ptr<table>>& tables) {
  for (const auto& e : tables) {
    if (e.second) {
      os << (path.empty() ? path : path + '.') << e.first << ": " << e.second->offset << '\n';
      dump(os, (path.empty() ? path : path + '.') + e.first, e.second->children);
      dump(os, (path.empty() ? path : path + '.') + e.first, e.second->props);
    }
  }
}

void create() {
  if (client) {
    for (auto it = client->GetAllClasses(); it; it = it->m_pNext) {
      if (it->m_pRecvTable) {
        database.emplace(it->m_pRecvTable->m_pNetTableName, load(it->m_pRecvTable, 0));
      }
    }
  }
}

void destroy() {
  database.clear();
}

}  // namespace

std::ptrdiff_t m_nModelIndex = 0;
std::ptrdiff_t m_flAnimTime = 0;
std::ptrdiff_t m_flSimulationTime = 0;
std::ptrdiff_t m_iTeamNum = 0;
std::ptrdiff_t m_vecOrigin = 0;
std::ptrdiff_t m_MoveType = 0;
std::ptrdiff_t m_Collision = 0;
std::ptrdiff_t m_bSpotted = 0;
std::ptrdiff_t m_bIsHeldByPlayer = 0;
std::ptrdiff_t m_bPinPulled = 0;
std::ptrdiff_t m_fThrowTime = 0;
std::ptrdiff_t m_flThrowStrength = 0;
std::ptrdiff_t m_flNextPrimaryAttack = 0;
std::ptrdiff_t m_viewPunchAngle = 0;
std::ptrdiff_t m_aimPunchAngle = 0;
std::ptrdiff_t m_vecViewOffset = 0;
std::ptrdiff_t m_nTickBase = 0;
std::ptrdiff_t m_vecVelocity = 0;
std::ptrdiff_t m_iHealth = 0;
std::ptrdiff_t m_lifeState = 0;
std::ptrdiff_t m_fFlags = 0;
std::ptrdiff_t m_hObserverTarget = 0;
std::ptrdiff_t m_hViewModel = 0;
std::ptrdiff_t m_szLastPlaceName = 0;
std::ptrdiff_t m_iShotsFired = 0;
std::ptrdiff_t m_angEyeAngles = 0;
std::ptrdiff_t m_iAccount = 0;
std::ptrdiff_t m_totalHitsOnServer = 0;
std::ptrdiff_t m_ArmorValue = 0;
std::ptrdiff_t m_bHasDefuser = 0;
std::ptrdiff_t m_bIsDefusing = 0;
std::ptrdiff_t m_bIsGrabbingHostage = 0;
std::ptrdiff_t m_bIsScoped = 0;
std::ptrdiff_t m_bGunGameImmunity = 0;
std::ptrdiff_t m_bIsRescuing = 0;
std::ptrdiff_t m_bHasHelmet = 0;
std::ptrdiff_t m_flFlashDuration = 0;
std::ptrdiff_t m_flFlashMaxAlpha = 0;
std::ptrdiff_t m_flLowerBodyYawTarget = 0;
std::ptrdiff_t m_hActiveWeapon = 0;
std::ptrdiff_t m_hMyWeapons = 0;
std::ptrdiff_t m_hMyWearables = 0;
std::ptrdiff_t m_iItemDefinitionIndex = 0;
std::ptrdiff_t m_bBombDropped = 0;
std::ptrdiff_t m_bBombPlanted = 0;
std::ptrdiff_t m_bBombTicking = 0;
std::ptrdiff_t m_flC4Blow = 0;
std::ptrdiff_t m_bBombDefused = 0;
std::ptrdiff_t m_hBombDefuser = 0;

#define check(variable, command)                  \
  if (!(variable = command)) {                    \
    if (cvar) {                                   \
      warn("could not get netvar: " # variable);  \
      destroy();                                  \
      return false;                               \
    }                                             \
  }

bool initialize() {
  create();
  check(m_nModelIndex, netvars::get(xorstr("DT_BaseViewModel"), xorstr("m_nModelIndex")));
  check(m_flAnimTime, netvars::get(xorstr("DT_BaseEntity"), xorstr("AnimTimeMustBeFirst"), xorstr("m_flAnimTime")));
  check(m_flSimulationTime, netvars::get(xorstr("DT_BaseEntity"), xorstr("m_flSimulationTime")));
  check(m_iTeamNum, netvars::get(xorstr("DT_BaseEntity"), xorstr("m_iTeamNum")));
  check(m_vecOrigin, netvars::get(xorstr("DT_BaseEntity"), xorstr("m_vecOrigin")));
  check(m_MoveType, netvars::get(xorstr("DT_BaseEntity"), xorstr("m_nRenderMode")) + 1);
  check(m_Collision, netvars::get(xorstr("DT_BaseEntity"), xorstr("m_Collision")));
  check(m_bSpotted, netvars::get(xorstr("DT_BaseEntity"), xorstr("m_bSpotted")));
  check(m_bIsHeldByPlayer, netvars::get(xorstr("DT_BaseCSGrenade"), xorstr("m_bIsHeldByPlayer")));
  check(m_bPinPulled, netvars::get(xorstr("DT_BaseCSGrenade"), xorstr("m_bPinPulled")));
  check(m_fThrowTime, netvars::get(xorstr("DT_BaseCSGrenade"), xorstr("m_fThrowTime")));
  check(m_flThrowStrength, netvars::get(xorstr("DT_BaseCSGrenade"), xorstr("m_flThrowStrength")));
  check(m_flNextPrimaryAttack, netvars::get(xorstr("DT_BaseCombatWeapon"), xorstr("LocalActiveWeaponData"), xorstr("m_flNextPrimaryAttack")));
  check(m_viewPunchAngle, netvars::get(xorstr("DT_BasePlayer"), xorstr("localdata"), xorstr("m_Local"), xorstr("m_viewPunchAngle")));
  check(m_aimPunchAngle, netvars::get(xorstr("DT_BasePlayer"), xorstr("localdata"), xorstr("m_Local"), xorstr("m_aimPunchAngle")));
  check(m_vecViewOffset, netvars::get(xorstr("DT_BasePlayer"), xorstr("localdata"), xorstr("m_vecViewOffset[0]")));
  check(m_nTickBase, netvars::get(xorstr("DT_BasePlayer"), xorstr("localdata"), xorstr("m_nTickBase")));
  check(m_vecVelocity, netvars::get(xorstr("DT_BasePlayer"), xorstr("localdata"), xorstr("m_vecVelocity[0]")));
  check(m_iHealth, netvars::get(xorstr("DT_BasePlayer"), xorstr("m_iHealth")));
  check(m_lifeState, netvars::get(xorstr("DT_BasePlayer"), xorstr("m_lifeState")));
  check(m_fFlags, netvars::get(xorstr("DT_BasePlayer"), xorstr("m_fFlags")));
  check(m_hObserverTarget, netvars::get(xorstr("DT_BasePlayer"), xorstr("m_hObserverTarget")));
  check(m_hViewModel, netvars::get(xorstr("DT_BasePlayer"), xorstr("m_hViewModel[0]")));
  check(m_szLastPlaceName, netvars::get(xorstr("DT_BasePlayer"), xorstr("m_szLastPlaceName")));
  check(m_iShotsFired, netvars::get(xorstr("DT_CSPlayer"), xorstr("cslocaldata"), xorstr("m_iShotsFired")));
  check(m_angEyeAngles, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_angEyeAngles")));
  check(m_iAccount, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_iAccount")));
  check(m_totalHitsOnServer, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_totalHitsOnServer")));
  check(m_ArmorValue, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_ArmorValue")));
  check(m_bHasDefuser, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_bHasDefuser")));
  check(m_bIsDefusing, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_bIsDefusing")));
  check(m_bIsGrabbingHostage, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_bIsGrabbingHostage")));
  check(m_bIsScoped, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_bIsScoped")));
  check(m_bGunGameImmunity, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_bGunGameImmunity")));
  check(m_bIsRescuing, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_bIsRescuing")));
  check(m_bHasHelmet, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_bHasHelmet")));
  check(m_flFlashDuration, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_flFlashDuration")));
  check(m_flFlashMaxAlpha, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_flFlashMaxAlpha")));
  check(m_flLowerBodyYawTarget, netvars::get(xorstr("DT_CSPlayer"), xorstr("m_flLowerBodyYawTarget")));
  check(m_hActiveWeapon, netvars::get(xorstr("DT_BaseCombatCharacter"), xorstr("m_hActiveWeapon")));
  check(m_hMyWeapons, netvars::get(xorstr("DT_BaseCombatCharacter"), xorstr("m_hMyWeapons")));
  check(m_hMyWearables, netvars::get(xorstr("DT_BaseCombatCharacter"), xorstr("m_hMyWearables")));
  check(m_iItemDefinitionIndex, netvars::get(xorstr("DT_BaseAttributableItem"), xorstr("m_AttributeManager"), xorstr("m_Item"), xorstr("m_iItemDefinitionIndex")));
  check(m_bBombDropped, netvars::get(xorstr("DT_CSGameRulesProxy"), xorstr("cs_gamerules_data"), xorstr("m_bBombDropped")));
  check(m_bBombPlanted, netvars::get(xorstr("DT_CSGameRulesProxy"), xorstr("cs_gamerules_data"), xorstr("m_bBombPlanted")));
  check(m_bBombTicking, netvars::get(xorstr("DT_PlantedC4"), xorstr("m_bBombTicking")));
  check(m_flC4Blow, netvars::get(xorstr("DT_PlantedC4"), xorstr("m_flC4Blow")));
  check(m_bBombDefused, netvars::get(xorstr("DT_PlantedC4"), xorstr("m_bBombDefused")));
  check(m_hBombDefuser, netvars::get(xorstr("DT_PlantedC4"), xorstr("m_hBombDefuser")));

  destroy();
  return true;
}

void dump(std::ostream& os) {
  create();
  dump(os, {}, database);
  destroy();
}

}  // namespace netvars
}  // namespace csgo
