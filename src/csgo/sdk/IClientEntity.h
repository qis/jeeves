#pragma once
#include "Common.h"
#include "IClientNetworkable.h"
#include "IClientRenderable.h"
#include "IClientUnknown.h"
#include "IClientThinkable.h"

namespace csgo {

struct SpatializationInfo_t;

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable {
public:
  virtual void             Release(void) = 0;
  virtual const Vector&    GetAbsOrigin(void) const = 0;  // qis: added reference
  virtual const QAngle     GetAbsAngles(void) const = 0;
  virtual void*            GetMouth(void) = 0;
  virtual bool             GetSoundSpatialization(SpatializationInfo_t info) = 0;
  virtual bool             IsBlurred(void) = 0;
};

}  // namespace csgo