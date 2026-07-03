#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include <wx/wx.h>
#include <vector>
#include "Account.h"

// Dialog for Adding/Editing an Account
class AccountDialog : public wxDialog {
public:
    // `existing` (optional) is the current account list, used to validate the
    // entered ID against duplicates. `editingIndex` is the index being edited
    // (-1 when adding) so an account isn't flagged as a duplicate of itself.
    AccountDialog(wxWindow* parent, const wxString& title,
                  const Account& acc = Account{},
                  const std::vector<Account>* existing = nullptr,
                  long editingIndex = -1);

    Account GetAccount() const;

private:
    void OnToggleReveal(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);

    wxTextCtrl *m_idCtrl, *m_passCtrl, *m_descCtrl;
    wxButton* m_revealBtn;
    bool m_revealed = false;

    const std::vector<Account>* m_existing;
    long m_editingIndex;
};

#endif // ACCOUNTDIALOG_H
