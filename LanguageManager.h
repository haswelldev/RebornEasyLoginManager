#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <wx/string.h>
#include <map>

class LanguageManager {
public:
    static LanguageManager& Get();
    
    bool LoadLanguage(const wxString& langCode);
    // Parses translation JSON that has already been loaded into memory.
    // Kept separate from LoadLanguage so it can be unit-tested without any
    // filesystem or Windows-resource access.
    bool LoadFromContent(const wxString& langCode, const wxString& content);
    wxString GetString(const wxString& key) const;
    wxString GetCurrentLanguage() const { return m_currentLang; }
    
private:
    LanguageManager() = default;
    std::map<wxString, wxString> m_strings;
    wxString m_currentLang;
};

// Global helper for translations
inline wxString L(const wxString& key) {
    return LanguageManager::Get().GetString(key);
}

#endif // LANGUAGEMANAGER_H
