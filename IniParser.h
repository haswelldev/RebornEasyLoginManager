#pragma once
#include <vector>
#include <wx/arrstr.h>
#include <wx/string.h>
#include "Account.h"

struct IniData {
    std::vector<Account> accounts;
    wxArrayString otherLines;
    bool showOnStart = false;
    bool hideLogin = false;
};

IniData ParseIni(const wxString& content);
wxString SerializeIni(const std::vector<Account>& accounts,
                      const wxArrayString& otherLines,
                      bool showOnStart, bool hideLogin);
