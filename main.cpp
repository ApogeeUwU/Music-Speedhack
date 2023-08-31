#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <cocos2d.h>
#include <MinHook.h>
#include <gd.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <Commdlg.h>
#pragma comment(lib, "Shlwapi.lib")
using namespace cocos2d;
namespace SpeedhackAudio {
    void* channel;
    double speed;
    bool initialized = false;
    void* (__stdcall* setVolume)(void* t_channel, float volume);
    void* (__stdcall* setPitch)(void* t_channel, float pitch);
    void* __stdcall SetVolumeHook(void* t_channel, float volume) {
        channel = t_channel;
        if (speed != 1.0) setPitch(channel, speed);
        return setVolume(channel, volume);
    }
    void InitSpeedhackAudio() {
        MH_Initialize();
        setPitch = (decltype(setPitch))GetProcAddress(GetModuleHandleW(L"fmod.dll"), "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
        unsigned long volume = (unsigned long)GetProcAddress(GetModuleHandleW(L"fmod.dll"), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
        MH_CreateHook((void*)volume, SetVolumeHook, (void**)&setVolume);
        speed = 1.0;
        initialized = true;
    }
    inline void SetSpeedhackAudio(float pitch) {
        if (!initialized) InitSpeedhackAudio();
        if (channel == nullptr) return;
        speed = pitch;
        setPitch(channel, pitch);
    }
}
bool ASCII(const std::string& str) {
    for (char c : str) {
        if (c < 0 || c > 127) {
            return false;
        }
    }
    return true;
}
namespace amogus {
    CCLabelBMFont* selectmenuloop;
    char szFile[260];
    namespace MoreOptionsLayer {
        class opt {
        public:
            void SelectMenu(CCObject*) {
                static OPENFILENAMEA ofn;
                RtlZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = "MP3 Files\0*.mp3\0WAV Files\0*.wav\0OGG Files\0*.ogg\0FLAC Files\0*.flac\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrTitle = "Select an MP3 File";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if (GetOpenFileNameA(&ofn)) {
                    std::string filename = szFile;
                    std::string extension = filename.substr(filename.find_last_of(".") + 1);
                    transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                    PathStripPathA(szFile);
                    if (ASCII(filename) == true) {
                        if (extension == "mp3" || extension == "ogg" || extension == "wav" || extension == "flac") {
                            selectmenuloop->setString(szFile);
                            gd::GameSoundManager::sharedState()->playBackgroundMusic(true, szFile);
                            return;
                        }
                        else {
                            selectmenuloop->setString("supported only mp3 ogg wav flac");
                        }
                    }
                    else {
                        selectmenuloop->setString("only ascii, rename file");
                    }
                }
            }
        };
        inline int(__thiscall* addToggle)(void* self, const char* display, const char* key, const char* extraInfo);
    };
    CCLabelBMFont* Style = nullptr;
    CCLabelBMFont* slidertext = nullptr;
    gd::Slider* slider = nullptr;
    double sliderval = 1.00;
    std::string sliderstr = "OFF";
    class optionss : public CCLayer {
    public:
        void sliderchange(CCObject* pSender) {
            double valueV = slider->getValue();
            double transformedValue = valueV * 2;
            sliderval = transformedValue;
            std::ostringstream value;
            value << std::fixed << std::setprecision(2) << transformedValue;
            sliderstr = value.str();
            slidertext->setString(value.str().c_str());
            if (transformedValue <= 0.01) {
                CCDirector::sharedDirector()->setAnimationInterval(1 / 1024.0);
                SpeedhackAudio::SetSpeedhackAudio(1.0);
                slidertext->setString("OFF");
                sliderstr = "OFF";
            }
            else if (transformedValue > 0.99 && transformedValue < 1.01) {
                slidertext->setString("OFF");
            }
            else {
                CCDirector::sharedDirector()->setAnimationInterval(1 / 1024.0);
                SpeedhackAudio::SetSpeedhackAudio(transformedValue);
            }
        }

    };
    inline bool(__thiscall* musicinit)(CCLayer* self);
    bool __fastcall musich(CCLayer* self) {
        if (!musicinit(self)) return false;
        CCLayer* layer = (CCLayer*)self->getChildren()->objectAtIndex(0);
        CCMenu* menums = CCMenu::create();
        auto options = gd::CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_playMusicBtn_001.png"), layer, menu_selector(MoreOptionsLayer::opt::SelectMenu));
        menums->addChild(options);
        menums->setPosition({ 135, 77 });
        slidertext = CCLabelBMFont::create(sliderstr.c_str(), "bigFont.fnt");
        slidertext->setPosition({ 146, 107 });
        slidertext->setScale(0.55);
        selectmenuloop = CCLabelBMFont::create(szFile, "bigFont.fnt");
        selectmenuloop->setPosition({ 284, 138.5 });
        selectmenuloop->setZOrder(777);
        selectmenuloop->setScale(0.325);
        slider = gd::Slider::create(layer, menu_selector(optionss::sliderchange), 1.f);
        slider->setValue(sliderval / 2);
        slider->updateBar();
        slider->setScale(0.625);
        slider->setAnchorPoint({ 0.0, 0.5 });
        slider->setPosition({ 182.0, 64.5 });
        layer->addChild(slider), layer->addChild(selectmenuloop), layer->addChild(slidertext), layer->addChild(menums);
        return true;
    }
}
unsigned long __stdcall dll() {
    MH_Initialize();
    MH_CreateHook((void*)(gd::base + 0x1DD420), amogus::musich, reinterpret_cast<void**>(&amogus::musicinit));
    MH_EnableHook(0);
    return 0;
}
int __stdcall DllMain(HMODULE hModule, unsigned long dw, void* lpv) {
    switch (dw) {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SpeedhackAudio::InitSpeedhackAudio, hModule, 0, 0);
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)dll, hModule, 0, 0);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return 1;
}
