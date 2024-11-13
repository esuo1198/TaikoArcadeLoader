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
        printWarning ("%s (%s): file does not exist\n", __func__, path.string ().c_str ());
        return 0;
    }

    std::ifstream stream (path);
    if (!stream.is_open ()) {
        printWarning ("%s (%s): could not open\n", __func__, path.string ().c_str ());
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
        printWarning ("%s (%s): %s\n", __func__, path.string ().c_str (), errorbuf);
        return 0;
    }

    return config;
}

toml_table_t *
openConfigSection (toml_table_t *config, const std::string &sectionName) {
    toml_table_t *section = toml_table_in (config, sectionName.c_str ());
    if (!section) {
        printWarning ("%s (%s): cannot find section\n", __func__, sectionName.c_str ());
        return 0;
    }

    return section;
}

bool
readConfigBool (toml_table_t *table, const std::string &key, bool notFoundValue) {
    toml_datum_t data = toml_bool_in (table, key.c_str ());
    if (!data.ok) return notFoundValue;

    return (bool)data.u.b;
}

int64_t
readConfigInt (toml_table_t *table, const std::string &key, int64_t notFoundValue) {
    toml_datum_t data = toml_int_in (table, key.c_str ());
    if (!data.ok) return notFoundValue;

    return data.u.i;
}

const std::string
readConfigString (toml_table_t *table, const std::string &key, const std::string &notFoundValue) {
    toml_datum_t data = toml_string_in (table, key.c_str ());
    if (!data.ok) return notFoundValue;
    std::string str = data.u.s;
    toml_myfree (data.u.s);

    return str;
}

std::vector<int64_t>
readConfigIntArray (toml_table_t *table, const std::string &key, std::vector<int64_t> notFoundValue) {
    toml_array_t *array = toml_array_in (table, key.c_str ());
    if (!array) return notFoundValue;

    std::vector<int64_t> datas;
    for (int i = 0;; i++) {
        toml_datum_t data = toml_int_at (array, i);
        if (!data.ok) break;
        datas.push_back (data.u.i);
    }

    return datas;
}

void
printColour (int colour, const char *format, ...) {
    va_list args;
    va_start (args, format);

    if (consoleHandle == 0) consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute (consoleHandle, colour);
    vprintf (format, args);
    SetConsoleTextAttribute (consoleHandle, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);

    va_end (args);
}

std::wstring
replace (const std::wstring orignStr, const std::wstring oldStr, const std::wstring newStr) {
    size_t pos = 0;
    std::wstring tempStr = orignStr;
    std::wstring::size_type newStrLen = newStr.length();
    std::wstring::size_type oldStrLen = oldStr.length();
    while(true)
    {
        pos = tempStr.find(oldStr, pos);
        if (pos == std::wstring::npos) break;

        tempStr.replace(pos, oldStrLen, newStr);        
        pos += newStrLen;
    }

    return tempStr;
}

std::string
replace (const std::string orignStr, const std::string oldStr, const std::string newStr) {
    size_t pos = 0;
    std::string tempStr = orignStr;
    std::string::size_type newStrLen = newStr.length();
    std::string::size_type oldStrLen = oldStr.length();
    while(true)
    {
        pos = tempStr.find(oldStr, pos);
        if (pos == std::string::npos) break;

        tempStr.replace(pos, oldStrLen, newStr);        
        pos += newStrLen;
    }

    return tempStr;
}