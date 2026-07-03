#include <wx/wx.h>
#include "MainFrame.h"
#include "LanguageManager.h"
#include "Languages.h"
#include <wx/intl.h>
#include <wx/config.h>

class MyApp : public wxApp {
public:
    bool OnInit() override {
        // Set app/vendor identity so every wxConfig::Get() call shares one
        // backing store (registry key on Windows, plist on macOS).
        SetVendorName("L2Reborn");
        SetAppName("EasyLoginManager");

        wxConfigBase* config = wxConfig::Get();
        wxString langCode = config->Read("/Language", "");

        if (langCode.IsEmpty()) {
            // Fall back to the system language if we ship a matching translation.
            const wxLanguageInfo* sysInfo = wxLocale::GetLanguageInfo(wxLocale::GetSystemLanguage());
            if (sysInfo) {
                wxString sysCode = sysInfo->CanonicalName.BeforeFirst('_');
                for (const auto& lang : AvailableLanguages()) {
                    if (sysCode == lang.code) { langCode = lang.code; break; }
                }
            }
            if (langCode.IsEmpty()) langCode = "en";
        }

        LanguageManager::Get().LoadLanguage(langCode);

        auto* frame = new MainFrame();
        frame->Show(true);
        frame->Raise();
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
