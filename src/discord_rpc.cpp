#include <ctime>
#include <stdio.h>
#include <windows.h>
#include <discord_rpc.h>

#include <iostream>
#include <fstream> //* - SC: gonna have to use these later for debugging
//SC: NFSMW2005 doesn't have proper debugging tools... why? why has EA forsaken us?
#include <config.h>
#include <utils.h>

using namespace std;
using namespace utils; //SC: in src, has todo

static void format_state (char* state, char length) {
  if (*C_CAR_PTR < 0 || *CARS_ADDR_PTR == 0 || *OPT_ADDR_PTR == 0) {
    state[0] = 0;
    return;
  }

  char c_mode = *(char*)(*OPT_ADDR_PTR + 0x12C);
  switch (c_mode) { //? - SC: ?????? what is this
    case 1: //* - SC: looks like, when referencing 47-57, these are codes for different parts of the game?
    case 4:
    case 33:
      break;
    default:
      state[0] = 0;
      return;
  }

  const int car_addr = (*CARS_ADDR_PTR + (*C_CAR_PTR * 0xD0));
  const char* const car_name = (char*)(car_addr + 0x10);

  const auto car_itr = CAR_TABLE.find(car_name);

  if (car_itr != CAR_TABLE.end()) {
    sprintf_s(state, length, "%s", car_itr->second); //* - SC: sprintf_s is built into C. hehe, oopsie! focus on format_details and format_state instead
  } else {
    const char* const car_brand = (char*)(car_addr + 0x40);
    sprintf_s(state, length, "%s %s", car_brand, car_name);
  }
}

static void format_details (char* details, char length) {
  if (*OPT_ADDR_PTR == 0 || *HEAT_PTR == 0 || *PURSUIT_PTR == 0) { return; }

  char c_mode = *(char*)(*OPT_ADDR_PTR + 0x12C);
  int heat = 0;
  int in_pursuit = 0;

  switch (c_mode) {
    case 4:
      sprintf_s(details, length, "Quick Race");
      break;
    case 1:
      //* - SC: adjusted, hopefully it works now
      heat = *(int*) (*HEAT_PTR+0x104);
      in_pursuit = *(int*) (*PURSUIT_PTR+0x100);
      if (in_pursuit == 1) {
        sprintf_s(details, length, "IN PURSUIT! - Heat %d", heat);
      } else {sprintf_s(details, length, "Career");}
      break;
    case 33:
      sprintf_s(details, length, "Customization Shop");
      break;
    default: //* - SC: was a bug in 2021 that wictor fixed where customization shop was improperly displayed because it defaults to main menu
      sprintf_s(details, length, IN_MENU); //? - where did 33 come from, though... must find out
  }
}

//* - SC: use below for debugging
// void initConsole() {
//   if (AllocConsole()) {
//     FILE* fDummy;
//     freopen_s(&fDummy, "CONIN$", "r", stdin);
//     freopen_s(&fDummy, "CONOUT$", "w", stderr);
//     freopen_s(&fDummy, "CONOUT$", "w", stdout);
//   }
// }

static DWORD WINAPI ThreadEntry (LPVOID lpParam) {
  char state[64]; //* - SC: these are best left untouched according to original modder
  char details[64]; //* - SC: 128 bytes /is/ a lot, after all...
  
  Discord_Initialize(APP_ID, 0, 0, 0);
  // initConsole();
  std::cout << "-- NSFWMW_RPC - SC --\nFrom here on out, console outputs are by NSFMW_RPC-SC.\nI know what I'm doing, I swear. Debug only." << std::endl;

  DiscordRichPresence discord_presence;
  memset(&discord_presence, 0, sizeof(discord_presence));

  discord_presence.startTimestamp = time(0);
  discord_presence.largeImageKey = IMG_KEY;
  discord_presence.largeImageText = IMG_TXT;

  discord_presence.state = state;
  discord_presence.details = details;

  details[0] = 0;

  while (1) {
    format_state(state, sizeof(state));
    format_details(details, sizeof(details));

    Discord_UpdatePresence(&discord_presence);
    Discord_RunCallbacks();
    Sleep(UPD_INTVL);
  }
}

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
      CreateThread(0, 0, ThreadEntry, 0, 0, 0);
      break;
    case DLL_PROCESS_DETACH:
      Discord_Shutdown();
      break;
  }

  return 1;
}
