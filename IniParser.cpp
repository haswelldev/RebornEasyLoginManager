#include "IniParser.h"
#include <wx/string.h>

IniData ParseIni(const wxString& content) {
    IniData data;

    if (content.IsEmpty()) {
        return data;
    }

    // Split content into lines, handling both \r\n and \n
    wxArrayString lines;
    wxString remaining = content;
    while (!remaining.IsEmpty()) {
        size_t pos = remaining.find('\n');
        wxString line;
        if (pos != wxString::npos) {
            line = remaining.Left(pos);
            remaining = remaining.Mid(pos + 1);
        } else {
            line = remaining;
            remaining.Clear();
        }
        // Strip trailing \r
        if (!line.IsEmpty() && line.Last() == '\r') {
            line = line.Left(line.length() - 1);
        }
        lines.Add(line);
    }

    // Temporary storage for slots 1-199
    struct RawAcc { wxString id, pass, desc; };
    std::vector<RawAcc> raw(200); // 1-indexed

    for (size_t i = 0; i < lines.GetCount(); ++i) {
        const wxString& line = lines[i];
        if (line.StartsWith("showOnStart=")) {
            long val;
            line.Mid(12).ToLong(&val);
            data.showOnStart = (val != 0);
        } else if (line.StartsWith("hideLogin=")) {
            long val;
            line.Mid(10).ToLong(&val);
            data.hideLogin = (val != 0);
        } else if (line.Contains("_id=") || line.Contains("_password=") || line.Contains("_description=")) {
            int underscorePos = line.Find('_');
            int equalPos = line.Find('=');
            if (underscorePos != wxNOT_FOUND && equalPos != wxNOT_FOUND) {
                long index;
                line.Left(underscorePos).ToLong(&index);
                wxString key = line.Mid(underscorePos + 1, equalPos - underscorePos - 1);
                wxString val = line.Mid(equalPos + 1);
                if (index >= 1 && index <= 199) {
                    if (key == "id") raw[index].id = val;
                    else if (key == "password") raw[index].pass = val;
                    else if (key == "description") raw[index].desc = val;
                }
            }
        } else if (!line.IsEmpty() && line != "[L2REBORN_EASYLOGIN]") {
            data.otherLines.Add(line);
        }
    }

    for (int i = 1; i <= 199; ++i) {
        if (!raw[i].id.IsEmpty()) {
            data.accounts.push_back({raw[i].id, raw[i].pass, raw[i].desc});
        }
    }

    return data;
}

wxString SerializeIni(const std::vector<Account>& accounts,
                      const wxArrayString& otherLines,
                      bool showOnStart, bool hideLogin) {
    wxString content;
    content += "[L2REBORN_EASYLOGIN]\n";

    for (int i = 1; i <= 199; ++i) {
        Account acc = {"", "", ""};
        if (i <= (int)accounts.size()) {
            acc = accounts[i - 1];
        }
        content += wxString::Format("%d_id=%s\n", i, acc.id);
        content += wxString::Format("%d_password=%s\n", i, acc.password);
        content += wxString::Format("%d_description=%s\n", i, acc.description);
    }

    for (size_t i = 0; i < otherLines.GetCount(); ++i) {
        content += otherLines[i] + "\n";
    }

    content += wxString::Format("showOnStart=%d\n", showOnStart ? 1 : 0);
    content += wxString::Format("hideLogin=%d\n", hideLogin ? 1 : 0);

    return content;
}
