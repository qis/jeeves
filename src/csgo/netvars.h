#pragma once
#include <ostream>
#include <cstddef>

namespace csgo {
namespace netvars {

extern std::ptrdiff_t m_nModelIndex;
extern std::ptrdiff_t m_flAnimTime;
extern std::ptrdiff_t m_flSimulationTime;
extern std::ptrdiff_t m_iTeamNum;
extern std::ptrdiff_t m_vecOrigin;
extern std::ptrdiff_t m_MoveType;
extern std::ptrdiff_t m_Collision;
extern std::ptrdiff_t m_bSpotted;
extern std::ptrdiff_t m_bIsHeldByPlayer;
extern std::ptrdiff_t m_bPinPulled;
extern std::ptrdiff_t m_fThrowTime;
extern std::ptrdiff_t m_flThrowStrength;
extern std::ptrdiff_t m_flNextPrimaryAttack;
extern std::ptrdiff_t m_viewPunchAngle;
extern std::ptrdiff_t m_aimPunchAngle;
extern std::ptrdiff_t m_vecViewOffset;
extern std::ptrdiff_t m_nTickBase;
extern std::ptrdiff_t m_vecVelocity;
extern std::ptrdiff_t m_iHealth;
extern std::ptrdiff_t m_lifeState;
extern std::ptrdiff_t m_fFlags;
extern std::ptrdiff_t m_hObserverTarget;
extern std::ptrdiff_t m_hViewModel;
extern std::ptrdiff_t m_szLastPlaceName;
extern std::ptrdiff_t m_iShotsFired;
extern std::ptrdiff_t m_angEyeAngles;
extern std::ptrdiff_t m_iAccount;
extern std::ptrdiff_t m_totalHitsOnServer;
extern std::ptrdiff_t m_ArmorValue;
extern std::ptrdiff_t m_bHasDefuser;
extern std::ptrdiff_t m_bIsDefusing;
extern std::ptrdiff_t m_bIsGrabbingHostage;
extern std::ptrdiff_t m_bIsScoped;
extern std::ptrdiff_t m_bGunGameImmunity;
extern std::ptrdiff_t m_bIsRescuing;
extern std::ptrdiff_t m_bHasHelmet;
extern std::ptrdiff_t m_flFlashDuration;
extern std::ptrdiff_t m_flFlashMaxAlpha;
extern std::ptrdiff_t m_flLowerBodyYawTarget;
extern std::ptrdiff_t m_hActiveWeapon;
extern std::ptrdiff_t m_hMyWeapons;
extern std::ptrdiff_t m_hMyWearables;
extern std::ptrdiff_t m_iItemDefinitionIndex;
extern std::ptrdiff_t m_bBombDropped;
extern std::ptrdiff_t m_bBombPlanted;
extern std::ptrdiff_t m_bBombTicking;
extern std::ptrdiff_t m_flC4Blow;
extern std::ptrdiff_t m_bBombDefused;
extern std::ptrdiff_t m_hBombDefuser;

bool initialize();
void dump(std::ostream& os);

}  // namespace netvars
}  // namespace csgo
