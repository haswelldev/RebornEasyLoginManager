#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>
#include <wx/timer.h>
#include <vector>
#include "Account.h"

class wxPopupWindow;

// Main Frame
class MainFrame : public wxFrame {
public:
    MainFrame();

private:
    wxPanel* m_mainPanel{};
    wxSearchCtrl* m_search{};
    wxListView* m_listView{};
    wxStaticText* m_emptyHint{};
    wxTextCtrl* m_editCtrl{};
    wxMenuBar* m_menuBar{};
    wxTimer m_statusTimer;
    long m_editingItem = -1;          // list row currently being inline-edited
    bool m_dragging = false;          // a drag-to-reorder is in progress
    long m_dragSourceAccount = -1;    // account index being dragged
    wxPopupWindow* m_dragGhost = nullptr;
    wxWindow* m_dropLine = nullptr;   // thin insertion indicator (child of the list)
    wxTimer m_dragScrollTimer;
    int m_dragScrollDir = 0;          // -1 up, +1 down, 0 none
    int m_rowHeightPx = 20;
    wxPoint m_lastDragPos;            // last pointer pos in list-event client coords
    wxString m_iniPath;
    std::vector<Account> m_accounts;  // the in-memory model (unfiltered)
    wxArrayString m_otherLines;       // lines that aren't account data, preserved verbatim

    bool m_showOnStart = false;
    bool m_hideLogin = false;
    bool m_dirty = false;
    wxChoice* m_choiceLang{};

    // Setup
    void SetupMenuBar();
    void BindMenuCommands();
    void SetupUI();

    // Model <-> view helpers
    bool IsFiltering() const;
    long SelectedAccountIndex() const;      // maps the selected row to an m_accounts index (-1 if none)
    void RefreshList();                     // rebuilds the (possibly filtered) list from m_accounts
    void SelectAccount(long accountIdx);    // selects the row backing the given account index
    void UpdateListViewItem(long accountIdx);
    void StartInlineEdit(long row);

    // Chrome / feedback
    void MarkDirty(bool dirty = true);
    void UpdateTitle();
    void UpdateStatusBar();
    void FlashStatus(const wxString& message);
    void UpdateEmptyHint();
    void UpdateControlsState();
    void OnStatusTimer(wxTimerEvent&);

    // File lifecycle
    void LoadConfig();
    void SaveConfig();
    void LoadFile(const wxString& path);
    void LoadFileWithPrompt(const wxString& path);
    bool DoSave(const wxString& path);
    bool SaveToCurrentOrPrompt();
    bool ConfirmDiscardChanges();

    // Command handlers (shared by menu items and toolbar buttons)
    void OnNew(wxCommandEvent&);
    void OnOpen(wxCommandEvent&);
    void OnSave(wxCommandEvent&);
    void OnSaveAs(wxCommandEvent&);
    void OnSettings(wxCommandEvent&);
    void OnAbout(wxCommandEvent&);
    void OnExit(wxCommandEvent&);
    void OnAddAccount(wxCommandEvent&);
    void OnEditAccount(wxCommandEvent&);
    void OnDuplicateAccount(wxCommandEvent&);
    void OnCopyPassword(wxCommandEvent&);
    void OnDeleteAccount(wxCommandEvent&);
    void OnMoveUp(wxCommandEvent&);
    void OnMoveDown(wxCommandEvent&);
    void OnMoveTop(wxCommandEvent&);
    void OnMoveBottom(wxCommandEvent&);
    void OnLanguageSelected(wxCommandEvent&);
    void RefreshLabels();

    // UI events
    void OnClose(wxCloseEvent&);
    void OnSearch(wxCommandEvent&);
    void OnListSelectionChanged(wxListEvent&);
    void OnListKeyDown(wxListEvent&);
    void OnListContextMenu(wxContextMenuEvent&);
    void OnLeftDClick(wxMouseEvent& event);
    void OnEditFinish(wxEvent& event);

    // Drag-to-reorder: a semi-transparent ghost of the row follows the cursor,
    // a drop indicator marks the target, and the list edge-auto-scrolls.
    void OnBeginDrag(wxListEvent& event);
    void OnDragMotion(wxMouseEvent& event);
    void OnDragEnd(wxMouseEvent& event);
    void OnDragScrollTimer(wxTimerEvent&);
    void EndDrag();
    void UpdateDragVisuals(const wxPoint& clientPos);
    void CreateDragGhost(long row);
    wxBitmap MakeRowBitmap(long row);
    void UpdateDropLine(long target);   // positions the insertion line, or hides it (wxNOT_FOUND)
    long DragTargetIndex(const wxPoint& pos) const;  // insertion index in [0, count]

    // Toolbar / footer buttons that need label refresh + enable/disable
    wxButton* m_btnOpen{};
    wxButton* m_btnSave{};
    wxButton* m_btnAdd{};
    wxButton* m_btnSettings{};
    wxButton* m_btnTop{};
    wxButton* m_btnUp{};
    wxButton* m_btnDown{};
    wxButton* m_btnBottom{};
    wxButton* m_btnEdit{};
    wxButton* m_btnDel{};
};

#endif // MAINFRAME_H
