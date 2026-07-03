#ifndef ICONS_H
#define ICONS_H

#include <wx/bmpbndl.h>
#include <wx/settings.h>
#include <wx/string.h>

// Toolbar/button icons, drawn from the Lucide icon set (ISC license) and
// rendered from SVG so they stay crisp at any DPI. Colour is taken from the
// system text colour at creation time, so the icons match light and dark mode.
namespace icons {

static const char* const kOpen =
    "<path d=\"m6 14 1.5-2.9A2 2 0 0 1 9.24 10H20a2 2 0 0 1 1.94 2.5l-1.54 6a2 2 0 0 1-1.95 1.5H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h3.9a2 2 0 0 1 1.69.9l.81 1.2a2 2 0 0 0 1.67.9H18a2 2 0 0 1 2 2v2\"/>";
static const char* const kSave =
    "<path d=\"M15.2 3a2 2 0 0 1 1.4.6l3.8 3.8a2 2 0 0 1 .6 1.4V19a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2z\"/>"
    "<path d=\"M17 21v-7a1 1 0 0 0-1-1H8a1 1 0 0 0-1 1v7\"/><path d=\"M7 3v4a1 1 0 0 0 1 1h7\"/>";
static const char* const kAdd =
    "<path d=\"M2 21a8 8 0 0 1 13.292-6\"/><circle cx=\"10\" cy=\"8\" r=\"5\"/>"
    "<path d=\"M19 16v6\"/><path d=\"M22 19h-6\"/>";
static const char* const kSettings =
    "<path d=\"M12.22 2h-.44a2 2 0 0 0-2 2v.18a2 2 0 0 1-1 1.73l-.43.25a2 2 0 0 1-2 0l-.15-.08a2 2 0 0 0-2.73.73l-.22.38a2 2 0 0 0 .73 2.73l.15.1a2 2 0 0 1 1 1.72v.51a2 2 0 0 1-1 1.74l-.15.09a2 2 0 0 0-.73 2.73l.22.38a2 2 0 0 0 2.73.73l.15-.08a2 2 0 0 1 2 0l.43.25a2 2 0 0 1 1 1.73V20a2 2 0 0 0 2 2h.44a2 2 0 0 0 2-2v-.18a2 2 0 0 1 1-1.73l.43-.25a2 2 0 0 1 2 0l.15.08a2 2 0 0 0 2.73-.73l.22-.39a2 2 0 0 0-.73-2.73l-.15-.08a2 2 0 0 1-1-1.74v-.5a2 2 0 0 1 1-1.74l.15-.09a2 2 0 0 0 .73-2.73l-.22-.38a2 2 0 0 0-2.73-.73l-.15.08a2 2 0 0 1-2 0l-.43-.25a2 2 0 0 1-1-1.73V4a2 2 0 0 0-2-2z\"/>"
    "<circle cx=\"12\" cy=\"12\" r=\"3\"/>";
static const char* const kTop =
    "<path d=\"M5 3h14\"/><path d=\"m18 13-6-6-6 6\"/><path d=\"M12 7v14\"/>";
static const char* const kUp =
    "<path d=\"m5 12 7-7 7 7\"/><path d=\"M12 19V5\"/>";
static const char* const kDown =
    "<path d=\"M12 5v14\"/><path d=\"m19 12-7 7-7-7\"/>";
static const char* const kBottom =
    "<path d=\"M12 17V3\"/><path d=\"m6 11 6 6 6-6\"/><path d=\"M19 21H5\"/>";
static const char* const kEdit =
    "<path d=\"M12 3H5a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7\"/>"
    "<path d=\"M18.4 2.6a2 2 0 0 1 2.8 2.8L12 14.6 8 15.6l1-4z\"/>";
static const char* const kDelete =
    "<path d=\"M3 6h18\"/><path d=\"M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2\"/>"
    "<line x1=\"10\" x2=\"10\" y1=\"11\" y2=\"17\"/><line x1=\"14\" x2=\"14\" y1=\"11\" y2=\"17\"/>";

inline wxBitmapBundle Make(const char* paths, int size = 16) {
    wxColour c = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxString svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\" "
        "fill=\"none\" stroke=\"" + c.GetAsString(wxC2S_HTML_SYNTAX) +
        "\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\">" +
        wxString::FromUTF8(paths) + "</svg>";
    const wxScopedCharBuffer utf8 = svg.utf8_str();
    return wxBitmapBundle::FromSVG(utf8.data(), wxSize(size, size));
}

} // namespace icons

#endif // ICONS_H
