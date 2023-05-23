#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <mhcolor.hpp>

#include <extensions2.hpp>

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>


using namespace MegaHackExt;


DWORD APIENTRY MainThread(LPVOID lpParam) {

    if(!MegaHackColor::Init()) {
        return 1;
    }

    if(MegaHackColor::GetMHVersion() != MegaHackColor::CorrectVersion) {
        MegaHackColor::OutdatedVersionWindow();
        return 1;
    }

    MegaHackColor::LoadConfig();
    MegaHackColor::SetupHooks();
    MegaHackColor::CreateModWindow();

    for(int i = 0 ;; i = (i + MegaHackColor::RainbowBool) % 361) {
        Sleep((16.0 / MegaHackColor::RainbowSpeed));

        if(MegaHackColor::RainbowBool) {
            MegaHackColor::MegaHackColorValue = MegaHackColor::Util::HSVtoRGB(static_cast<float>(i), MegaHackColor::RainbowSaturation, MegaHackColor::RainbowValue);
            MegaHackColor::Picker->set(MegaHackColor::MegaHackColorValue);
        }
    }

    return 0;
}

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    switch(dwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0x1000, reinterpret_cast<LPTHREAD_START_ROUTINE>(&MainThread), NULL, 0, NULL);
            break;

        default:
            break;
    }

    return TRUE;
}