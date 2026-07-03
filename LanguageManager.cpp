#include "LanguageManager.h"
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/log.h>

#ifdef __WINDOWS__
#include <wx/msw/private.h>
#endif

// For simple JSON parsing without external dependencies, 
// since we know the structure is simple.

LanguageManager& LanguageManager::Get() {
    static LanguageManager instance;
    return instance;
}

bool LanguageManager::LoadLanguage(const wxString& langCode) {
    wxString content;
    bool loaded = false;

#ifdef __WINDOWS__
    // Try to load from resources on Windows
    wxString resName = "ID_I18N_" + langCode.Upper();
    HRSRC hRes = FindResource(wxGetInstance(), resName.t_str(), RT_RCDATA);
    if (hRes) {
        HGLOBAL hData = LoadResource(wxGetInstance(), hRes);
        if (hData) {
            DWORD size = SizeofResource(wxGetInstance(), hRes);
            const char* ptr = (const char*)LockResource(hData);
            if (ptr && size > 0) {
                content = wxString::FromUTF8(ptr, size);
                loaded = !content.IsEmpty();
            }
        }
    }
#endif

    if (!loaded) {
        wxString exePath = wxStandardPaths::Get().GetExecutablePath();
        wxFileName fn(exePath);
        wxString resPath = fn.GetPath();
        
#ifdef __WXOSX__
        // On macOS, resources are in Contents/Resources
        if (resPath.Contains(".app/Contents/MacOS")) {
            resPath = resPath.BeforeLast('/') + "/Resources";
        }
#endif

        wxString filePath = resPath + "/i18n/" + langCode + ".json";
        
        // Fallback for development (if i18n is in the current directory)
        if (!wxFile::Exists(filePath)) {
            filePath = "i18n/" + langCode + ".json";
        }
        
        if (wxFile::Exists(filePath)) {
            wxFile file(filePath);
            file.ReadAll(&content);
            loaded = !content.IsEmpty();
        }
    }

    if (!loaded) {
        wxLogError("Language file not found or empty for: %s", langCode);
        return false;
    }

    return LoadFromContent(langCode, content);
}

bool LanguageManager::LoadFromContent(const wxString& langCode, const wxString& content) {
    if (content.IsEmpty()) {
        return false;
    }

    std::map<wxString, wxString> parsed;

    // Very basic JSON parser for "key": "value" pairs
    size_t pos = 0;
    while ((pos = content.find('"', pos)) != wxString::npos) {
        size_t keyStart = pos + 1;
        size_t keyEnd = content.find('"', keyStart);
        if (keyEnd == wxString::npos) break;

        wxString key = content.Mid(keyStart, keyEnd - keyStart);

        pos = content.find(':', keyEnd);
        if (pos == wxString::npos) break;

        pos = content.find('"', pos);
        if (pos == wxString::npos) break;

        size_t valStart = pos + 1;
        size_t valEnd = content.find('"', valStart);
        if (valEnd == wxString::npos) break;

        wxString val = content.Mid(valStart, valEnd - valStart);
        parsed[key] = val;

        pos = valEnd + 1;
    }

    if (parsed.empty()) {
        return false;
    }

    m_strings = std::move(parsed);
    m_currentLang = langCode;
    return true;
}

wxString LanguageManager::GetString(const wxString& key) const {
    auto it = m_strings.find(key);
    if (it != m_strings.end()) {
        return it->second;
    }
    return key;
}
