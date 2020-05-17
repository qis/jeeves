# Notes
Normal start options.

```
-console -novid -nojoy -fullscreen -nod3d9ex1 +cl_forcepreload 1
-console -novid -nojoy -window -w 2560 -h 1080 -noborder +cl_forcepreload 1
```

Automatically start Dust 2 in competitive mode.

```
-console -novid -nojoy -window -w 2560 -h 1080 -noborder -nod3d9ex1 +cl_forcepreload 1 -insecure +sv_cheats 1 +sv_lan 1 +game_type 0 +game_mode 1 +map de_dust2
```

Automatically start Dust 2 in deathmatch mode.

```
-console -novid -nojoy -window -w 2560 -h 1080 -noborder -nod3d9ex1 +cl_forcepreload 1 -insecure +sv_cheats 1 +sv_lan 1 +game_type 1 +game_mode 2 +map de_dust2
-console -novid -nojoy -window -w 2560 -h 1080 -noborder -nod3d9ex1 +cl_forcepreload 1 -insecure +sv_cheats 1 +sv_lan 1 +game_type 1 +game_mode 2 +map aim_ag_texture2
```

Useful game commands bound to the alias "start".

```
mp_warmuptime 0;
mp_roundtime 60;
mp_roundtime_defuse 60;
mp_buytime 3600;
mp_buy_anywhere 1;
mp_maxmoney 60000;
mp_startmoney 60000;
mp_afterroundmoney 60000;
mp_restartgame 1;
```

Useful game commands.

```
bot_stop 1
bot_difficulty 3
bot_knives_only 1
bot_quota 0
bot_kick
bot_add t
bot_add ct
record <demoname>
stop
demoui
```

Get client IDs.

```
sv_dump_...
```

## Libraries
* https://github.com/stephenfewer/ReflectiveDLLInjection
* https://github.com/MarkHC/CSGO-SDK-Example
* https://github.com/MarkHC/CSGOSimple

```cpp
void ESP::DrawProjectile(C_BaseEntity *projectile) {
	if (!*mESPFilterProjectiles) {
		return;
	}

	model_t *grenadeModel = projectile->GetModel();

	if (grenadeModel == nullptr) {
		return;
	}

	studiohdr_t *hdr = mInterfaceManager->modelInfo->GetStudioModel(grenadeModel);

	if (hdr == nullptr) {
		return;
	}

	if (strstr(hdr->szName, "thrown") != nullptr && strstr(hdr->szName, "dropped") != nullptr) {
		return;
	}

	Color grenadeColor = Color(255, 255, 255, 255);
	std::string grenadeName = "Unknown";

	IMaterial *mats[32];
	mInterfaceManager->modelInfo->GetModelMaterials(grenadeModel, hdr->numtextures, mats);

	for (int i = 0; i < hdr->numtextures; i++) {
		IMaterial *mat = mats[i];

		if (mat == nullptr) {
			continue;
		}

		if (strstr(mat->GetName(), "flashbang") != nullptr) {
			grenadeName = "Flashbang";
			grenadeColor = *mESPColorFlashbang;
			break;
		}

		if (strstr(mat->GetName(), "m67_grenade") != nullptr || strstr(mat->GetName(), "hegrenade") != nullptr) {
			grenadeName = "HE Grenade";
			grenadeColor = *mESPColorGrenade;
			break;
		}

		if (strstr(mat->GetName(), "smoke") != nullptr) {
			grenadeName = "Smoke";
			grenadeColor = *mESPColorSmoke;
			break;
		}

		if (strstr(mat->GetName(), "decoy") != nullptr) {
			grenadeName = "Decoy";
			grenadeColor = *mESPColorDecoy;
			break;
		}

		if (strstr(mat->GetName(), "incendiary") != nullptr || strstr(mat->GetName(), "molotov") != nullptr) {
			grenadeName = "Molotov";
			grenadeColor = *mESPColorMolotov;
			break;
		}
	}

	DrawEntity(projectile, grenadeColor, grenadeName.c_str());
}
```
