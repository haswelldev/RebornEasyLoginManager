#include "MainFrame.h"
#include "AccountDialog.h"
#include "SettingsDialog.h"
#include "LanguageManager.h"
#include "Languages.h"
#include "IniParser.h"
#include <wx/config.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/textfile.h>
#include <wx/icon.h>
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/dnd.h>
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/popupwin.h>
#include <wx/dcmemory.h>
#include <wx/settings.h>

// Custom command ids for actions without a suitable stock id.
enum {
    ID_COPY_PASSWORD = wxID_HIGHEST + 1,
    ID_MOVE_TOP,
    ID_MOVE_BOTTOM,
    ID_STATUS_TIMER,
    ID_DRAG_SCROLL_TIMER,
};

// Drop target: dropping an .ini file onto the list opens it (with an
// unsaved-changes prompt, same as File > Open).
class IniFileDropTarget : public wxFileDropTarget {
public:
    explicit IniFileDropTarget(std::function<void(const wxString&)> onDrop)
        : m_onDrop(std::move(onDrop)) {}

    bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames) override {
        for (const auto& name : filenames) {
            if (name.Lower().EndsWith(".ini")) {
                m_onDrop(name);
                return true;
            }
        }
        return false;
    }

private:
    std::function<void(const wxString&)> m_onDrop;
};

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, L("TITLE"), wxDefaultPosition, wxSize(760, 540)) {
#ifdef __WXMSW__
    SetIcon(wxIcon("IDI_ICON1", wxBITMAP_TYPE_ICO_RESOURCE));
#endif
    m_statusTimer.SetOwner(this, ID_STATUS_TIMER);
    Bind(wxEVT_TIMER, &MainFrame::OnStatusTimer, this, ID_STATUS_TIMER);
    m_dragScrollTimer.SetOwner(this, ID_DRAG_SCROLL_TIMER);
    Bind(wxEVT_TIMER, &MainFrame::OnDragScrollTimer, this, ID_DRAG_SCROLL_TIMER);
    Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

    SetupMenuBar();
    BindMenuCommands();
    SetupUI();

    CreateStatusBar(2);
    int widths[2] = {-1, 200};
    GetStatusBar()->SetStatusWidths(2, widths);

    LoadConfig();
    UpdateTitle();
    UpdateStatusBar();
    UpdateControlsState();
}

void MainFrame::SetupMenuBar() {
    m_menuBar = new wxMenuBar();

    auto* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, L("NEW_FILE") + "\tCtrl+N");
    fileMenu->Append(wxID_OPEN, L("OPEN_FILE") + "\tCtrl+O");
    fileMenu->Append(wxID_SAVE, L("SAVE") + "\tCtrl+S");
    fileMenu->Append(wxID_SAVEAS, L("SAVE_AS") + "\tCtrl+Shift+S");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_PREFERENCES, L("MANAGER_SETTINGS"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, L("EXIT") + "\tCtrl+Q");

    auto* editMenu = new wxMenu();
    editMenu->Append(wxID_ADD, L("ADD_ACCOUNT") + "\tCtrl+Shift+A");
    editMenu->Append(wxID_EDIT, L("EDIT") + "\tCtrl+E");
    editMenu->Append(wxID_DUPLICATE, L("DUPLICATE") + "\tCtrl+D");
    editMenu->Append(ID_COPY_PASSWORD, L("COPY_PASSWORD") + "\tCtrl+Shift+C");
    editMenu->AppendSeparator();
    editMenu->Append(ID_MOVE_TOP, L("MOVE_TOP") + "\tCtrl+Shift+Home");
    editMenu->Append(wxID_UP, L("MOVE_UP") + "\tCtrl+Shift+Up");
    editMenu->Append(wxID_DOWN, L("MOVE_DOWN") + "\tCtrl+Shift+Down");
    editMenu->Append(ID_MOVE_BOTTOM, L("MOVE_BOTTOM") + "\tCtrl+Shift+End");
    editMenu->AppendSeparator();
    editMenu->Append(wxID_DELETE, L("DELETE"));

    auto* helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, L("ABOUT"));

    m_menuBar->Append(fileMenu, L("MENU_FILE"));
    m_menuBar->Append(editMenu, L("MENU_EDIT"));
    m_menuBar->Append(helpMenu, L("MENU_HELP"));

    // SetMenuBar detaches but does not delete a previously attached menu bar,
    // so free it ourselves when rebuilding (e.g. on language switch).
    wxMenuBar* old = GetMenuBar();
    SetMenuBar(m_menuBar);
    delete old;
}

void MainFrame::BindMenuCommands() {
    Bind(wxEVT_MENU, &MainFrame::OnNew, this, wxID_NEW);
    Bind(wxEVT_MENU, &MainFrame::OnOpen, this, wxID_OPEN);
    Bind(wxEVT_MENU, &MainFrame::OnSave, this, wxID_SAVE);
    Bind(wxEVT_MENU, &MainFrame::OnSaveAs, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &MainFrame::OnSettings, this, wxID_PREFERENCES);
    Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &MainFrame::OnAddAccount, this, wxID_ADD);
    Bind(wxEVT_MENU, &MainFrame::OnEditAccount, this, wxID_EDIT);
    Bind(wxEVT_MENU, &MainFrame::OnDuplicateAccount, this, wxID_DUPLICATE);
    Bind(wxEVT_MENU, &MainFrame::OnCopyPassword, this, ID_COPY_PASSWORD);
    Bind(wxEVT_MENU, &MainFrame::OnMoveTop, this, ID_MOVE_TOP);
    Bind(wxEVT_MENU, &MainFrame::OnMoveUp, this, wxID_UP);
    Bind(wxEVT_MENU, &MainFrame::OnMoveDown, this, wxID_DOWN);
    Bind(wxEVT_MENU, &MainFrame::OnMoveBottom, this, ID_MOVE_BOTTOM);
    Bind(wxEVT_MENU, &MainFrame::OnDeleteAccount, this, wxID_DELETE);
    Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
}

void MainFrame::SetupUI() {
    m_mainPanel = new wxPanel(this);
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Toolbar (Buttons)
    auto* toolbarSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnOpen = new wxButton(m_mainPanel, wxID_OPEN, L("OPEN_FILE"));
    m_btnSave = new wxButton(m_mainPanel, wxID_SAVE, L("SAVE"));
    m_btnAdd = new wxButton(m_mainPanel, wxID_ADD, L("ADD_ACCOUNT"));
    m_btnSettings = new wxButton(m_mainPanel, wxID_PREFERENCES, L("MANAGER_SETTINGS"));

#ifdef __WXMSW__
    m_btnOpen->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_BUTTON));
    m_btnSave->SetBitmap(wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_BUTTON));
    m_btnAdd->SetBitmap(wxArtProvider::GetBitmap(wxART_NEW, wxART_BUTTON));
    m_btnSettings->SetBitmap(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_BUTTON));
#endif

    toolbarSizer->Add(m_btnOpen, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
    toolbarSizer->Add(m_btnSave, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
    toolbarSizer->Add(m_btnAdd, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
    toolbarSizer->Add(m_btnSettings, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

    toolbarSizer->AddStretchSpacer();

    // Language picker, populated from the shared language table.
    wxArrayString choices;
    for (const auto& lang : AvailableLanguages()) {
        choices.Add(wxString(lang.display));
    }
    m_choiceLang = new wxChoice(m_mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
    m_choiceLang->SetSelection(LanguageIndexForCode(LanguageManager::Get().GetCurrentLanguage()));
    m_choiceLang->SetMinSize(wxSize(110, -1));
    toolbarSizer->Add(m_choiceLang, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 5);

    mainSizer->Add(toolbarSizer, 0, wxEXPAND | wxTOP, 5);

    // Search / filter box
    m_search = new wxSearchCtrl(m_mainPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_search->ShowSearchButton(true);
    m_search->ShowCancelButton(true);
    m_search->SetDescriptiveText(L("SEARCH_HINT"));
    mainSizer->Add(m_search, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // List
    m_listView = new wxListView(m_mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
#ifdef __WXMSW__
    m_listView->SetWindowStyleFlag(m_listView->GetWindowStyleFlag() | wxLC_HRULES | wxLC_VRULES);
#endif
    m_listView->AppendColumn(L("ACCOUNT_ID"), wxLIST_FORMAT_LEFT, 180);
    m_listView->AppendColumn(L("DESCRIPTION"), wxLIST_FORMAT_LEFT, 250);

    mainSizer->Add(m_listView, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // Empty-state hint, centered over the (empty) list.
    m_emptyHint = new wxStaticText(m_listView, wxID_ANY, L("EMPTY_HINT"),
                                   wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
    m_emptyHint->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));

    // Thin insertion line shown between rows during a drag-to-reorder.
    m_dropLine = new wxWindow(m_listView, wxID_ANY, wxDefaultPosition, wxSize(-1, 3));
    m_dropLine->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    m_dropLine->Hide();

    m_listView->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        int width = event.GetSize().GetWidth();
        int col0Width = m_listView->GetColumnWidth(0);
        if (width > col0Width + 20) {
            m_listView->SetColumnWidth(1, width - col0Width - 20);
        }
        // Re-center the empty hint.
        if (m_emptyHint && m_emptyHint->IsShown()) {
            wxSize client = m_listView->GetClientSize();
            wxSize hint = m_emptyHint->GetBestSize();
            m_emptyHint->SetSize((client.x - hint.x) / 2, (client.y - hint.y) / 2, hint.x, hint.y);
        }
        event.Skip();
    });

    // Footer buttons
    auto* footerSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnTop = new wxButton(m_mainPanel, ID_MOVE_TOP, L("MOVE_TOP"));
    m_btnUp = new wxButton(m_mainPanel, wxID_UP, L("MOVE_UP"));
    m_btnDown = new wxButton(m_mainPanel, wxID_DOWN, L("MOVE_DOWN"));
    m_btnBottom = new wxButton(m_mainPanel, ID_MOVE_BOTTOM, L("MOVE_BOTTOM"));
    m_btnEdit = new wxButton(m_mainPanel, wxID_EDIT, L("EDIT"));
    m_btnDel = new wxButton(m_mainPanel, wxID_DELETE, L("DELETE"));

#ifdef __WXMSW__
    m_btnTop->SetBitmap(wxArtProvider::GetBitmap(wxART_GOTO_FIRST, wxART_BUTTON));
    m_btnUp->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_UP, wxART_BUTTON));
    m_btnDown->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_BUTTON));
    m_btnBottom->SetBitmap(wxArtProvider::GetBitmap(wxART_GOTO_LAST, wxART_BUTTON));
    m_btnEdit->SetBitmap(wxArtProvider::GetBitmap(wxART_EDIT, wxART_BUTTON));
    m_btnDel->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_BUTTON));
#endif

    footerSizer->Add(m_btnTop, 0, wxALL, 5);
    footerSizer->Add(m_btnUp, 0, wxALL, 5);
    footerSizer->Add(m_btnDown, 0, wxALL, 5);
    footerSizer->Add(m_btnBottom, 0, wxALL, 5);
    footerSizer->AddStretchSpacer();
    footerSizer->Add(m_btnEdit, 0, wxALL, 5);
    footerSizer->Add(m_btnDel, 0, wxALL, 5);
    mainSizer->Add(footerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    m_mainPanel->SetSizer(mainSizer);

    // Button bindings (route to the same handlers as the menu items)
    m_btnOpen->Bind(wxEVT_BUTTON, &MainFrame::OnOpen, this);
    m_btnSave->Bind(wxEVT_BUTTON, &MainFrame::OnSave, this);
    m_btnAdd->Bind(wxEVT_BUTTON, &MainFrame::OnAddAccount, this);
    m_btnSettings->Bind(wxEVT_BUTTON, &MainFrame::OnSettings, this);
    m_btnTop->Bind(wxEVT_BUTTON, &MainFrame::OnMoveTop, this);
    m_btnUp->Bind(wxEVT_BUTTON, &MainFrame::OnMoveUp, this);
    m_btnDown->Bind(wxEVT_BUTTON, &MainFrame::OnMoveDown, this);
    m_btnBottom->Bind(wxEVT_BUTTON, &MainFrame::OnMoveBottom, this);
    m_btnEdit->Bind(wxEVT_BUTTON, &MainFrame::OnEditAccount, this);
    m_btnDel->Bind(wxEVT_BUTTON, &MainFrame::OnDeleteAccount, this);
    m_choiceLang->Bind(wxEVT_CHOICE, &MainFrame::OnLanguageSelected, this);

    // Search
    m_search->Bind(wxEVT_TEXT, &MainFrame::OnSearch, this);
    m_search->Bind(wxEVT_SEARCH, &MainFrame::OnSearch, this);
    m_search->Bind(wxEVT_SEARCH_CANCEL, &MainFrame::OnSearch, this);

    // List interactions
    m_listView->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::OnListSelectionChanged, this);
    m_listView->Bind(wxEVT_LIST_ITEM_DESELECTED, &MainFrame::OnListSelectionChanged, this);
    m_listView->Bind(wxEVT_LIST_KEY_DOWN, &MainFrame::OnListKeyDown, this);
    m_listView->Bind(wxEVT_CONTEXT_MENU, &MainFrame::OnListContextMenu, this);
    m_listView->Bind(wxEVT_LEFT_DCLICK, &MainFrame::OnLeftDClick, this);

    // Drag-to-reorder rows with the mouse.
    m_listView->Bind(wxEVT_LIST_BEGIN_DRAG, &MainFrame::OnBeginDrag, this);
    m_listView->Bind(wxEVT_MOTION, &MainFrame::OnDragMotion, this);
    m_listView->Bind(wxEVT_LEFT_UP, &MainFrame::OnDragEnd, this);
    m_listView->Bind(wxEVT_MOUSE_CAPTURE_LOST, [this](wxMouseCaptureLostEvent&) { EndDrag(); });

    // Drag & drop an .ini onto the list to open it.
    m_listView->SetDropTarget(new IniFileDropTarget([this](const wxString& path) {
        LoadFileWithPrompt(path);
    }));

    // Inline description editor (floating over the list).
    m_editCtrl = new wxTextCtrl(m_listView, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_SIMPLE);
    m_editCtrl->Hide();
    m_editCtrl->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnEditFinish, this);
    m_editCtrl->Bind(wxEVT_KILL_FOCUS, &MainFrame::OnEditFinish, this);
    m_editCtrl->Bind(wxEVT_CHAR, [this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_ESCAPE) {
            m_editCtrl->Hide();
            m_editingItem = -1;
        } else {
            event.Skip();
        }
    });
}

// ---------------------------------------------------------------------------
// Model <-> view helpers
// ---------------------------------------------------------------------------

bool MainFrame::IsFiltering() const {
    return m_search && !m_search->GetValue().IsEmpty();
}

long MainFrame::SelectedAccountIndex() const {
    long sel = m_listView->GetFirstSelected();
    if (sel == wxNOT_FOUND) return -1;
    return (long)m_listView->GetItemData(sel);
}

void MainFrame::RefreshList() {
    m_listView->DeleteAllItems();

    wxString filter = IsFiltering() ? m_search->GetValue().Lower() : wxString();
    for (size_t i = 0; i < m_accounts.size(); ++i) {
        const Account& acc = m_accounts[i];
        if (!filter.IsEmpty() &&
            !acc.id.Lower().Contains(filter) &&
            !acc.description.Lower().Contains(filter)) {
            continue;
        }
        long row = m_listView->InsertItem(m_listView->GetItemCount(), acc.id);
        m_listView->SetItem(row, 1, acc.description);
        m_listView->SetItemData(row, (wxUIntPtr)i);
    }

    UpdateEmptyHint();
    UpdateStatusBar();
    UpdateControlsState();
}

void MainFrame::SelectAccount(long accountIdx) {
    for (int row = 0; row < m_listView->GetItemCount(); ++row) {
        if ((long)m_listView->GetItemData(row) == accountIdx) {
            m_listView->Select(row);
            m_listView->Focus(row);
            m_listView->EnsureVisible(row);
            return;
        }
    }
}

void MainFrame::UpdateListViewItem(long accountIdx) {
    for (int row = 0; row < m_listView->GetItemCount(); ++row) {
        if ((long)m_listView->GetItemData(row) == accountIdx) {
            m_listView->SetItem(row, 0, m_accounts[accountIdx].id);
            m_listView->SetItem(row, 1, m_accounts[accountIdx].description);
            return;
        }
    }
}

void MainFrame::StartInlineEdit(long row) {
    if (row < 0 || row >= m_listView->GetItemCount()) return;
    wxRect rect;
    m_listView->GetSubItemRect(row, 1, rect);
    m_editingItem = row;
    m_editCtrl->SetSize(rect);
    m_editCtrl->SetValue(m_listView->GetItemText(row, 1));
    m_editCtrl->Show();
    m_editCtrl->SetFocus();
    m_editCtrl->SelectAll();
}

// ---------------------------------------------------------------------------
// Chrome / feedback
// ---------------------------------------------------------------------------

void MainFrame::MarkDirty(bool dirty) {
    if (m_dirty == dirty) return;
    m_dirty = dirty;
    UpdateTitle();
}

void MainFrame::UpdateTitle() {
    // Decode the em-dash explicitly as UTF-8: a narrow " — " literal is
    // interpreted in the local ANSI codepage on Windows, which mangles it.
    const wxString sep = wxString::FromUTF8(" \xE2\x80\x94 ");
    wxString name = m_iniPath.IsEmpty() ? L("TITLE")
                                        : wxFileNameFromPath(m_iniPath) + sep + L("TITLE");
    SetTitle((m_dirty ? "*" : "") + name);
}

void MainFrame::UpdateStatusBar() {
    if (!GetStatusBar()) return;
    SetStatusText(m_iniPath.IsEmpty() ? L("STATUS_NO_FILE") : m_iniPath, 0);
    SetStatusText(wxString::Format(L("STATUS_ACCOUNTS"), (int)m_accounts.size()), 1);
}

void MainFrame::FlashStatus(const wxString& message) {
    if (!GetStatusBar()) return;
    SetStatusText(message, 0);
    m_statusTimer.Start(2500, wxTIMER_ONE_SHOT);
}

void MainFrame::OnStatusTimer(wxTimerEvent&) {
    UpdateStatusBar();
}

void MainFrame::UpdateEmptyHint() {
    if (!m_emptyHint) return;
    bool show = m_accounts.empty();
    if (show) {
        m_emptyHint->SetLabel(m_iniPath.IsEmpty() ? L("EMPTY_HINT") : L("EMPTY_HINT_NO_ACCOUNTS"));
        wxSize client = m_listView->GetClientSize();
        wxSize hint = m_emptyHint->GetBestSize();
        m_emptyHint->SetSize((client.x - hint.x) / 2, (client.y - hint.y) / 2, hint.x, hint.y);
    }
    m_emptyHint->Show(show);
}

void MainFrame::UpdateControlsState() {
    long accIdx = SelectedAccountIndex();
    bool hasSel = accIdx != -1;
    bool canMoveUp = hasSel && !IsFiltering() && accIdx > 0;
    bool canMoveDown = hasSel && !IsFiltering() && accIdx < (long)m_accounts.size() - 1;
    bool canAdd = m_accounts.size() < 199;

    m_btnAdd->Enable(canAdd);
    m_btnEdit->Enable(hasSel);
    m_btnDel->Enable(hasSel);
    m_btnTop->Enable(canMoveUp);
    m_btnUp->Enable(canMoveUp);
    m_btnDown->Enable(canMoveDown);
    m_btnBottom->Enable(canMoveDown);

    if (m_menuBar) {
        m_menuBar->Enable(wxID_ADD, canAdd);
        m_menuBar->Enable(wxID_EDIT, hasSel);
        m_menuBar->Enable(wxID_DUPLICATE, hasSel && canAdd);
        m_menuBar->Enable(ID_COPY_PASSWORD, hasSel);
        m_menuBar->Enable(wxID_DELETE, hasSel);
        m_menuBar->Enable(ID_MOVE_TOP, canMoveUp);
        m_menuBar->Enable(wxID_UP, canMoveUp);
        m_menuBar->Enable(wxID_DOWN, canMoveDown);
        m_menuBar->Enable(ID_MOVE_BOTTOM, canMoveDown);
    }
}

// ---------------------------------------------------------------------------
// File lifecycle
// ---------------------------------------------------------------------------

void MainFrame::LoadConfig() {
    wxConfigBase* config = wxConfig::Get();
    wxString path;
    if (config->Read("/LastIniPath", &path) && wxFileExists(path)) {
        LoadFile(path);
    }
}

void MainFrame::SaveConfig() {
    wxConfigBase* config = wxConfig::Get();
    config->Write("/LastIniPath", m_iniPath);
    config->Flush();
}

void MainFrame::LoadFile(const wxString& path) {
    wxFile file(path);
    if (!file.IsOpened()) return;

    wxString content;
    file.ReadAll(&content);

    IniData data = ParseIni(content);
    m_accounts = std::move(data.accounts);
    m_otherLines = data.otherLines;
    m_showOnStart = data.showOnStart;
    m_hideLogin = data.hideLogin;

    m_iniPath = path;
    if (m_search) m_search->ChangeValue("");
    MarkDirty(false);
    RefreshList();
    UpdateTitle();
    SaveConfig();
}

void MainFrame::LoadFileWithPrompt(const wxString& path) {
    if (!ConfirmDiscardChanges()) return;
    LoadFile(path);
}

bool MainFrame::DoSave(const wxString& path) {
    wxString content = SerializeIni(m_accounts, m_otherLines, m_showOnStart, m_hideLogin);

    // Write via wxTextFile so the file keeps the platform's native line
    // endings, which is what the game client historically produced.
    wxTextFile file;
    if (wxFileExists(path)) {
        if (!file.Open(path)) { wxMessageBox(L("SAVE_FAILED"), L("SAVE_FAILED_TITLE"), wxOK | wxICON_ERROR); return false; }
        file.Clear();
    } else if (!file.Create(path)) {
        wxMessageBox(L("SAVE_FAILED"), L("SAVE_FAILED_TITLE"), wxOK | wxICON_ERROR);
        return false;
    }

    wxArrayString lines = wxSplit(content, '\n', '\0');
    if (!lines.IsEmpty() && lines.Last().IsEmpty()) lines.RemoveAt(lines.GetCount() - 1);
    for (const auto& line : lines) file.AddLine(line);

    if (!file.Write()) {
        wxMessageBox(L("SAVE_FAILED"), L("SAVE_FAILED_TITLE"), wxOK | wxICON_ERROR);
        return false;
    }

    m_iniPath = path;
    MarkDirty(false);
    UpdateTitle();
    UpdateStatusBar();
    SaveConfig();
    FlashStatus(L("SAVE_SUCCESS"));
    return true;
}

bool MainFrame::SaveToCurrentOrPrompt() {
    wxString path = m_iniPath;
    if (path.IsEmpty()) {
        wxFileDialog dialog(this, L("SAVE_INI_DIALOG_TITLE"), "", "",
                            L("INI_FILES_WILDCARD"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dialog.ShowModal() == wxID_CANCEL) return false;
        path = dialog.GetPath();
    }
    return DoSave(path);
}

bool MainFrame::ConfirmDiscardChanges() {
    if (!m_dirty) return true;
    int answer = wxMessageBox(L("UNSAVED_MSG"), L("UNSAVED_TITLE"),
                              wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
    if (answer == wxYES) return SaveToCurrentOrPrompt();
    if (answer == wxNO) return true;
    return false; // Cancel
}

// ---------------------------------------------------------------------------
// Command handlers
// ---------------------------------------------------------------------------

void MainFrame::OnNew(wxCommandEvent&) {
    if (!ConfirmDiscardChanges()) return;
    m_accounts.clear();
    m_otherLines.Clear();
    m_showOnStart = false;
    m_hideLogin = false;
    m_iniPath.Clear();
    if (m_search) m_search->ChangeValue("");
    MarkDirty(false);
    RefreshList();
    UpdateTitle();
}

void MainFrame::OnOpen(wxCommandEvent&) {
    if (!ConfirmDiscardChanges()) return;
    wxFileDialog openFileDialog(this, L("OPEN_INI_DIALOG_TITLE"), "", "",
                                L("INI_FILES_WILDCARD"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) return;
    LoadFile(openFileDialog.GetPath());
}

void MainFrame::OnSave(wxCommandEvent&) {
    SaveToCurrentOrPrompt();
}

void MainFrame::OnSaveAs(wxCommandEvent&) {
    wxFileDialog dialog(this, L("SAVE_INI_DIALOG_TITLE"), "", "",
                        L("INI_FILES_WILDCARD"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_CANCEL) return;
    DoSave(dialog.GetPath());
}

void MainFrame::OnSettings(wxCommandEvent&) {
    SettingsDialog dialog(this, m_showOnStart, m_hideLogin);
    if (dialog.ShowModal() == wxID_OK) {
        bool newShow = dialog.GetShowOnStart();
        bool newHide = dialog.GetHideLogin();
        if (newShow != m_showOnStart || newHide != m_hideLogin) {
            m_showOnStart = newShow;
            m_hideLogin = newHide;
            MarkDirty();
        }
    }
}

void MainFrame::OnAbout(wxCommandEvent&) {
    wxAboutDialogInfo info;
    info.SetName(L("TITLE"));
    info.SetVersion("1.1.0");
    info.SetDescription(L("ABOUT_TEXT"));
    wxAboutBox(info, this);
}

void MainFrame::OnExit(wxCommandEvent&) {
    Close(false);
}

void MainFrame::OnAddAccount(wxCommandEvent&) {
    if (m_accounts.size() >= 199) {
        wxMessageBox(L("LIMIT_REACHED_MESSAGE"), L("LIMIT_REACHED_TITLE"), wxOK | wxICON_WARNING);
        return;
    }
    AccountDialog dialog(this, L("ADD_ACCOUNT_TITLE"), Account{}, &m_accounts);
    if (dialog.ShowModal() == wxID_OK) {
        m_accounts.push_back(dialog.GetAccount());
        if (m_search) m_search->ChangeValue(""); // ensure the new row is visible
        MarkDirty();
        RefreshList();
        SelectAccount((long)m_accounts.size() - 1);
    }
}

void MainFrame::OnEditAccount(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx == -1) return;

    AccountDialog dialog(this, L("EDIT_ACCOUNT_TITLE"), m_accounts[idx], &m_accounts, idx);
    if (dialog.ShowModal() == wxID_OK) {
        m_accounts[idx] = dialog.GetAccount();
        MarkDirty();
        RefreshList();
        SelectAccount(idx);
    }
}

void MainFrame::OnDuplicateAccount(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx == -1) return;
    if (m_accounts.size() >= 199) {
        wxMessageBox(L("LIMIT_REACHED_MESSAGE"), L("LIMIT_REACHED_TITLE"), wxOK | wxICON_WARNING);
        return;
    }
    Account copy = m_accounts[idx];
    copy.description = (copy.description + " " + L("COPY_SUFFIX")).Trim(false).Trim(true);
    m_accounts.insert(m_accounts.begin() + idx + 1, copy);
    if (m_search) m_search->ChangeValue("");
    MarkDirty();
    RefreshList();
    SelectAccount(idx + 1);
}

void MainFrame::OnCopyPassword(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx == -1) return;
    const wxString& pass = m_accounts[idx].password;
    if (pass.IsEmpty()) {
        FlashStatus(L("NO_PASSWORD"));
        return;
    }
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(pass));
        wxTheClipboard->Close();
        FlashStatus(L("PASSWORD_COPIED"));
    }
}

void MainFrame::OnDeleteAccount(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx == -1) return;

    if (wxMessageBox(L("CONFIRM_DELETE_MSG"), L("CONFIRM_DELETE_TITLE"),
                     wxYES_NO | wxICON_QUESTION, this) != wxYES) {
        return;
    }

    m_accounts.erase(m_accounts.begin() + idx);
    MarkDirty();
    RefreshList();
    if (!m_accounts.empty()) {
        SelectAccount(std::min(idx, (long)m_accounts.size() - 1));
    }
}

void MainFrame::OnMoveUp(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx <= 0 || IsFiltering()) return;
    std::swap(m_accounts[idx], m_accounts[idx - 1]);
    MarkDirty();
    RefreshList();
    SelectAccount(idx - 1);
}

void MainFrame::OnMoveDown(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx == -1 || IsFiltering() || idx >= (long)m_accounts.size() - 1) return;
    std::swap(m_accounts[idx], m_accounts[idx + 1]);
    MarkDirty();
    RefreshList();
    SelectAccount(idx + 1);
}

void MainFrame::OnMoveTop(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx <= 0 || IsFiltering()) return;
    Account acc = m_accounts[idx];
    m_accounts.erase(m_accounts.begin() + idx);
    m_accounts.insert(m_accounts.begin(), acc);
    MarkDirty();
    RefreshList();
    SelectAccount(0);
}

void MainFrame::OnMoveBottom(wxCommandEvent&) {
    long idx = SelectedAccountIndex();
    if (idx == -1 || IsFiltering() || idx >= (long)m_accounts.size() - 1) return;
    Account acc = m_accounts[idx];
    m_accounts.erase(m_accounts.begin() + idx);
    m_accounts.push_back(acc);
    MarkDirty();
    RefreshList();
    SelectAccount((long)m_accounts.size() - 1);
}

void MainFrame::OnLanguageSelected(wxCommandEvent&) {
    wxString langCode = LanguageCodeForIndex(m_choiceLang->GetSelection());
    if (LanguageManager::Get().LoadLanguage(langCode)) {
        wxConfig::Get()->Write("/Language", langCode);
        wxConfig::Get()->Flush();
        RefreshLabels();
    }
}

void MainFrame::RefreshLabels() {
    UpdateTitle();
    m_btnOpen->SetLabel(L("OPEN_FILE"));
    m_btnSave->SetLabel(L("SAVE"));
    m_btnAdd->SetLabel(L("ADD_ACCOUNT"));
    m_btnSettings->SetLabel(L("MANAGER_SETTINGS"));
    m_btnTop->SetLabel(L("MOVE_TOP"));
    m_btnUp->SetLabel(L("MOVE_UP"));
    m_btnDown->SetLabel(L("MOVE_DOWN"));
    m_btnBottom->SetLabel(L("MOVE_BOTTOM"));
    m_btnEdit->SetLabel(L("EDIT"));
    m_btnDel->SetLabel(L("DELETE"));
    m_search->SetDescriptiveText(L("SEARCH_HINT"));

    wxListItem col0;
    m_listView->GetColumn(0, col0);
    col0.SetText(L("ACCOUNT_ID"));
    m_listView->SetColumn(0, col0);

    wxListItem col1;
    m_listView->GetColumn(1, col1);
    col1.SetText(L("DESCRIPTION"));
    m_listView->SetColumn(1, col1);

    // Rebuild the menu bar so its labels follow the new language.
    SetupMenuBar();

    UpdateEmptyHint();
    UpdateStatusBar();
    UpdateControlsState();
    m_mainPanel->Layout();
}

// ---------------------------------------------------------------------------
// UI events
// ---------------------------------------------------------------------------

void MainFrame::OnClose(wxCloseEvent& event) {
    if (event.CanVeto() && !ConfirmDiscardChanges()) {
        event.Veto();
        return;
    }
    Destroy();
}

void MainFrame::OnSearch(wxCommandEvent&) {
    RefreshList();
}

void MainFrame::OnListSelectionChanged(wxListEvent&) {
    UpdateControlsState();
}

void MainFrame::OnListKeyDown(wxListEvent& event) {
    wxCommandEvent dummy;
    switch (event.GetKeyCode()) {
        case WXK_DELETE:
        case WXK_BACK:
            OnDeleteAccount(dummy);
            break;
        case WXK_F2: {
            long sel = m_listView->GetFirstSelected();
            if (sel != wxNOT_FOUND) StartInlineEdit(sel);
            break;
        }
        case WXK_INSERT:
            OnAddAccount(dummy);
            break;
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            OnEditAccount(dummy);
            break;
        default:
            event.Skip();
            break;
    }
}

void MainFrame::OnListContextMenu(wxContextMenuEvent& event) {
    wxPoint pos = event.GetPosition();
    // Keyboard-triggered context menus report (-1,-1); anchor to the selection.
    if (pos == wxPoint(-1, -1)) {
        long sel = m_listView->GetFirstSelected();
        wxRect rect;
        if (sel != wxNOT_FOUND && m_listView->GetItemRect(sel, rect)) {
            pos = m_listView->ClientToScreen(rect.GetBottomLeft());
        } else {
            pos = m_listView->ClientToScreen(wxPoint(0, 0));
        }
    } else {
        int flags = 0;
        long item = m_listView->HitTest(m_listView->ScreenToClient(pos), flags);
        if (item != wxNOT_FOUND) m_listView->Select(item);
    }

    if (SelectedAccountIndex() == -1) return;

    wxMenu menu;
    menu.Append(wxID_EDIT, L("EDIT"));
    menu.Append(wxID_DUPLICATE, L("DUPLICATE"));
    menu.Append(ID_COPY_PASSWORD, L("COPY_PASSWORD"));
    menu.AppendSeparator();
    menu.Append(wxID_DELETE, L("DELETE"));
    PopupMenu(&menu, m_listView->ScreenToClient(pos));
}

void MainFrame::OnLeftDClick(wxMouseEvent& event) {
    int flags = 0;
    long item = m_listView->HitTest(event.GetPosition(), flags);
    if (item == wxNOT_FOUND) {
        event.Skip();
        return;
    }

    // Double-clicking the description column starts inline editing; double-
    // clicking elsewhere opens the full edit dialog.
    wxRect rect;
    m_listView->GetSubItemRect(item, 1, rect);
    if (event.GetPosition().x >= rect.x && event.GetPosition().x <= rect.x + rect.width) {
        StartInlineEdit(item);
    } else {
        m_listView->Select(item);
        wxCommandEvent dummy;
        OnEditAccount(dummy);
    }
}

void MainFrame::OnEditFinish(wxEvent& event) {
    if (m_editingItem != -1 && m_editCtrl->IsShown()) {
        long accIdx = (long)m_listView->GetItemData(m_editingItem);
        wxString newVal = m_editCtrl->GetValue();
        if (accIdx >= 0 && accIdx < (long)m_accounts.size() &&
            m_accounts[accIdx].description != newVal) {
            m_accounts[accIdx].description = newVal;
            m_listView->SetItem(m_editingItem, 1, newVal);
            MarkDirty();
        }
        m_editCtrl->Hide();
        m_editingItem = -1;
    }
    event.Skip();
}

// ---------------------------------------------------------------------------
// Drag-to-reorder (ghost image + drop indicator + edge auto-scroll)
// ---------------------------------------------------------------------------

// Insertion index the pointer maps to, in the range [0, count]: index i means
// "the item will land before row i" (i == count means append at the end). The
// row's vertical midpoint decides between inserting above vs. below it, so the
// drop matches the line the user sees. Not filtering during a drag, so account
// index == row index.
long MainFrame::DragTargetIndex(const wxPoint& pos) const {
    int count = m_listView->GetItemCount();
    if (count == 0) return wxNOT_FOUND;

    int flags = 0;
    long row = m_listView->HitTest(pos, flags);
    if (row != wxNOT_FOUND) {
        wxRect r;
        m_listView->GetItemRect(row, r);
        return (pos.y >= r.GetTop() + r.height / 2) ? row + 1 : row;
    }

    wxRect first, last;
    m_listView->GetItemRect(0, first);
    m_listView->GetItemRect(count - 1, last);
    if (pos.y >= last.GetBottom()) return count;   // append
    if (pos.y <= first.GetTop()) return 0;
    return wxNOT_FOUND;
}

void MainFrame::UpdateDropLine(long target) {
    if (!m_dropLine) return;

    int count = m_listView->GetItemCount();
    if (target == wxNOT_FOUND || count == 0) {
        m_dropLine->Hide();
        return;
    }

    int y;
    if (target >= count) {
        wxRect last;
        m_listView->GetItemRect(count - 1, last);
        y = last.GetBottom();          // append: line below the last row
    } else {
        wxRect r;
        m_listView->GetItemRect(target, r);
        y = r.GetTop();                // insert before target: line above that row
    }

    int w = m_listView->GetClientSize().x;
    m_dropLine->SetSize(0, y - 1, w, 3);
    m_dropLine->Show();
    m_dropLine->Raise();
}

wxBitmap MainFrame::MakeRowBitmap(long row) {
    wxRect rr;
    m_listView->GetItemRect(row, rr);
    int w = m_listView->GetClientSize().x;
    int h = rr.height > 0 ? rr.height : m_rowHeightPx;
    if (w < 40) w = 200;

    wxBitmap bmp(w, h);
    wxMemoryDC dc(bmp);
    wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    dc.SetBackground(wxBrush(bg));
    dc.Clear();
    dc.SetPen(wxPen(bg.ChangeLightness(70)));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, w, h);
    dc.SetFont(m_listView->GetFont());
    dc.SetTextForeground(fg);
    int ty = (h - dc.GetCharHeight()) / 2;
    dc.DrawText(m_listView->GetItemText(row, 0), 6, ty);
    int col0 = m_listView->GetColumnWidth(0);
    dc.DrawText(m_listView->GetItemText(row, 1), col0 + 6, ty);
    dc.SelectObject(wxNullBitmap);
    return bmp;
}

void MainFrame::CreateDragGhost(long row) {
    wxBitmap bmp = MakeRowBitmap(row);
    auto* ghost = new wxPopupWindow(this, wxBORDER_NONE);
    ghost->SetSize(bmp.GetWidth(), bmp.GetHeight());
    ghost->Bind(wxEVT_PAINT, [ghost, bmp](wxPaintEvent&) {
        wxPaintDC dc(ghost);
        dc.DrawBitmap(bmp, 0, 0, true);
    });
    ghost->SetTransparent(170);
    ghost->Show();
    m_dragGhost = ghost;
}

void MainFrame::UpdateDragVisuals(const wxPoint& clientPos) {
    if (!m_dragging) return;

    // Ghost position is screen-based; the drop indicator must use the same
    // client coordinates as the mouse events (which match GetItemRect).
    wxPoint screen = wxGetMousePosition();
    if (m_dragGhost) m_dragGhost->Move(screen.x + 12, screen.y + 10);

    UpdateDropLine(DragTargetIndex(clientPos));
}

void MainFrame::EndDrag() {
    if (!m_dragging && !m_dragGhost) return;
    m_dragging = false;
    m_dragSourceAccount = -1;
    m_dragScrollDir = 0;
    if (m_dragScrollTimer.IsRunning()) m_dragScrollTimer.Stop();
    if (m_dragGhost) { m_dragGhost->Destroy(); m_dragGhost = nullptr; }
    if (m_dropLine) m_dropLine->Hide();
    m_listView->SetCursor(wxNullCursor);
}

void MainFrame::OnBeginDrag(wxListEvent& event) {
    // Reordering isn't meaningful while the list is filtered.
    if (IsFiltering()) return;

    long row = event.GetIndex();
    if (row == wxNOT_FOUND) return;

    // No explicit CaptureMouse(): the list is already tracking the button
    // drag, so motion/up events are delivered until the button is released.
    m_dragging = true;
    m_dragSourceAccount = (long)m_listView->GetItemData(row);

    wxRect rr;
    m_listView->GetItemRect(row, rr);
    if (rr.height > 0) m_rowHeightPx = rr.height;

    m_listView->SetCursor(wxCursor(wxCURSOR_HAND));
    CreateDragGhost(row);
    m_lastDragPos = event.GetPoint();
    UpdateDragVisuals(m_lastDragPos);
}

void MainFrame::OnDragMotion(wxMouseEvent& event) {
    if (!m_dragging) {
        event.Skip();
        return;
    }

    m_lastDragPos = event.GetPosition();
    UpdateDragVisuals(m_lastDragPos);

    // Edge auto-scroll: run a timer while the pointer sits near the top/bottom.
    const int margin = 24;
    int y = event.GetPosition().y;
    int height = m_listView->GetClientSize().y;
    if (y < margin)                 m_dragScrollDir = -1;
    else if (y > height - margin)   m_dragScrollDir = +1;
    else                            m_dragScrollDir = 0;

    if (m_dragScrollDir != 0) {
        if (!m_dragScrollTimer.IsRunning()) m_dragScrollTimer.Start(60);
    } else {
        m_dragScrollTimer.Stop();
    }
}

void MainFrame::OnDragScrollTimer(wxTimerEvent&) {
    if (!m_dragging || m_dragScrollDir == 0) {
        m_dragScrollTimer.Stop();
        return;
    }
    m_listView->ScrollList(0, m_dragScrollDir * m_rowHeightPx);
    UpdateDragVisuals(m_lastDragPos);
}

void MainFrame::OnDragEnd(wxMouseEvent& event) {
    if (!m_dragging) {
        event.Skip();
        return;
    }

    long src = m_dragSourceAccount;
    long ins = DragTargetIndex(event.GetPosition());   // insertion index in [0, n]
    int n = (int)m_accounts.size();

    EndDrag();

    if (src < 0 || src >= n || ins == wxNOT_FOUND) return;

    Account acc = m_accounts[src];
    m_accounts.erase(m_accounts.begin() + src);

    // Removing src shifts everything after it down by one.
    long insertPos = (ins > src) ? ins - 1 : ins;

    m_accounts.insert(m_accounts.begin() + insertPos, acc);
    MarkDirty();
    RefreshList();
    SelectAccount(insertPos);
}
