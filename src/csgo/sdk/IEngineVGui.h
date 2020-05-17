#pragma once
#include "Common.h"

namespace csgo {

enum VGuiPanel {
  PANEL_ROOT = 0,
  PANEL_GAMEUIDLL,
  PANEL_CLIENTDLL,
  PANEL_TOOLS,
  PANEL_INGAMESCREENS,
  PANEL_GAMEDLL,
  PANEL_CLIENTDLL_TOOLS
};

enum PaintMode_t {
  PAINT_UIPANELS     = (1 << 0),
  PAINT_INGAMEPANELS = (1 << 1),
  PAINT_CURSOR       = (1 << 2)
};

class IEngineVGui {
public:
  virtual ~IEngineVGui() {}
  virtual VPANEL GetPanel(VGuiPanel type) = 0;
  virtual bool IsGameUIVisible() = 0;
  virtual void ActivateGameUI() = 0;
};

}  // namespace csgo