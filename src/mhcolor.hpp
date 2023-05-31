#define __midhook void __declspec(naked)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <iostream>
#include <stdlib.h>

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

    void SaveConfig();
    void CreateModWindow();

    static char* Base;

    static const std::string CorrectVersion = "v7.1.1-GM1";

    static Colour MegaHackColorValue = {0xAD, 0x62, 0xEE};
    static Colour MegaHackColorValueProfile1 = {0xAD, 0x62, 0xEE};
    static Colour MegaHackColorValueProfile2 = {0xAD, 0x62, 0xEE};
    static Colour MegaHackColorValueProfile3 = {0xAD, 0x62, 0xEE};
    static Colour MegaHackColorValueProfile4 = {0xAD, 0x62, 0xEE};
    static Colour MegaHackColorValueProfile5 = {0xAD, 0x62, 0xEE};
    static Colour MegaHackColorValueProfile6 = {0xAD, 0x62, 0xEE};
    static Colour LastColor = {0xAD, 0x62, 0xEE};

    static int Profile = 1;
    static std::string ModVersion = "Version 1.3";

    // These are used in the mhcolor.hpp file, so these values are the ones that are being used outside this file
    static double RainbowSpeed = 1;
    static int RainbowSaturation = 100;
    static int RainbowValue = 100;
    static bool RainbowBool = 0;

    // Profile 1 Values
    static double RainbowSpeedProfile1 = 1;
    static int RainbowSaturationProfile1 = 100;
    static int RainbowValueProfile1 = 100;
    static bool RainbowBoolProfile1 = 0;

    // Profile 2 Values
    static double RainbowSpeedProfile2 = 1;
    static int RainbowSaturationProfile2 = 100;
    static int RainbowValueProfile2 = 100;
    static bool RainbowBoolProfile2 = 0;

    // Profile 3 Values
    static double RainbowSpeedProfile3 = 1;
    static int RainbowSaturationProfile3 = 100;
    static int RainbowValueProfile3 = 100;
    static bool RainbowBoolProfile3 = 0;

    // Profile 4 Values
    static double RainbowSpeedProfile4 = 1;
    static int RainbowSaturationProfile4 = 100;
    static int RainbowValueProfile4 = 100;
    static bool RainbowBoolProfile4 = 0;

    // Profile 5 Values
    static double RainbowSpeedProfile5 = 1;
    static int RainbowSaturationProfile5 = 100;
    static int RainbowValueProfile5 = 100;
    static bool RainbowBoolProfile5 = 0;

    // Profile 6 Values
    static double RainbowSpeedProfile6 = 1;
    static int RainbowSaturationProfile6 = 100;
    static int RainbowValueProfile6 = 100;
    static bool RainbowBoolProfile6 = 0;

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

    void SetValue(int ActiveProfile){
        if(ActiveProfile == 1){
            MegaHackColorValue = MegaHackColorValueProfile1;
            RainbowSpeed = RainbowSpeedProfile1;
            RainbowSaturation = RainbowSaturationProfile1;
            RainbowValue = RainbowValueProfile1;
            RainbowBool = RainbowBoolProfile1;
        }else if(ActiveProfile == 2){
            MegaHackColorValue = MegaHackColorValueProfile2;
            RainbowSpeed = RainbowSpeedProfile2;
            RainbowSaturation = RainbowSaturationProfile2;
            RainbowValue = RainbowValueProfile2;
            RainbowBool = RainbowBoolProfile2;
        }else if(ActiveProfile == 3){
            MegaHackColorValue = MegaHackColorValueProfile3;
            RainbowSpeed = RainbowSpeedProfile3;
            RainbowSaturation = RainbowSaturationProfile3;
            RainbowValue = RainbowValueProfile3;
            RainbowBool = RainbowBoolProfile3;
        }else if(ActiveProfile == 4){
            MegaHackColorValue = MegaHackColorValueProfile4;
            RainbowSpeed = RainbowSpeedProfile4;
            RainbowSaturation = RainbowSaturationProfile4;
            RainbowValue = RainbowValueProfile4;
            RainbowBool = RainbowBoolProfile4;
        }else if(ActiveProfile == 5){
            MegaHackColorValue = MegaHackColorValueProfile5;
            RainbowSpeed = RainbowSpeedProfile5;
            RainbowSaturation = RainbowSaturationProfile5;
            RainbowValue = RainbowValueProfile5;
            RainbowBool = RainbowBoolProfile5;
        }else if(ActiveProfile == 6){
            MegaHackColorValue = MegaHackColorValueProfile6;
            RainbowSpeed = RainbowSpeedProfile6;
            RainbowSaturation = RainbowSaturationProfile6;
            RainbowValue = RainbowValueProfile6;
            RainbowBool = RainbowBoolProfile6;
        }
    }

    void SaveConfig() {
        nlohmann::json Data;

        Data["Profile"] = Profile;

        Data["MegaHackColorValueProfile1"] = (RainbowBoolProfile1 ? LastColor : MegaHackColorValueProfile1).toHexString();
        Data["MegaHackColorValueProfile2"] = (RainbowBoolProfile2 ? LastColor : MegaHackColorValueProfile2).toHexString();
        Data["MegaHackColorValueProfile3"] = (RainbowBoolProfile3 ? LastColor : MegaHackColorValueProfile3).toHexString();
        Data["MegaHackColorValueProfile4"] = (RainbowBoolProfile4 ? LastColor : MegaHackColorValueProfile4).toHexString();
        Data["MegaHackColorValueProfile5"] = (RainbowBoolProfile5 ? LastColor : MegaHackColorValueProfile5).toHexString();
        Data["MegaHackColorValueProfile6"] = (RainbowBoolProfile6 ? LastColor : MegaHackColorValueProfile6).toHexString();

        // Profile 1
        Data["RainbowSpeedProfile1"] = RainbowSpeedProfile1;
        Data["RainbowSaturationProfile1"] = RainbowSaturationProfile1;
        Data["RainbowValueProfile1"] = RainbowValueProfile1;
        Data["RainbowBoolProfile1"] = RainbowBoolProfile1;

        // Profile 2
        Data["RainbowSpeedProfile2"] = RainbowSpeedProfile2;
        Data["RainbowSaturationProfile2"] = RainbowSaturationProfile2;
        Data["RainbowValueProfile2"] = RainbowValueProfile2;
        Data["RainbowBoolProfile2"] = RainbowBoolProfile2;

        // Profile 3
        Data["RainbowSpeedProfile3"] = RainbowSpeedProfile3;
        Data["RainbowSaturationProfile3"] = RainbowSaturationProfile3;
        Data["RainbowValueProfile3"] = RainbowValueProfile3;
        Data["RainbowBoolProfile3"] = RainbowBoolProfile3;

        // Profile 4
        Data["RainbowSpeedProfile4"] = RainbowSpeedProfile4;
        Data["RainbowSaturationProfile4"] = RainbowSaturationProfile4;
        Data["RainbowValueProfile4"] = RainbowValueProfile4;
        Data["RainbowBoolProfile4"] = RainbowBoolProfile4;

        // Profile 5
        Data["RainbowSpeedProfile5"] = RainbowSpeedProfile5;
        Data["RainbowSaturationProfile5"] = RainbowSaturationProfile5;
        Data["RainbowValueProfile5"] = RainbowValueProfile5;
        Data["RainbowBoolProfile5"] = RainbowBoolProfile5;

        // Profile 6
        Data["RainbowSpeedProfile6"] = RainbowSpeedProfile6;
        Data["RainbowSaturationProfile6"] = RainbowSaturationProfile6;
        Data["RainbowValueProfile6"] = RainbowValueProfile6;
        Data["RainbowBoolProfile6"] = RainbowBoolProfile6;

        SetValue(Profile);

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

            Profile = Data["Profile"];

            MegaHackColorValueProfile1.fromHexString(Data["MegaHackColorValueProfile1"]);
            MegaHackColorValueProfile2.fromHexString(Data["MegaHackColorValueProfile2"]);
            MegaHackColorValueProfile3.fromHexString(Data["MegaHackColorValueProfile3"]);
            MegaHackColorValueProfile4.fromHexString(Data["MegaHackColorValueProfile4"]);
            MegaHackColorValueProfile5.fromHexString(Data["MegaHackColorValueProfile5"]);
            MegaHackColorValueProfile6.fromHexString(Data["MegaHackColorValueProfile6"]);
            if(Profile == 1){
                LastColor = MegaHackColorValueProfile1;
            }else if(Profile == 2){
                LastColor = MegaHackColorValueProfile2;
            }else if(Profile == 3){
                LastColor = MegaHackColorValueProfile3;
            }else if(Profile == 4){
                LastColor = MegaHackColorValueProfile4;
            }else if(Profile == 5){
                LastColor = MegaHackColorValueProfile5;
            }else if(Profile == 6){
                LastColor = MegaHackColorValueProfile6;
            }

            // Profile 1
            RainbowSpeedProfile1 = Data["RainbowSpeedProfile1"];
            RainbowSaturationProfile1 = Data["RainbowSaturationProfile1"];
            RainbowValueProfile1 = Data["RainbowValueProfile1"];
            RainbowBoolProfile1 = Data["RainbowBoolProfile1"];

            // Profile 2
            RainbowSpeedProfile2 = Data["RainbowSpeedProfile2"];
            RainbowSaturationProfile2 = Data["RainbowSaturationProfile2"];
            RainbowValueProfile2 = Data["RainbowValueProfile2"];
            RainbowBoolProfile2 = Data["RainbowBoolProfile2"];

            // Profile 3
            RainbowSpeedProfile3 = Data["RainbowSpeedProfile3"];
            RainbowSaturationProfile3= Data["RainbowSaturationProfile3"];
            RainbowValueProfile3 = Data["RainbowValueProfile3"];
            RainbowBoolProfile3 = Data["RainbowBoolProfile3"];

            // Profile 4
            RainbowSpeedProfile4 = Data["RainbowSpeedProfile4"];
            RainbowSaturationProfile4 = Data["RainbowSaturationProfile4"];
            RainbowValueProfile4 = Data["RainbowValueProfile4"];
            RainbowBoolProfile4 = Data["RainbowBoolProfile4"];

            // Profile 5
            RainbowSpeedProfile5 = Data["RainbowSpeedProfile5"];
            RainbowSaturationProfile5 = Data["RainbowSaturationProfile5"];
            RainbowValueProfile5 = Data["RainbowValueProfile5"];
            RainbowBoolProfile5 = Data["RainbowBoolProfile5"];

            // Profile 6
            RainbowSpeedProfile6 = Data["RainbowSpeedProfile6"];
            RainbowSaturationProfile6 = Data["RainbowSaturationProfile6"];
            RainbowValueProfile6 = Data["RainbowValueProfile6"];
            RainbowBoolProfile6 = Data["RainbowBoolProfile6"];

            SetValue(Profile);
        } catch (std::exception& E) {
            MegaHackColorValue = {0xAD, 0x62, 0xEE};
            LastColor = MegaHackColorValue;

            // Default Values
            RainbowSpeed = 1.0;
            RainbowSaturation = 100;
            RainbowValue = 100;
            RainbowBool = 0;

            // Profile 1
            RainbowSpeedProfile1 = 1.0;
            RainbowSaturationProfile1 = 100;
            RainbowValueProfile1 = 100;
            RainbowBoolProfile1 = 0;

            // Profile 2
            RainbowSpeedProfile2 = 1.0;
            RainbowSaturationProfile2 = 100;
            RainbowValueProfile2 = 100;
            RainbowBoolProfile2 = 0;

            // Profile 3
            RainbowSpeedProfile3 = 1.0;
            RainbowSaturationProfile3 = 100;
            RainbowValueProfile3 = 100;
            RainbowBoolProfile3 = 0;

            // Profile 4
            RainbowSpeedProfile4 = 1.0;
            RainbowSaturationProfile4 = 100;
            RainbowValueProfile4 = 100;
            RainbowBoolProfile4 = 0;

            // Profile 5
            RainbowSpeedProfile5 = 1.0;
            RainbowSaturationProfile5 = 100;
            RainbowValueProfile5 = 100;
            RainbowBoolProfile5 = 0;

            // Profile 6
            RainbowSpeedProfile6 = 1.0;
            RainbowSaturationProfile6 = 100;
            RainbowValueProfile6 = 100;
            RainbowBoolProfile6 = 0;
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
                if(Profile == 1){
                    MegaHackColorValueProfile1 = MegaHackColorValue;
                }else if(Profile == 2){
                    MegaHackColorValueProfile2 = MegaHackColorValue;
                }else if(Profile == 3){
                    MegaHackColorValueProfile3 = MegaHackColorValue;
                }else if(Profile == 4){
                    MegaHackColorValueProfile4 = MegaHackColorValue;
                }else if(Profile == 5){
                    MegaHackColorValueProfile5 = MegaHackColorValue;
                }else if(Profile == 6){
                    MegaHackColorValueProfile6 = MegaHackColorValue;
                }
                SaveConfig();
            }
        });

        CheckBox* RainbowCheckbox = CheckBox::CreateEx("Rainbow", RainbowBool, 1,
        [](CheckBox* Self, bool Toggle) {
            RainbowBool = Toggle;
            if(Profile == 1){
                RainbowBoolProfile1 = RainbowBool;
            }else if(Profile == 2){
                RainbowBoolProfile2 = RainbowBool;
            }else if(Profile == 3){
                RainbowBoolProfile3 = RainbowBool;
            }else if(Profile == 4){
                RainbowBoolProfile4 = RainbowBool;
            }else if(Profile == 5){
                RainbowBoolProfile5 = RainbowBool;
            }else if(Profile == 6){
                RainbowBoolProfile6 = RainbowBool;
            }
            if(!Toggle) {
                Picker->set(LastColor);
            }
            SaveConfig();
        });

        Spinner* Saturation = Spinner::CreateEx("S: ", "%", RainbowSaturation, 1,
        [](Spinner* Self, double Value) {
            RainbowSaturation = (Value > 100) ? 100 : (Value < 0) ? 0 : Value;
            Self->set(RainbowSaturation, 0);
            if(Profile == 1){
                RainbowSaturationProfile1 = RainbowSaturation;
            }else if(Profile == 2){
                RainbowSaturationProfile2 = RainbowSaturation;
            }else if(Profile == 3){
                RainbowSaturationProfile3 = RainbowSaturation;
            }else if(Profile == 4){
                RainbowSaturationProfile4 = RainbowSaturation;
            }else if(Profile == 5){
                RainbowSaturationProfile5 = RainbowSaturation;
            }else if(Profile == 6){
                RainbowSaturationProfile6 = RainbowSaturation;
            }
            SaveConfig();
        });

        Spinner* Value = Spinner::CreateEx("V: ", "%", RainbowValue, 1,
        [](Spinner* Self, double Value) {
            RainbowValue = (Value > 100) ? 100 : (Value < 0) ? 0 : Value;
            Self->set(RainbowValue, 0);
            if(Profile == 1){
                RainbowValueProfile1 = RainbowValue;
            }else if(Profile == 2){
                RainbowValueProfile2 = RainbowValue;
            }else if(Profile == 3){
                RainbowValueProfile3 = RainbowValue;
            }else if(Profile == 4){
                RainbowValueProfile4 = RainbowValue;
            }else if(Profile == 5){
                RainbowValueProfile5 = RainbowValue;
            }else if(Profile == 6){
                RainbowValueProfile6 = RainbowValue;
            }
            SaveConfig();
        });

        Spinner* Speed = Spinner::CreateEx("Speed: ", "x", RainbowSpeed, 0.5,
        [](Spinner* Self, double Value) {
            RainbowSpeed = (Value > 16) ? 16 : (Value < 0.1) ? 0.1 : Value;
            Self->set(RainbowSpeed, 0);
            if(Profile == 1){
                RainbowSpeedProfile1 = RainbowSpeed;
            }else if(Profile == 2){
                RainbowSpeedProfile2 = RainbowSpeed;
            }else if(Profile == 3){
                RainbowSpeedProfile3 = RainbowSpeed;
            }else if(Profile == 4){
                RainbowSpeedProfile4 = RainbowSpeed;
            }else if(Profile == 5){
                RainbowSpeedProfile5 = RainbowSpeed;
            }else if(Profile == 6){
                RainbowSpeedProfile6 = RainbowSpeed;
            }
            SaveConfig();
        });

        Spinner* ProfileSpinner = Spinner::CreateEx("Profile: ", "x", Profile, 0.5,
        [](Spinner* Self, double Value) {
            Profile = (Value > 6) ? 1 : (Value < 1) ? 6 : Value;
            Self->set(Profile, 0);
            SaveConfig();
        });

        const char *infoText[] = {"TsudaKageyu", "nlohmann", "Absolute", "Creeper76", "Ikszyon", ModVersion.c_str(), nullptr};
        SelectionBox* Info = SelectionBox::Create("More Info");
        Info->setValues(infoText);

        Window->addElements({Picker,
                             HorizontalLayout::Create(RainbowCheckbox, Speed),
                             HorizontalLayout::Create(Saturation, Value),
                             ProfileSpinner,
                             Info});
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