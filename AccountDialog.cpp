#include "AccountDialog.h"
#include "LanguageManager.h"
#include <wx/artprov.h>

AccountDialog::AccountDialog(wxWindow* parent, const wxString& title, const Account& acc,
                             const std::vector<Account>* existing, long editingIndex)
    : wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize),
      m_existing(existing), m_editingIndex(editingIndex) {

    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* panel = new wxPanel(this);
    auto* panelSizer = new wxBoxSizer(wxVERTICAL);

    auto gridSizer = new wxFlexGridSizer(2, 5, 5);
    gridSizer->AddGrowableCol(1, 1);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, L("ACCOUNT_ID_LABEL")), 0, wxALIGN_CENTER_VERTICAL);
    m_idCtrl = new wxTextCtrl(panel, wxID_ANY, acc.id);
    gridSizer->Add(m_idCtrl, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, L("PASSWORD_LABEL")), 0, wxALIGN_CENTER_VERTICAL);
    auto* passSizer = new wxBoxSizer(wxHORIZONTAL);
    m_passCtrl = new wxTextCtrl(panel, wxID_ANY, acc.password, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    m_revealBtn = new wxButton(panel, wxID_ANY, L("REVEAL"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
#ifdef __WXMSW__
    m_revealBtn->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON));
#endif
    passSizer->Add(m_passCtrl, 1, wxEXPAND);
    passSizer->Add(m_revealBtn, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
    gridSizer->Add(passSizer, 1, wxEXPAND);

    gridSizer->Add(new wxStaticText(panel, wxID_ANY, L("DESCRIPTION_LABEL")), 0, wxALIGN_CENTER_VERTICAL);
    m_descCtrl = new wxTextCtrl(panel, wxID_ANY, acc.description);
    gridSizer->Add(m_descCtrl, 1, wxEXPAND);

    panelSizer->Add(gridSizer, 1, wxEXPAND | wxALL, 10);
    panel->SetSizer(panelSizer);

    mainSizer->Add(panel, 1, wxEXPAND);
    mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALIGN_RIGHT | wxALL, 10);

    SetSizerAndFit(mainSizer);
    SetMinSize(wxSize(360, -1));
    Fit();

    m_idCtrl->SetFocus();

    m_revealBtn->Bind(wxEVT_BUTTON, &AccountDialog::OnToggleReveal, this);
    Bind(wxEVT_BUTTON, &AccountDialog::OnOk, this, wxID_OK);
}

void AccountDialog::OnOk(wxCommandEvent& event) {
    wxString id = m_idCtrl->GetValue();
    id.Trim(false).Trim(true);

    if (id.IsEmpty()) {
        wxMessageBox(L("EMPTY_ID_MSG"), L("VALIDATION_TITLE"), wxOK | wxICON_WARNING, this);
        m_idCtrl->SetFocus();
        return;
    }

    if (m_existing) {
        for (size_t i = 0; i < m_existing->size(); ++i) {
            if ((long)i == m_editingIndex) continue;
            if ((*m_existing)[i].id == id) {
                if (wxMessageBox(L("DUPLICATE_ID_MSG"), L("VALIDATION_TITLE"),
                                 wxYES_NO | wxICON_WARNING, this) != wxYES) {
                    m_idCtrl->SetFocus();
                    return;
                }
                break;
            }
        }
    }

    event.Skip(); // let the default handler close the dialog with wxID_OK
}

void AccountDialog::OnToggleReveal(wxCommandEvent&) {
    m_revealed = !m_revealed;
    m_revealBtn->SetLabel(m_revealed ? L("HIDE") : L("REVEAL"));
    wxString val = m_passCtrl->GetValue();

    auto* sizer = m_passCtrl->GetContainingSizer();

    // Find the item in the sizer to get its flags and proportion
    wxSizerItem* item = nullptr;
    for (size_t i = 0; i < sizer->GetItemCount(); ++i) {
        if (sizer->GetItem(i)->GetWindow() == m_passCtrl) {
            item = sizer->GetItem(i);
            break;
        }
    }

    int flags = item ? item->GetFlag() : wxEXPAND;
    int proportion = item ? item->GetProportion() : 1;

    // wxWidgets can't toggle wxTE_PASSWORD at runtime, so the control is
    // recreated with the desired style. (Caret position is not preserved.)
    wxWindowID id = m_passCtrl->GetId();
    m_passCtrl->Destroy();

    long newStyle = m_revealed ? 0 : wxTE_PASSWORD;
    m_passCtrl = new wxTextCtrl(m_revealBtn->GetParent(), id, val, wxDefaultPosition, wxDefaultSize, newStyle);

    sizer->Prepend(m_passCtrl, proportion, flags);
    sizer->Layout();
}

Account AccountDialog::GetAccount() const {
    Account acc = {m_idCtrl->GetValue(), m_passCtrl->GetValue(), m_descCtrl->GetValue()};
    acc.id.Trim(false).Trim(true);
    return acc;
}
