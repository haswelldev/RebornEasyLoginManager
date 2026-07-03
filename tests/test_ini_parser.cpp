#include <catch2/catch_test_macros.hpp>
#include "IniParser.h"

TEST_CASE("ParseIni - minimal valid INI with one account", "[ini]") {
    wxString content =
        "[L2REBORN_EASYLOGIN]\n"
        "1_id=myuser\n"
        "1_password=mypass\n"
        "1_description=My Desc\n";

    IniData data = ParseIni(content);
    REQUIRE(data.accounts.size() == 1);
    REQUIRE(data.accounts[0].id == "myuser");
    REQUIRE(data.accounts[0].password == "mypass");
    REQUIRE(data.accounts[0].description == "My Desc");
}

TEST_CASE("ParseIni - slots 1 and 3 filled, slot 2 empty", "[ini]") {
    wxString content =
        "[L2REBORN_EASYLOGIN]\n"
        "1_id=user1\n"
        "1_password=pass1\n"
        "1_description=\n"
        "3_id=user3\n"
        "3_password=pass3\n"
        "3_description=\n";

    IniData data = ParseIni(content);
    REQUIRE(data.accounts.size() == 2);
    REQUIRE(data.accounts[0].id == "user1");
    REQUIRE(data.accounts[1].id == "user3");
}

TEST_CASE("ParseIni - showOnStart and hideLogin flags parsed", "[ini]") {
    wxString content =
        "[L2REBORN_EASYLOGIN]\n"
        "showOnStart=1\n"
        "hideLogin=1\n";

    IniData data = ParseIni(content);
    REQUIRE(data.showOnStart == true);
    REQUIRE(data.hideLogin == true);
}

TEST_CASE("ParseIni - unknown lines preserved in otherLines", "[ini]") {
    wxString content =
        "[L2REBORN_EASYLOGIN]\n"
        "someUnknown=value\n";

    IniData data = ParseIni(content);
    REQUIRE(data.otherLines.GetCount() == 1);
    REQUIRE(data.otherLines[0] == "someUnknown=value");
}

TEST_CASE("ParseIni - empty content produces empty result without crash", "[ini]") {
    IniData data = ParseIni("");
    REQUIRE(data.accounts.empty());
    REQUIRE(data.otherLines.IsEmpty());
    REQUIRE(data.showOnStart == false);
    REQUIRE(data.hideLogin == false);
}

TEST_CASE("ParseIni/SerializeIni round-trip restores accounts", "[ini]") {
    std::vector<Account> accounts = {
        {"user1", "pass1", "desc1"},
        {"user2", "pass2", "desc2"}
    };

    wxString serialized = SerializeIni(accounts, wxArrayString(), false, false);
    IniData data = ParseIni(serialized);

    REQUIRE(data.accounts.size() == 2);
    REQUIRE(data.accounts[0].id == "user1");
    REQUIRE(data.accounts[0].password == "pass1");
    REQUIRE(data.accounts[0].description == "desc1");
    REQUIRE(data.accounts[1].id == "user2");
    REQUIRE(data.accounts[1].password == "pass2");
    REQUIRE(data.accounts[1].description == "desc2");
}

TEST_CASE("SerializeIni - 0 accounts produces 199 empty slots", "[ini]") {
    wxString content = SerializeIni({}, wxArrayString(), false, false);

    int count = 0;
    size_t pos = 0;
    while ((pos = content.find("_id=", pos)) != wxString::npos) {
        ++count;
        ++pos;
    }
    REQUIRE(count == 199);
}

TEST_CASE("SerializeIni - settings written after account slots", "[ini]") {
    wxString content = SerializeIni({}, wxArrayString(), true, false);

    REQUIRE(content.Contains("showOnStart=1"));
    REQUIRE(content.Contains("hideLogin=0"));

    size_t lastSlotPos  = content.rfind("199_description=");
    size_t showOnStartPos = content.find("showOnStart=");
    REQUIRE(showOnStartPos != wxString::npos);
    REQUIRE(lastSlotPos   != wxString::npos);
    REQUIRE(showOnStartPos > lastSlotPos);
}
