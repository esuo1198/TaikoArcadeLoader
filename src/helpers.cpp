#include "helpers.h"
#include <windows.h>

static void
toml_myfree (void *p) {
    if (p) {
        char *pp = (char *)p;
        delete[] pp;
    }
}

toml_table_t *
openConfig (std::filesystem::path path) {
    if (!std::filesystem::exists (path) || !path.has_filename ()) {
        LogMessage (LOG_LEVEL_WARN, (std::string (path.string ()) + ": file does not exist").c_str ());
        return 0;
    }

    std::ifstream stream (path);
    if (!stream.is_open ()) {
        LogMessage (LOG_LEVEL_WARN, ("Could not open " + std::string (path.string ())).c_str ());
        return 0;
    }

    stream.seekg (0, stream.end);
    size_t length = stream.tellg ();
    stream.seekg (0, stream.beg);

    char *buf = (char *)calloc (length + 1, sizeof (char));
    stream.read (buf, length);

    char errorbuf[200];
    toml_table_t *config = toml_parse (buf, errorbuf, 200);
    stream.close ();
    free (buf);

    if (!config) {
        LogMessage (LOG_LEVEL_WARN, (path.string () + ": " + errorbuf).c_str ());
        return 0;
    }

    return config;
}

toml_table_t *
openConfigSection (toml_table_t *config, const std::string &sectionName) {
    toml_table_t *section = toml_table_in (config, sectionName.c_str ());
    if (!section) {
        LogMessage (LOG_LEVEL_ERROR, ("Cannot find section " + sectionName).c_str ());
        return 0;
    }

    return section;
}

bool
readConfigBool (toml_table_t *table, const std::string &key, bool notFoundValue) {
    toml_datum_t data = toml_bool_in (table, key.c_str ());
    if (!data.ok) {
        LogMessage (LOG_LEVEL_WARN, ("Could not find Boolean named " + key).c_str ());
        return notFoundValue;
    }
    return (bool)data.u.b;
}

int64_t
readConfigInt (toml_table_t *table, const std::string &key, int64_t notFoundValue) {
    toml_datum_t data = toml_int_in (table, key.c_str ());
    if (!data.ok) {
        LogMessage (LOG_LEVEL_WARN, ("Could not find Int named " + key).c_str ());
        return notFoundValue;
    }
    return data.u.i;
}

const std::string
readConfigString (toml_table_t *table, const std::string &key, const std::string &notFoundValue) {
    toml_datum_t data = toml_string_in (table, key.c_str ());
    if (!data.ok) {
        LogMessage (LOG_LEVEL_WARN, ("Could not find String named " + key).c_str ());
        return notFoundValue;
    }
    std::string str = data.u.s;
    toml_myfree (data.u.s);
    return str;
}

std::vector<int64_t>
readConfigIntArray (toml_table_t *table, const std::string &key, std::vector<int64_t> notFoundValue) {
    toml_array_t *array = toml_array_in (table, key.c_str ());
    if (!array) {
        LogMessage (LOG_LEVEL_WARN, ("Could not find int Array named " + key).c_str ());
        return notFoundValue;
    }

    std::vector<int64_t> datas;
    for (int i = 0;; i++) {
        toml_datum_t data = toml_int_at (array, i);
        if (!data.ok) break;
        datas.push_back (data.u.i);
    }

    return datas;
}

std::wstring
replace (const std::wstring orignStr, const std::wstring oldStr, const std::wstring newStr) {
    size_t pos                        = 0;
    std::wstring tempStr              = orignStr;
    std::wstring::size_type newStrLen = newStr.length ();
    std::wstring::size_type oldStrLen = oldStr.length ();
    while (true) {
        pos = tempStr.find (oldStr, pos);
        if (pos == std::wstring::npos) break;

        tempStr.replace (pos, oldStrLen, newStr);
        pos += newStrLen;
    }

    return tempStr;
}

std::string
replace (const std::string orignStr, const std::string oldStr, const std::string newStr) {
    size_t pos                       = 0;
    std::string tempStr              = orignStr;
    std::string::size_type newStrLen = newStr.length ();
    std::string::size_type oldStrLen = oldStr.length ();
    while (true) {
        pos = tempStr.find (oldStr, pos);
        if (pos == std::string::npos) break;

        tempStr.replace (pos, oldStrLen, newStr);
        pos += newStrLen;
    }

    return tempStr;
}

const char *
GameVersionToString (GameVersion version) {
    switch (version) {
    case GameVersion::JPN00: return "JPN00";
    case GameVersion::JPN08: return "JPN08";
    case GameVersion::JPN39: return "JPN39";
    case GameVersion::CHN00: return "CHN00";
    default: return "UNKNOWN";
    }
}

const char *
languageStr (int language) {
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
    std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
    return converter.to_bytes (wstr);
}

bool
AreAllBytesZero (const uint8_t *array, size_t offset, size_t length) {
    for (size_t i = 0; i < length; ++i)
        if (array[offset + i] != 0x00) return false;
    return true;
}