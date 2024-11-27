#include <codecvt>
#include "helpers.h"

static void
toml_myfree (void *p) {
    if (p) {
        const char *pp = static_cast<char *> (p);
        delete[] pp;
    }
}

toml_table_t *
openConfig (const std::filesystem::path &path) {
    if (!exists (path) || !path.has_filename ()) {
        LogMessage (LogLevel::WARN, std::string (path.string ()) + ": file does not exist");
        return nullptr;
    }

    std::ifstream stream (path);
    if (!stream.is_open ()) {
        LogMessage (LogLevel::WARN, ("Could not open " + std::string (path.string ())).c_str ());
        return nullptr;
    }

    stream.seekg (0, stream.end);
    const size_t length = stream.tellg ();
    stream.seekg (0, stream.beg);

    const auto buf = static_cast<char *> (calloc (length + 1, sizeof (char)));
    stream.read (buf, length);

    char errorBuffer[200];
    toml_table_t *config = toml_parse (buf, errorBuffer, 200);
    stream.close ();
    free (buf);

    if (!config) {
        LogMessage (LogLevel::WARN, (path.string () + ": " + errorBuffer).c_str ());
        return nullptr;
    }

    return config;
}

toml_table_t *
openConfigSection (const toml_table_t *config, const std::string &sectionName) {
    toml_table_t *section = toml_table_in (config, sectionName.c_str ());
    if (!section) {
        LogMessage (LogLevel::ERROR, ("Cannot find section " + sectionName).c_str ());
        return nullptr;
    }

    return section;
}

bool
readConfigBool (const toml_table_t *table, const std::string &key, const bool notFoundValue) {
    const auto [ok, u] = toml_bool_in (table, key.c_str ());
    if (!ok) {
        LogMessage (LogLevel::WARN, ("Could not find Boolean named " + key).c_str ());
        return notFoundValue;
    }
    return static_cast<bool> (u.b);
}

i64
readConfigInt (const toml_table_t *table, const std::string &key, const i64 notFoundValue) {
    const auto [ok, u] = toml_int_in (table, key.c_str ());
    if (!ok) {
        LogMessage (LogLevel::WARN, ("Could not find Int named " + key).c_str ());
        return notFoundValue;
    }
    return u.i;
}

std::string
readConfigString (const toml_table_t *table, const std::string &key, const std::string &notFoundValue) {
    const auto [ok, u] = toml_string_in (table, key.c_str ());
    if (!ok) {
        LogMessage (LogLevel::WARN, ("Could not find String named " + key).c_str ());
        return notFoundValue;
    }
    std::string str = u.s;
    toml_myfree (u.s);
    return str;
}

std::vector<i64>
readConfigIntArray (const toml_table_t *table, const std::string &key, std::vector<i64> notFoundValue) {
    const toml_array_t *array = toml_array_in (table, key.c_str ());
    if (!array) {
        LogMessage (LogLevel::WARN, ("Could not find int Array named " + key).c_str ());
        return notFoundValue;
    }

    std::vector<i64> ret;
    for (int i = 0;; i++) {
        auto [ok, u] = toml_int_at (array, i);
        if (!ok) break;
        ret.push_back (u.i);
    }

    return ret;
}

std::wstring
replace (const std::wstring &orignStr, const std::wstring &oldStr, const std::wstring &newStr) {
    size_t pos                              = 0;
    std::wstring tempStr                    = orignStr;
    const std::wstring::size_type newStrLen = newStr.length ();
    const std::wstring::size_type oldStrLen = oldStr.length ();
    while (true) {
        pos = tempStr.find (oldStr, pos);
        if (pos == std::wstring::npos) break;

        tempStr.replace (pos, oldStrLen, newStr);
        pos += newStrLen;
    }

    return tempStr;
}

std::string
replace (const std::string &orignStr, const std::string &oldStr, const std::string &newStr) {
    size_t pos                             = 0;
    std::string tempStr                    = orignStr;
    const std::string::size_type newStrLen = newStr.length ();
    const std::string::size_type oldStrLen = oldStr.length ();
    while (true) {
        pos = tempStr.find (oldStr, pos);
        if (pos == std::string::npos) break;

        tempStr.replace (pos, oldStrLen, newStr);
        pos += newStrLen;
    }

    return tempStr;
}

const char *
GameVersionToString (const GameVersion version) {
    switch (version) {
    case GameVersion::JPN00: return "JPN00";
    case GameVersion::JPN08: return "JPN08";
    case GameVersion::JPN39: return "JPN39";
    case GameVersion::CHN00: return "CHN00";
    default: return "UNKNOWN";
    }
}

const char *
languageStr (const int language) {
    switch (language) {
    case 1: return "en_us";
    case 2: return "cn_tw";
    case 3: return "kor";
    case 4: return "cn_cn";
    default: return "jpn";
    }
}

std::string
ConvertWideToUtf8 (const std::wstring &wstr) {
    if (wstr.empty ()) return {};

    // Determine the size of the resulting UTF-8 string
    const int utf8Size = WideCharToMultiByte (CP_UTF8, 0, wstr.c_str (), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Size <= 0) {
        LogMessage (LogLevel::ERROR, "Failed to convert wide string to UTF-8");
        return {};
    }

    // Allocate buffer and perform the conversion
    std::string utf8Str (utf8Size, '\0'); // -1 to exclude null terminator
    WideCharToMultiByte (CP_UTF8, 0, wstr.c_str (), -1, data (utf8Str), utf8Size, nullptr, nullptr);

    return utf8Str;
}

bool
AreAllBytesZero (const u8 *array, const size_t offset, const size_t length) {
    for (size_t i = 0; i < length; ++i)
        if (array[offset + i] != 0x00) return false;
    return true;
}