#pragma once
#include "windows.h"

#define addr_cardaddr 0x00553A54
#define addr_stage_count 0x0062BA80
#define addr_enm_list_1p 0x005D1A68
#define addr_enm_list_2p 0x005D1AA4
#define addr_hits_ps_count_base_1p 0x005D1AD0

#define addr_ppl_1p 0x005D1A64
#define addr_ppl_2p 0x005D1AA0

#define addr_attack_vtbl_2p 0x005D1AD4

#define addr_att_lv_base_1p 0x005D1A78
#define addr_att_lv_base_2p 0x005D1AB4
#define addr_ex_att_lv_1p ((Address<DWORD>(0x005D1A78) + 0xCC + 0x2C + 0x38))
#define addr_ex_att_lv_2p ((Address<DWORD>(0x005D1AB4) + 0xCC + 0x2C + 0x38))
#define addr_boss_att_lv_1p ((Address<DWORD>(0x005D1A78) + 0xCC + 0x2C + 0x3C))
#define addr_boss_att_lv_2p ((Address<DWORD>(0x005D1AB4) + 0xCC + 0x2C + 0x3C))

#define addr_C_1p 0x0062B0E4
#define addr_C_2p 0x0062B1AC

#define addr_player_die_state4 0x00545EE3

#define addr_life_count_1p 0x0062B114
#define addr_life_count_2p 0x0062B1DC

#define addr_sb_4CFAE0_add_card 0x4DEBA0
#define addr_sb_532E70_prob_InitPlayerCard 0x5483D0
#define addr_sb_4CEFC0_showCard_leftBottom 0x4DE0E0

#define addr_card_base_1p 0x5D1A80
#define addr_card_base_2p 0x5D1ABC

#define addr_diff_rank 0x0062B250

#define addr_bgm_set 0x00518B3C
#define code_bgm_set_orig_1 0xEF2D2FE8
#define code_bgm_set_orig_2 0xFF

#define addr_bg_set 0x00518206
#define code_bg_set_orig_1 0x5C20888B
#define code_bg_set_orig_2 0x005C

void Assert(const WCHAR* s);
void Hook(LPVOID addr_inject, size_t move_bytes, LPVOID callee);
//inject a call( void call(void) ) in anywhere
void HookCall(LPVOID addr_call, LPVOID callee);
//the function must be same with the function injected

void HookD3D();

BOOL hookVTable(void* pInterface, int index, void* hookFunction, void** oldAddress);
BOOL unhookVTable(void* pInterface, int index, void* oldAddress);

void** findImportAddress(HANDLE hookModule, LPCSTR moduleName, LPCSTR functionName);
BOOL hookIAT(HANDLE hookModule, LPCSTR moduleName, LPCSTR functionName, void* hookFunction, void** oldAddress);
BOOL unhookIAT(HANDLE hookModule, LPCSTR moduleName, LPCSTR functionName);

HWND GetMainWindow();
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

void HookWindow();
void unHook();

void InjectAll();

void HookCreateMutex();