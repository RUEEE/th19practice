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
#include <queue>
#include <filesystem>

#include <iostream>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <commdlg.h>

#define MAX_SIZE_SIGN 256
struct Rep
{
    char sign[MAX_SIZE_SIGN];
    int original_seed[5][7];    //seed[type][stage],stage=0,1,2,3,4,5,6
    std::queue<DWORD> key_p1;
    std::queue<DWORD> key_p2;
    std::queue<DWORD> key_msg;
    std::queue<int> random_wave_index;
};

Rep g_rep;
bool g_is_playing_rep = false;


void LPWSTR2LPSTR(LPWSTR lpwszStrIn,char* pszOut)
{
    int nInputStrLen = wcslen(lpwszStrIn);
    int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
    memset(pszOut, 0x00, nOutputStrLen);
    WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
    return;
}

std::string chooseFile_exist() {
    OPENFILENAME ofn;
    TCHAR szOpenFileName[MAX_PATH] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szOpenFileName;
    ofn.nMaxFile = sizeof(szOpenFileName);
    ofn.lpstrFile[0] = _T('\0');
    ofn.lpstrFilter = _T("*.rep\0*.*\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = _T("play rep file");
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
    if (!GetOpenFileName(&ofn)) {
        return std::string("");
    }
    char buf[MAX_PATH * 2+2];
    LPWSTR2LPSTR(ofn.lpstrFile, buf);
    return std::string(buf);
}

std::string chooseFile() {
    OPENFILENAME ofn;
    TCHAR szOpenFileName[MAX_PATH] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szOpenFileName;
    ofn.nMaxFile = sizeof(szOpenFileName);
    ofn.lpstrFile[0] = _T('\0');
    ofn.lpstrFilter = _T("*.rep\0*.*\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = _T("save rep file");
    ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
    if (!GetOpenFileName(&ofn)) {
        return std::string("");
    }
    char buf[MAX_PATH * 2 + 2];
    LPWSTR2LPSTR(ofn.lpstrFile, buf);
    return std::string(buf);
}

DWORD __fastcall Original_GetRng(DWORD* thiz)
{
    unsigned int v2; // edi
    int v3; // ebx
    thiz[1] += 2;
    v2 = (unsigned __int16)((*(WORD*)thiz ^ 0x9630) - 25939);
    v3 = (unsigned __int16)(((4 * v2 + (v2 >> 14)) ^ 0x9630) - 25939);
    *(WORD*)thiz = 4 * v3 + ((unsigned __int16)(((4 * v2 + (v2 >> 14)) ^ 0x9630) - 25939) >> 14);
    return v3 | (v2 << 16);
}

unsigned __int16 __fastcall Original_GetRng2(DWORD* thiz)
{
    unsigned __int16 v2;
    ++thiz[1];
    v2 = (*(WORD*)thiz ^ 0x9630) - 25939;
    *(WORD*)thiz = 4 * v2 + (v2 >> 14);
    return v2;
}

DWORD __fastcall GetRng(DWORD thiz)
{
    if(thiz== 0x005AE410 || thiz== 0x005AE420 || thiz== 0x005AE428 || thiz==0x005AE430 || (VALUED(0x005AE464) && thiz==(VALUED(0x005AE464)+0x168)))
        return Original_GetRng((DWORD*)thiz);
    return g_rep.original_seed[0][1];
}

SHORT __fastcall GetRng2(DWORD thiz)
{
    if (thiz == 0x005AE410 || thiz == 0x005AE420 || thiz == 0x005AE428 || thiz == 0x005AE430 || (VALUED(0x005AE464) && thiz == (VALUED(0x005AE464) + 0x168)))
        return Original_GetRng((DWORD*)thiz);
    return g_rep.original_seed[0][1];
}

void ClearRep(Rep* pRep)
{
    pRep->key_p1 = {};
    pRep->key_p2 = {};
    pRep->key_msg = {};
    pRep->random_wave_index = {};
    memset(pRep->original_seed, 0, sizeof(pRep->original_seed));
    memset(pRep->sign, 0, sizeof(pRep->sign));
}

void SaveRep(std::ofstream& fs,Rep* rep)
{
    int sz1 = rep->key_p1.size(), sz2 = rep->key_p2.size(), sz3 = rep->key_msg.size(), sz4 = rep->random_wave_index.size();
    fs.write(rep->sign,sizeof(rep->sign));
    fs.write((const char*)(rep->original_seed),sizeof(rep->original_seed));
    fs.write((const char*)(& sz1), sizeof(int));
    fs.write((const char*)(& sz2), sizeof(int));
    fs.write((const char*)(& sz3), sizeof(int));
    fs.write((const char*)(& sz4), sizeof(int));
    for (int i = 0; i < sz1; i++) { int x = rep->key_p1.front(); fs.write((const char*)&x, sizeof(x)); rep->key_p1.pop(); }
    for (int i = 0; i < sz2; i++) { int x = rep->key_p2.front(); fs.write((const char*)&x, sizeof(x)); rep->key_p2.pop(); }
    for (int i = 0; i < sz3; i++) { int x = rep->key_msg.front(); fs.write((const char*)&x, sizeof(x)); rep->key_msg.pop(); }
    for (int i = 0; i < sz4; i++) { int x = rep->random_wave_index.front(); fs.write((const char*)&x, sizeof(x)); rep->random_wave_index.pop(); }
}

void ReadRep(std::ifstream& fs, Rep* rep)
{
    ClearRep(rep);
    int sz1 = 0, sz2 = 0, sz3 = 0,sz4=0;
    fs.read(rep->sign, sizeof(rep->sign));
    fs.read((char*)(rep->original_seed), sizeof(rep->original_seed));
    fs.read((char*)(&sz1), sizeof(int));
    fs.read((char*)(&sz2), sizeof(int));
    fs.read((char*)(&sz3), sizeof(int));
    fs.read((char*)(&sz4), sizeof(int));
    for (int i = 0; i < sz1; i++) { int x = 0; fs.read((char*)&x, sizeof(x)); rep->key_p1.push(x);}
    for (int i = 0; i < sz2; i++) { int x = 0; fs.read((char*)&x, sizeof(x)); rep->key_p2.push(x); }
    for (int i = 0; i < sz3; i++) { int x = 0; fs.read((char*)&x, sizeof(x)); rep->key_msg.push(x); }
    for (int i = 0; i < sz4; i++) { int x = 0; fs.read((char*)&x, sizeof(x)); rep->random_wave_index.push(x); }
}

void RecordSeed(Rep* pRep,int stage)
{
    pRep->original_seed[0][stage] = VALUED(0x005AE410);
    pRep->original_seed[1][stage] = VALUED(0x005AE420);
    pRep->original_seed[2][stage] = VALUED(0x005AE428);
    pRep->original_seed[3][stage] = VALUED(0x005AE430);
    pRep->original_seed[4][stage] = VALUED(0x005AE464)?VALUED(VALUED(0x005AE464) + 0x168):0;
}

void SetSeed(Rep* pRep, int stage)
{
    VALUED(0x005AE410) = pRep->original_seed[0][stage];
    VALUED(0x005AE420) = pRep->original_seed[1][stage];
    VALUED(0x005AE428) = pRep->original_seed[2][stage];
    VALUED(0x005AE430) = pRep->original_seed[3][stage];
    if(VALUED(0x005AE464))
        VALUED(VALUED(0x005AE464) + 0x168)=pRep->original_seed[4][stage];
}

void SetPlayer(){
    int stage = VALUED(0x006082C0);
    if (!g_is_playing_rep)
    {
        if (VALUED(0x005AE4B0) == 0) {//p2 is not inited
        
            if (stage == 0 || stage == 1)
                ClearRep(&g_rep);
            RecordSeed(&g_rep, stage);
        }
    }else{
        if (VALUED(0x005AE4B0) == 0) {//p2 is not inited
            SetSeed(&g_rep, stage);
        }
    }
}

int __fastcall m_GetRandomWaveIndex(DWORD thiz,int edx,int a2)
{
    if (!g_is_playing_rep)
    {
        int index=*(DWORD*)(thiz + 4 * (*(DWORD*)(thiz + 4 * a2 + 0x5A4) % *(DWORD*)(thiz + 0x9A4)) + 0x1A4);
        g_rep.random_wave_index.push(index);
        return index;
    }else{
        int index = 0;
        if (g_rep.random_wave_index.empty()){
            LogError("no wave index");
        }else{
            index = g_rep.random_wave_index.front();
            g_rep.random_wave_index.pop();
        }
        return index;
    }
}

void __fastcall PlayerState(DWORD thiz)
{
    int index_player = VALUED(thiz + 0x14C40);
    int keyboardindex = VALUED(VALUED(0x5AE3A0) + 4 * index_player + 0x2E30);
    DWORD* p_key = (DWORD*)(VALUED(0x005AE390 + 4 * keyboardindex) + 0x29C);
    DWORD* p_key_s = (DWORD*)(VALUED(0x005AE390 + 4 * keyboardindex) + 0x2A8);
    if (!g_is_playing_rep){
        if (thiz == VALUED(0x005AE474)){// 1P
            g_rep.key_p1.push(*p_key);
        }else{
            g_rep.key_p2.push(*p_key);
        }
    }else{
        if (thiz == VALUED(0x005AE474)) {// 1P
            if (g_rep.key_p1.empty())
            {
                LogError("No P1 key");
                g_is_playing_rep = false;
            }else{
                DWORD lst_key = *p_key;
                *p_key = g_rep.key_p1.front();
                *p_key_s = lst_key ^ (*p_key);
                g_rep.key_p1.pop();
            }
        }else {
            if (g_rep.key_p2.empty())
            {
                LogError("No P2 key");
                g_is_playing_rep = false;
            }else{
                DWORD lst_key = *p_key;
                *p_key = g_rep.key_p2.front();
                *p_key_s = lst_key ^ (*p_key);
                g_rep.key_p2.pop();
            }
        }
    }
}

void __fastcall Msg(DWORD thiz)
{
    int index = VALUED(thiz + 0x1F8);
    int keyboardindex = VALUED(VALUED(0x5AE3A0) + 4 * index + 0x2E30);
    DWORD* p_key = (DWORD*)(VALUED(0x005AE390 + 4 * keyboardindex) + 0x10);
    if (!g_is_playing_rep) {
        g_rep.key_msg.push(*p_key);
    } else {
        if (g_rep.key_msg.empty())
        {
            LogError("No MSG key");
        }else {
            *p_key = g_rep.key_msg.front();
            g_rep.key_msg.pop();
        }
   }
}

void Injection_Rep()
{
    Address<BYTE>(0x402C00).SetValue(0xE9);
    Address<DWORD>(0x402C01).SetValue((DWORD)(GetRng)-0x402C00 - 5);

    Address<BYTE>(0x402340).SetValue(0xE9);
    Address<DWORD>(0x402341).SetValue((DWORD)(GetRng2)-0x402340 - 5);


    Address<WORD>(0x53D3F9).SetValue(0x9090);//stage pass card
	Hook((LPVOID)0x0053239F, 7, SetPlayer);
	Hook((LPVOID)0x00530ED6, 5, PlayerState);
	Hook((LPVOID)0x004FDDE5, 6, Msg);
    HookCall((LPVOID)0x4F3262, m_GetRandomWaveIndex);
}

bool g_rep_UI = false;

void RepUI()
{
    if (!g_rep_UI)
        return;
    ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f, 120.0f), ImVec2(300.0f,120.0f));
    ImGui::Begin("rep");
    auto charfilter = [](ImGuiInputTextCallbackData* data)->int
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
            if ((data->EventChar >= '0' && data->EventChar <= '9') || 
                data->EventChar == ' ' || 
                (data->EventChar >= 'a' && data->EventChar <= 'z') ||
                (data->EventChar >= 'A' && data->EventChar <= 'Z') ||
                data->EventChar == '_' ||
                data->EventChar == '-') {
                return 0;
            }
            else {
                data->EventChar = 0;
                return 1;
            }
        }
        return 0;
    };
    static char sign[MAX_SIZE_SIGN]="11";
    ImGui::Text("recorded operations: %6d;%6d;%3d",g_rep.key_p1.size(),g_rep.key_p2.size(),g_rep.random_wave_index.size());
    if (g_is_playing_rep){
        ImGui::Text("playing...");
    }else{
        ImGui::Text("not playing...");
    }
    ImGui::InputText("sign", sign, sizeof(sign),ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCharFilter, charfilter,0);
    if (ImGui::Button("save rep")){
        memcpy(g_rep.sign, sign, sizeof(g_rep.sign));
        auto repfile=chooseFile();
        std::ofstream fs(repfile, std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);
        SaveRep(fs, &g_rep);
        fs.close();
    }
    ImGui::SameLine();
    if (!g_is_playing_rep && ImGui::Button("play rep")){
        auto repfile = chooseFile_exist();
        std::ifstream fs(repfile, std::ios_base::binary | std::ios_base::in);
        if (fs.is_open()){
            ReadRep(fs, &g_rep);
            memcpy(sign, g_rep.sign, sizeof(g_rep.sign));
            g_is_playing_rep = true;
            fs.close();
        }else{
            LogError("cannot open rep file");
            MessageBoxA(NULL, "cannot open rep file", "error", MB_OK);
        }
    }
    if (g_is_playing_rep && ImGui::Button("stop rep")){
        g_is_playing_rep = false;
    }
    ImGui::End();
}