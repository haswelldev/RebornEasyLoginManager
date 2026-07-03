#include <catch2/catch_test_macros.hpp>
#include "LanguageManager.h"

TEST_CASE("LanguageManager - valid JSON returns correct string", "[lang]") {
    wxString json = "{\"TITLE\": \"My App\", \"OK\": \"OK\"}";
    REQUIRE(LanguageManager::Get().LoadFromContent("en", json));
    REQUIRE(LanguageManager::Get().GetString("TITLE") == "My App");
    REQUIRE(LanguageManager::Get().GetString("OK") == "OK");
}

TEST_CASE("LanguageManager - unknown key returns key as fallback", "[lang]") {
    wxString json = "{\"TITLE\": \"My App\"}";
    LanguageManager::Get().LoadFromContent("en", json);
    REQUIRE(LanguageManager::Get().GetString("MISSING") == "MISSING");
}

TEST_CASE("LanguageManager - language switch updates strings and language code", "[lang]") {
    wxString enJson = "{\"HELLO\": \"Hello\"}";
    wxString frJson = "{\"HELLO\": \"Bonjour\"}";

    LanguageManager::Get().LoadFromContent("en", enJson);
    REQUIRE(LanguageManager::Get().GetString("HELLO") == "Hello");
    REQUIRE(LanguageManager::Get().GetCurrentLanguage() == "en");

    LanguageManager::Get().LoadFromContent("fr", frJson);
    REQUIRE(LanguageManager::Get().GetString("HELLO") == "Bonjour");
    REQUIRE(LanguageManager::Get().GetCurrentLanguage() == "fr");
}

TEST_CASE("LanguageManager - empty content returns false", "[lang]") {
    REQUIRE_FALSE(LanguageManager::Get().LoadFromContent("en", ""));
}

TEST_CASE("LanguageManager - malformed JSON with no quotes returns false", "[lang]") {
    REQUIRE_FALSE(LanguageManager::Get().LoadFromContent("en", "no quotes at all: just plain text"));
}
