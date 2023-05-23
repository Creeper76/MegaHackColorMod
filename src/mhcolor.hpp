#define __midhook void __declspec(naked)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

#include <extensions2.hpp>

#include <json.hpp>

#include <minhook.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>


using namespace MegaHackExt;


namespace MegaHackColor {

    static char* Base;

    static const std::string CorrectVersion = "v7.1.1-GM1";

    static Colour MegaHackColorValue = {0xAD, 0x62, 0xEE};
    static Colour LastColor = {0xAD, 0x62, 0xEE};

    static double RainbowSpeed = 1;
    static int RainbowSaturation = 100;
    static int RainbowValue = 100;
    static bool RainbowBool = 0;

    static ColourPicker* Picker;

    struct MidHook { // Target, ReturnOffset, Detour, DetourSize
        uintptr_t Target;
        uintptr_t ReturnOffset;

        uintptr_t Detour;
        int DetourSize;

        void PlaceHook() {
            MH_CreateHook(reinterpret_cast<void*>(Base + Target),
                          reinterpret_cast<void*>(Detour),
                          NULL);

            DWORD OldProt;
            VirtualProtect(reinterpret_cast<void*>(Detour), DetourSize, PAGE_EXECUTE_READWRITE, &OldProt);
            *(uintptr_t*)(Detour + DetourSize - 0x4) = (uintptr_t)(Base + Target + ReturnOffset);
            VirtualProtect(reinterpret_cast<void*>(Detour), DetourSize, OldProt, &OldProt);
        }
    };

    namespace Util {
        int FindInMHDLL(const char* Haystack) {
            return std::search(Base, Base + 0x800000, Haystack, Haystack + strlen(Haystack)) - Base;
        }

        Colour HSVtoRGB(float H, float S, float V) {
            if(H > 360 || H < 0 || S > 100 || S < 0 || V > 100 || V < 0) {
                return {0, 0, 0};
            }

            float s = S/100;
            float v = V/100;
            float C = s*v;
            float X = C*(1-abs(fmod(H/60.0, 2)-1));
            float m = v-C;
            float r,g,b;
            if(H >= 0 && H < 60){
                r = C,g = X,b = 0;
            }
            else if(H >= 60 && H < 120){
                r = X,g = C,b = 0;
            }
            else if(H >= 120 && H < 180){
                r = 0,g = C,b = X;
            }
            else if(H >= 180 && H < 240){
                r = 0,g = X,b = C;
            }
            else if(H >= 240 && H < 300){
                r = X,g = 0,b = C;
            }
            else{
                r = C,g = 0,b = X;
            }
            unsigned char R = (r+m)*255;
            unsigned char G = (g+m)*255;
            unsigned char B = (b+m)*255;
            return {R, G, B};
        }
    }

    namespace Hooks {
        __midhook Titlebar() {
            __asm {
                movzx eax, BYTE PTR [MegaHackColorValue + 2]
                push eax
                movzx eax, BYTE PTR [MegaHackColorValue + 1]
                push eax
                movzx eax, BYTE PTR [MegaHackColorValue]
                push eax

                // I have to do this cus shitass c++ inline asm
                // doesnt let me do jmp 0x12345678
                push 0x12345678
                ret
            }
        }

        __midhook CheckBoxIndicator() {
            __asm {
                push [MegaHackColorValue + 2]
                push [MegaHackColorValue + 1]
                push [MegaHackColorValue]

                push 0xDEADBEEF
                ret
            }
        }

        __midhook CheckBoxIndicatorOpaque() {
            __asm {
                push [MegaHackColorValue + 2]
                push [MegaHackColorValue + 1]
                push [MegaHackColorValue]

                push 0xAFDAFD88
                ret
            }
        }

        __midhook EnabledText() {
            __asm {
                push edx

                mov edx, dword ptr [MegaHackColorValue]
                or edx, 0xFF000000
                mov [ecx + eax * 8 + 0x5050], edx

                pop edx

                push 0x18718769
                ret
            }
        }

        __midhook EnabledTextOpaque() {
            __asm {
                push edx

                mov edx, dword ptr [MegaHackColorValue]
                and edx, 0x00FFFFFF
                or edx, 0x40000000
                mov [ecx + eax * 8 + 0x5050], edx

                pop edx

                push 0x12341234
                ret
            }
        }

        __midhook SelectionBoxText() {
            __asm {
                push ecx

                cmp ecx, 0xFFE3EBEB
                je movval

                mov ecx, dword ptr [MegaHackColorValue]
                or ecx, 0xFF000000

              movval:
                mov [edx + eax * 8 + 0x5050], ecx

                pop ecx
                push 0x18769187
                ret
            }
        }

        __midhook SelectionBoxIndicator() {
            __asm {
                push [MegaHackColorValue + 2]
                push [MegaHackColorValue + 1]
                push [MegaHackColorValue]

                push 0x99999999
                ret
            }
        }
    }

    void SaveConfig() {
        nlohmann::json Data;

        Data["MegaHackColorValue"] = (RainbowBool ? LastColor : MegaHackColorValue).toHexString();

        Data["RainbowSpeed"] = RainbowSpeed;
        Data["RainbowSaturation"] = RainbowSaturation;
        Data["RainbowValue"] = RainbowValue;
        Data["RainbowBool"] = RainbowBool;

        if(!std::filesystem::is_directory("MegaHackColor")) {
            std::filesystem::create_directory("MegaHackColor");
        }

        std::ofstream ConfigFile("MegaHackColor/config.json", std::ios::out | std::ios::trunc);
        ConfigFile << Data.dump(4);
        ConfigFile.close();
    }

    void LoadConfig() {
        if(!std::filesystem::exists("MegaHackColor/config.json")) {
            SaveConfig();
            return;
        }

        std::ifstream ConfigFile("MegaHackColor/config.json");

        try {
            nlohmann::json Data = nlohmann::json::parse(ConfigFile);

            MegaHackColorValue.fromHexString(Data["MegaHackColorValue"]);
            LastColor = MegaHackColorValue;

            RainbowSpeed = Data["RainbowSpeed"];
            RainbowSaturation = Data["RainbowSaturation"];
            RainbowValue = Data["RainbowValue"];
            RainbowBool = Data["RainbowBool"];
        } catch (std::exception& E) {
            MegaHackColorValue = {0xAD, 0x62, 0xEE};
            LastColor = MegaHackColorValue;

            RainbowSpeed = 1.0;
            RainbowSaturation = 100;
            RainbowValue = 100;
            RainbowBool = 0;
        }

        ConfigFile.close();
    }

    std::string GetMHVersion() {
        return reinterpret_cast<char*>(Base + Util::FindInMHDLL("Successfully updated to version ") + 0x20);
    }

    bool Init() {
        if(!std::filesystem::exists("hackpro.dll")) {
            return 0;
        }

        while(GetModuleHandle("hackpro.dll") == 0) {};
        Base = reinterpret_cast<char*>(GetModuleHandle("hackpro.dll"));

        MH_Initialize();

        return 1;
    }

    void SetupHooks() {
        std::vector<MidHook> Hooks =
        {
            {0x147165, 0x18,
             reinterpret_cast<uintptr_t>(&Hooks::Titlebar), 0x1D},

            {0x14A9F2, 0xC,
             reinterpret_cast<uintptr_t>(&Hooks::CheckBoxIndicator), 0x17},

            {0x14AA25, 0xC,
             reinterpret_cast<uintptr_t>(&Hooks::CheckBoxIndicatorOpaque), 0x17},

            {0x14AA13, 0xB,
             reinterpret_cast<uintptr_t>(&Hooks::EnabledText), 0x1A},

            {0x14AA42, 0xB,
             reinterpret_cast<uintptr_t>(&Hooks::EnabledTextOpaque), 0x20},

            {0x14A26E, 0x7,
             reinterpret_cast<uintptr_t>(&Hooks::SelectionBoxText), 0x22},

            {0x14A354, 0xC,
             reinterpret_cast<uintptr_t>(&Hooks::SelectionBoxIndicator), 0x17}
        };

        for(auto& i : Hooks) {
            i.PlaceHook();
        }

        MH_EnableHook(MH_ALL_HOOKS);
    }

    void CreateModWindow() {
        Window* Window = Window::Create("Mega Hack Color");

        Picker = ColourPicker::CreateEx({0xAD, 0x62, 0xEE}, MegaHackColorValue,
        [](ColourPicker*, Colour Color) {
            MegaHackColorValue = Color;
            if(!RainbowBool) {
                LastColor = Color;
                SaveConfig();
            }
        });

        CheckBox* RainbowCheckbox = CheckBox::CreateEx("Rainbow", RainbowBool, 1,
        [](CheckBox* Self, bool Toggle) {
            RainbowBool = Toggle;
            if(!Toggle) {
                Picker->set(LastColor);
            }
            SaveConfig();
        });

        Spinner* Saturation = Spinner::CreateEx("S: ", "%", RainbowSaturation, 1,
        [](Spinner* Self, double Value) {
            RainbowSaturation = (Value > 100) ? 100 : (Value < 0) ? 0 : Value;
            Self->set(RainbowSaturation, 0);
            SaveConfig();
        });

        Spinner* Value = Spinner::CreateEx("V: ", "%", RainbowValue, 1,
        [](Spinner* Self, double Value) {
            RainbowValue = (Value > 100) ? 100 : (Value < 0) ? 0 : Value;
            Self->set(RainbowValue, 0);
            SaveConfig();
        });

        Spinner* Speed = Spinner::CreateEx("Speed: ", "x", RainbowSpeed, 1.0,
        [](Spinner* Self, double Value) {
            RainbowSpeed = (Value > 16) ? 16 : (Value < 0.1 /*Change from 1 to .1*/) ? 1 : Value;
            Self->set(RainbowSpeed, 0);
            SaveConfig();
        });

        Label* CreditsLine1 = Label::Create("Original by: Ikszyon");
        Label* CreditsLine2 = Label::Create("Modifyed by: Creeper76");

        Window->addElements({Picker,
                             HorizontalLayout::Create(RainbowCheckbox, Speed),
                             HorizontalLayout::Create(Saturation, Value),
                             CreditsLine1,
                             CreditsLine2});
    }

    void OutdatedVersionWindow() {
        Window* Window = Window::Create("Mega Hack Color");

        Label* VersionLabel1 = Label::Create("Incorrect Version");

        std::string Label2Text = "Installed Version: " + GetMHVersion();
        Label* VersionLabel2 = Label::Create(Label2Text.c_str());

        std::string Label3Text = "Mod made for: " + CorrectVersion;
        Label* VersionLabel3 = Label::Create(Label3Text.c_str());

        Button* GitHub = Button::CreateEx("Ikszyon's GitHub",
        [](Button*) {
            ShellExecute(NULL, NULL, "https://github.com/Ikszyon/MegaHack-Recolor/releases/latest", NULL, NULL, SW_SHOW);
        });

        Button* GitHub2 = Button::CreateEx("Creeper76's GitHub",
        [](Button*) {
            ShellExecute(NULL, NULL, "https://github.com/Creeper76?tab=repositories", NULL, NULL, SW_SHOW);
        });

        Window->addElements({VersionLabel1,
                             VersionLabel2,
                             VersionLabel3,
                             GitHub,
                             GitHub2});
    }

}