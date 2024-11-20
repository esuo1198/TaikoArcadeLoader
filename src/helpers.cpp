#include "helpers.h"
#include <windows.h>

void *consoleHandle = 0;

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
        LogMessage (__FUNCTION__, __FILE__, __LINE__, (std::string (path.string ()) + ": file does not exist").c_str (), LOG_LEVEL_WARN);
        return 0;
    }

    std::ifstream stream (path);
    if (!stream.is_open ()) {
        LogMessage (__FUNCTION__, __FILE__, __LINE__, ("Could not open " + std::string (path.string ())).c_str (), LOG_LEVEL_WARN);
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
        LogMessage (__FUNCTION__, __FILE__, __LINE__, (path.string () + ": " + errorbuf).c_str (), LOG_LEVEL_WARN);
        return 0;
    }

    return config;
}

toml_table_t *
openConfigSection (toml_table_t *config, const std::string &sectionName) {
    toml_table_t *section = toml_table_in (config, sectionName.c_str ());
    if (!section) {
        LogMessage (__FUNCTION__, __FILE__, __LINE__, ("Cannot find section " + sectionName).c_str (), LOG_LEVEL_WARN);
        return 0;
    }

    return section;
}

bool
readConfigBool (toml_table_t *table, const std::string &key, bool notFoundValue) {
    toml_datum_t data = toml_bool_in (table, key.c_str ());
    if (!data.ok) {
        LogMessage (__FUNCTION__, __FILE__, __LINE__, ("Could not find Boolean named " + key).c_str (), LOG_LEVEL_WARN);
        return notFoundValue;
    }
    return (bool)data.u.b;
}

int64_t
readConfigInt (toml_table_t *table, const std::string &key, int64_t notFoundValue) {
    toml_datum_t data = toml_int_in (table, key.c_str ());
    if (!data.ok) {
        LogMessage (__FUNCTION__, __FILE__, __LINE__, ("Could not find Int named " + key).c_str (), LOG_LEVEL_WARN);
        return notFoundValue;
    }
    return data.u.i;
}

const std::string
readConfigString (toml_table_t *table, const std::string &key, const std::string &notFoundValue) {
    toml_datum_t data = toml_string_in (table, key.c_str ());
    if (!data.ok) {
        LogMessage (__FUNCTION__, __FILE__, __LINE__, ("Could not find String named " + key).c_str (), LOG_LEVEL_WARN);
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
        LogMessage (__FUNCTION__, __FILE__, __LINE__, ("Could not find int Array named " + key).c_str (), LOG_LEVEL_WARN);
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