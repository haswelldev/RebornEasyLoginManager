#include "SettingsDialog.h"
#include "LanguageManager.h"

SettingsDialog::SettingsDialog(wxWindow* parent, bool showOnStart, bool hideLogin)
    : wxDialog(parent, wxID_ANY, L("SETTINGS_TITLE"), wxDefaultPosition, wxDefaultSize) {

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* panel = new wxPanel(this);
    auto* panelSizer = new wxBoxSizer(wxVERTICAL);

    // These two options are directives for the game client, stored in the .ini
    // file; the manager itself doesn't act on them. Make that explicit so the
    // checkboxes don't look like they "do nothing".
    auto* intro = new wxStaticText(panel, wxID_ANY, L("SETTINGS_INTRO"));
    intro->Wrap(360);
    intro->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    panelSizer->Add(intro, 0, wxLEFT | wxRIGHT | wxTOP, 10);

    m_chkShowOnStart = new wxCheckBox(panel, wxID_ANY, L("SHOW_ON_START"));
    m_chkShowOnStart->SetValue(showOnStart);
    m_chkShowOnStart->SetToolTip(L("SHOW_ON_START_TIP"));
    panelSizer->Add(m_chkShowOnStart, 0, wxALL, 10);

    m_chkHideLogin = new wxCheckBox(panel, wxID_ANY, L("HIDE_LOGIN"));
    m_chkHideLogin->SetValue(hideLogin);
    m_chkHideLogin->SetToolTip(L("HIDE_LOGIN_TIP"));
    panelSizer->Add(m_chkHideLogin, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    panel->SetSizer(panelSizer);
    mainSizer->Add(panel, 1, wxEXPAND);

    mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);
}

bool SettingsDialog::GetShowOnStart() const {
    return m_chkShowOnStart->GetValue();
}

bool SettingsDialog::GetHideLogin() const {
    return m_chkHideLogin->GetValue();
}
