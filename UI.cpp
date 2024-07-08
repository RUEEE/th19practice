#include "pch.h"
#include "UI.h"
#include "Utils.h"
#include <windows.h>
#include "injection.h"


#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx9.h"
#include "imgui\imgui_impl_win32.h"

#include "Address.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <vector>
#include <sstream>
#include <string>
#include <format>
#include <unordered_map>
#include <format>
#include <fstream>


bool g_is_change_card = true;
int g_cards[5] = { 0,2,4,6,8 };

int g_BGM = 0;
int g_BG = 0;

void __fastcall SetStoryModeCard(int thiz)
{
    if (!g_is_change_card)
        return;
    int stage = *(DWORD*)addr_stage_count;
    if (stage >= 1 && stage <= 5)
    {
        *(DWORD*)(thiz + 0x18) = g_cards[stage-1];
    }
}

__declspec(naked) void SetStoryModeCard_53D2D4_Wrap()
{
    __asm
    {
        pushad
        mov ecx,esi
        call SetStoryModeCard
        popad
        ret
    }
}
//ABGR
#define COL_HP 0xFFFF55FF
#define COL_HP_CON 0xAA555555
void ShowHP()
{
    auto clientSz = ImGui::GetMainViewport()->Size;
    auto p = ImGui::GetOverlayDrawList();
    DWORD pEnmbase[] = { addr_enm_list_1p,addr_enm_list_2p };
    bool is_1p[] = { true,false };
    for (int i = 0; i < 2; i++)
    {
        if (auto pEnmLst = VALUED(pEnmbase[i]); pEnmLst) {
            for (auto iter = VALUED(pEnmLst + 0x4B50); iter; iter = VALUED(iter + 4)) {
                DWORD curEnm= VALUED(iter);
                DWORD flag = VALUED(curEnm + 0x639C);
                if ((flag & 0x21) == 0){
                    int HP = VALUED(curEnm + 0x6238);
                    float x = VALUEF(curEnm + 0x1278);
                    float y = VALUEF(curEnm + 0x127C);
                    ImVec2 client = GetClientFromStage(ImVec2(x, y), clientSz, is_1p[i]);
                    client.y -= 64.0f * (ImGui::GetMainViewport()->Size.y / 960.0f);
                    auto text=std::format("{:^5}", HP);
                    ImFont font(*ImGui::GetFont());
                    int fontSz = 30;
                    auto fontArea=font.CalcTextSizeA(fontSz, FLT_MAX, 0.0f, text.c_str());
                    client.x -= fontArea.x / 2.0f;
                    p->AddRectFilled(client, ImVec2(client.x + fontArea.x, client.y + fontArea.y), COL_HP_CON, 0.0f);
                    p->AddText(&font, fontSz, client, COL_HP, text.c_str(), NULL);
                }
            }
        }
    }
}

std::vector<std::vector<int>> hit_attack= { 
{300,300,400,200                     },
{300,300,500,400                     },
{300,300,500,300,600                 },
{300,350,500,300,500,300             },
{300,300,150,300,300,300,100         },
{300,200,200,200,200,400,200,300,100 } 
};


extern bool g_rep_UI;

void PracticeUI()
{
    {
        static bool is_show_HP = true;
        if (is_show_HP)
            ShowHP();
        ImGui::SetNextWindowSizeConstraints(ImVec2(380.0f, 310.0f), ImVec2(640.0f, 480.0f));

        //[[005AE4E0]+34]+8
        int hit = 0;
        int hit_remain = 0;
        if (VALUED(addr_hits_ps_count_base_1p) && VALUED(VALUED(addr_hits_ps_count_base_1p) + 0x34))
        {
            hit=VALUED(VALUED(VALUED(addr_hits_ps_count_base_1p) + 0x34) + 8);
            int cur_stage = VALUED(addr_stage_count);
            auto &h=hit_attack[cur_stage - 1];
            int ch= VALUED(VALUED(VALUED(addr_hits_ps_count_base_1p) + 0x34) + 4);
            if (ch >= 0 && ch < h.size()){
                hit_remain = h[ch]-hit;
            }
        }
        int hh = 0;
        if (VALUED(addr_ppl_1p))
        {
            hh=VALUED(VALUED(addr_ppl_1p) + 0x22BC);
        }
        ImGui::Begin(std::format("hit: {:>4} , remain: {:>4} , imm: {:>4} ###boss",hit,hit_remain,hh).c_str());
        static int x = 0;
        
        ImGui::SliderInt("###boss attack", &x, 0, 17, "boss attack %d");
        ImGui::SameLine();
        if (ImGui::Button("attack") || (ImGui::IsKeyDown(VK_BACK) && !ImGui::IsWindowHovered()))
        {
            //int(__thiscall * sb_4FC7D0_Boss_BossAttack)(int thiz, int a2);
            //sb_4FC7D0_Boss_BossAttack = (decltype(sb_4FC7D0_Boss_BossAttack))(0x4FC7D0);
            //sb_4FC7D0_Boss_BossAttack(*(DWORD*)(0x5AE4E4), x);

            auto thiz = *(DWORD*)(addr_attack_vtbl_2p);
            if (thiz != 0)
            {
                auto v8 = *(DWORD**)(thiz + 0x18);
                if (v8 != 0)
                {
                    (*(void(__thiscall**)(DWORD*, int))(*v8 + 0x1C))(v8, x);
                }
            }
        }

        static char buf[100]="0 2 4 6 8";
        auto charfilter = [](ImGuiInputTextCallbackData* data)->int
        {
            if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter){
                if ((data->EventChar >= '0' && data->EventChar <= '9') || data->EventChar == ' '){
                    return 0;
                }else{
                    data->EventChar = 0;
                    return 1;
                }
            }
            return 0;
        };
        static bool is_ok = true;
        if (ImGui::InputTextWithHint("###cards", "cards needed: %d %d %d %d %d", buf, sizeof(buf),ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCharFilter, charfilter,0)){
            if (sscanf_s(buf, "%d%d%d%d%d", g_cards, g_cards + 1, g_cards + 2, g_cards + 3, g_cards + 4)!=5){
                is_ok = false;
            }else{
                is_ok = true;
                for (int i = 0; i < 5; i++) {
                    if ((g_cards[i] >= 0 && g_cards[i] <= 38) == false) {
                        is_ok = false; break;
                    }
                }
            }
        }
        int default_cards[] = { 0,2,4,6,8 };
        ImGui::SameLine();
        if (is_ok){
            ImGui::LabelText("###info", "cards ok");
        }else{
            for (int i = 0; i < 5; i++)
                g_cards[i] = default_cards[i];
            ImGui::LabelText("###info", "cards failed");
        }

        //[[[005AE488 / 005AE4C4]+ CC] + 2C] + 34
        //[[[005AE488 / 005AE4C4]+ CC] + 2C] + 38
        static int diff = 7;
        if (ImGui::SliderInt(" difficulty rank", &diff, 0, 7, "diff %d"))
        {
            *(DWORD*)(addr_diff_rank) = diff;
        }

        static int lv[4] = { 7,7,7,7 };
        if (ImGui::SliderInt4(" LV setting", lv, 0, 7, "Lvs"))
        {
            if (Address<DWORD>(addr_att_lv_base_1p).GetValue())
                addr_ex_att_lv_1p.SetValue(lv[0]);
            if (Address<DWORD>(addr_att_lv_base_2p).GetValue())
                addr_ex_att_lv_2p.SetValue(lv[1]);
            if (Address<DWORD>(addr_att_lv_base_1p).GetValue())
                addr_boss_att_lv_1p.SetValue(lv[2]);
            if (Address<DWORD>(addr_att_lv_base_2p).GetValue())
                addr_boss_att_lv_1p.SetValue(lv[3]);
        }
        static int power[2];
        if (ImGui::SliderInt2(" set power", power, 0, 2500, "%d"))
        {
            *(DWORD*)(addr_C_1p) = power[0];
            *(DWORD*)(addr_C_2p) = power[1];
        }

        //checkboxes
        {
            static bool lock_lv = false;
            ImGui::Checkbox("lock LV", &lock_lv);
            if (lock_lv) {
                if (Address<DWORD>(addr_att_lv_base_1p).GetValue())
                    addr_ex_att_lv_1p.SetValue(lv[0]);
                if (Address<DWORD>(addr_att_lv_base_2p).GetValue())
                    addr_ex_att_lv_2p.SetValue(lv[1]);
                if (Address<DWORD>(addr_att_lv_base_1p).GetValue())
                    addr_boss_att_lv_1p.SetValue(lv[2]);
                if (Address<DWORD>(addr_att_lv_base_2p).GetValue())
                    addr_boss_att_lv_1p.SetValue(lv[3]);
            }
            static bool lock_power1 = false;
            static bool lock_power2 = false;
            ImGui::SameLine();
            ImGui::Checkbox("lock 1P power", &lock_power1);
            if (lock_power1)
                *(DWORD*)(addr_C_1p) = power[0];
            ImGui::SameLine();
            ImGui::Checkbox("lock 2P power", &lock_power2);
            if (lock_power2)
                *(DWORD*)(addr_C_2p) = power[1];

            ImGui::Checkbox("change card", &g_is_change_card);
            ImGui::SameLine();
            static bool is_invincible = false;
            static BYTE original_code[] = { 0xC7,0x47,0x10,0x04,0x00,0x00,0x00 };
            if (ImGui::Checkbox("invincible", &is_invincible))
            {
                if (is_invincible)
                {
                    for (int i = 0; i < 7; i++) {
                        auto addr = Address<BYTE>(addr_player_die_state4 + i);
                        original_code[i] = addr.GetValue();
                        addr.SetValue(0x90);
                    }
                }
                else {
                    for (int i = 0; i < 7; i++) {
                        auto addr = Address<BYTE>(addr_player_die_state4 + i);
                        addr.SetValue(original_code[i]);
                    }
                }
            }
            ImGui::SameLine();
            ImGui::Checkbox("show HP", &is_show_HP);
        }
        
        //kill player
        {
            if (ImGui::Button("kill 1P")) {
                *(DWORD*)(addr_life_count_1p) = 0;
                auto p1 = *(DWORD*)(addr_ppl_1p);
                if (p1)
                    *(DWORD*)(p1 + 0x10) = 4;
            }
            ImGui::SameLine();
            if (ImGui::Button("kill 2P")) {
                *(DWORD*)(addr_life_count_2p) = 0;
                auto p2 = *(DWORD*)(addr_ppl_2p);
                if (p2)
                    *(DWORD*)(p2 + 0x10) = 4;
            }
            ImGui::SameLine();
            if (ImGui::Button("kill 2P & 1P")) {
                *(DWORD*)(addr_life_count_1p) = 0;
                *(DWORD*)(addr_life_count_2p) = 0;
                auto p2 = *(DWORD*)(addr_ppl_2p);
                if (p2)
                    *(DWORD*)(p2 + 0x10) = 4;
                auto p1 = *(DWORD*)(addr_ppl_1p);
                if (p1)
                    *(DWORD*)(p1 + 0x10) = 4;
            }
        }
        
        //get cards
        {
            static char buf2[1000];
            static std::vector<int> cardIds;
            
            if(ImGui::InputTextWithHint("cardsID###cards2", "cards needed (%d )+", buf2, sizeof(buf2), ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCharFilter, charfilter, 0))
            {
                std::stringstream ss;
                ss << buf2;
                int c=0;
                cardIds.clear();
                while (ss>>c){
                    if(c>=0 && c<=38)
                        cardIds.push_back(c);
                }
            }

            int(__thiscall* sb_4CFAE0_init_card)(DWORD thiz, int a2, int cardID);
            sb_4CFAE0_init_card = (decltype(sb_4CFAE0_init_card))(addr_sb_4CFAE0_add_card);
            int(__thiscall* sub_532E70)(DWORD pPlayer, int a2);
            sub_532E70 = (decltype(sub_532E70))addr_sb_532E70_prob_InitPlayerCard;
            int(__thiscall* sb_4CEFC0_showCard_leftBottom)(DWORD pAbility);
            sb_4CEFC0_showCard_leftBottom = (decltype(sb_4CEFC0_showCard_leftBottom))addr_sb_4CEFC0_showCard_leftBottom;

            if (ImGui::Button("get card1")) {
                if (VALUED(addr_ppl_1p)){
                    for(auto card_id :cardIds)
                        int pCard = sb_4CFAE0_init_card(VALUED(addr_card_base_1p), card_id, 0);
                    sb_4CEFC0_showCard_leftBottom(VALUED(addr_card_base_1p));
                    sub_532E70(VALUED(addr_ppl_1p), 1);
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("get card2")) {
                if (VALUED(addr_ppl_2p)) {
                    for (auto card_id : cardIds)
                        int pCard = sb_4CFAE0_init_card(VALUED(addr_card_base_2p), card_id, 0);
                    sb_4CEFC0_showCard_leftBottom(VALUED(addr_card_base_2p));
                    sub_532E70(VALUED(addr_ppl_2p), 1);
                }
            }
        }
        {
            static bool forceBGM = false;
            static bool forceBG = false;
            bool is_changed_value = false;

            is_changed_value = is_changed_value || ImGui::Checkbox("BGM###forceBGM", &forceBGM);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(140.0f);
            is_changed_value = is_changed_value || ImGui::SliderInt("###BGM", &g_BGM, 0, 18);
            
            is_changed_value = is_changed_value || ImGui::Checkbox("BG ###forceBG", &forceBG);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(140.0f);
            is_changed_value = is_changed_value || ImGui::SliderInt("##BG", &g_BG, 0, 16);
            
            if (is_changed_value)
            {
                if (forceBGM)
                {
                    Address<BYTE>(addr_bgm_set).SetValue(0xB8);
                    Address<DWORD>((addr_bgm_set+1)).SetValue(g_BGM);
                }else{
                    Address<DWORD>(addr_bgm_set).SetValue(code_bgm_set_orig_1);
                    Address<BYTE>((addr_bgm_set + 1)).SetValue(code_bgm_set_orig_2);
                }
                if (forceBG)
                {
                    Address<BYTE>(addr_bg_set).SetValue(0xB9);
                    Address<DWORD>((addr_bg_set+1)).SetValue(g_BG);
                    Address<BYTE>((addr_bg_set+5)).SetValue(0x90);
                }else{
                    Address<DWORD>(addr_bg_set).SetValue(code_bg_set_orig_1);
                    Address<WORD>((addr_bg_set+4)).SetValue(code_bg_set_orig_2);
                }
            }
        }
        
        ImGui::End();
        return;
    }
}


void SetUI(IDirect3DDevice9* device)
{
    PracticeUI();
    return;
}