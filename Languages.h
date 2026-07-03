#ifndef LANGUAGES_H
#define LANGUAGES_H

#include <wx/string.h>
#include <vector>

// Single source of truth for the languages the app ships with.
// Adding a language here (plus its i18n/<code>.json and resources.rc entry)
// is enough for the picker, the selection mapping and startup detection to
// all pick it up. displayName is the language's own endonym, so it stays the
// same regardless of the currently active UI language.
struct LanguageEntry {
    const char* code;         // ISO code, matches i18n/<code>.json
    const wchar_t* display;   // endonym shown in the language picker
};

inline const std::vector<LanguageEntry>& AvailableLanguages() {
    static const std::vector<LanguageEntry> langs = {
        {"en", L"English"},
        {"fr", L"Français"},
        {"el", L"Ελληνικά"},
        {"pt", L"Português"},
        {"pl", L"Polski"},
        {"os", L"Ирон"},
        {"zh", L"中文"},
    };
    return langs;
}

// Index of a language code in AvailableLanguages(), or 0 (English) if unknown.
inline int LanguageIndexForCode(const wxString& code) {
    const auto& langs = AvailableLanguages();
    for (size_t i = 0; i < langs.size(); ++i) {
        if (code == langs[i].code) return (int)i;
    }
    return 0;
}

inline wxString LanguageCodeForIndex(int index) {
    const auto& langs = AvailableLanguages();
    if (index >= 0 && index < (int)langs.size()) {
        return wxString::FromAscii(langs[index].code);
    }
    return "en";
}

#endif // LANGUAGES_H
