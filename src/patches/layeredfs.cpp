#include "helpers.h"
#include <tomcrypt.h>
#include <zlib.h>

bool useLayeredFs = false;

std::string datatableKey = "3530304242323633353537423431384139353134383346433246464231354534";
std::string fumenKey     = "4434423946383537303842433443383030333843444132343339373531353830";

#define CRCPOLY 0x82F63B78

namespace patches::LayeredFs {
class RegisteredHandler {
public:
    std::function<std::string (std::string, std::string)> handlerMethod;
    RegisteredHandler (const std::function<std::string (std::string, std::string)> &handlerMethod) { this->handlerMethod = handlerMethod; }
};

std::vector<RegisteredHandler *> beforeHandlers = {};
std::vector<RegisteredHandler *> afterHandlers  = {};

uint32_t
CRC32C (uint32_t crc, const unsigned char *buf, size_t len) {
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (CRCPOLY & (0 - (crc & 1)));
    }
    return ~crc;
}

bool
CheckCRC (const std::string &path, uint32_t crc) {
    if (std::filesystem::exists (path)) {
        std::filesystem::path crc_path = path;
        crc_path.replace_extension (".crc");
        std::ifstream crc_file (crc_path, std::ios::binary);
        std::string crc_content ((std::istreambuf_iterator<char> (crc_file)), std::istreambuf_iterator<char> ());
        return std::stoul (crc_content) != crc;
    }
    return 1;
}

void
CreateDirectories (const std::string &path) {
    size_t pos            = 0;
    std::string delimiter = "\\";
    std::string current_path;

    while ((pos = path.find (delimiter, pos)) != std::string::npos) {
        current_path = path.substr (0, pos++);
        if (!current_path.empty () && !CreateDirectory (current_path.c_str (), NULL) && GetLastError () != ERROR_ALREADY_EXISTS)
            throw std::runtime_error ("Error creating directory: " + current_path);
    }
    if (!path.empty () && !CreateDirectory (path.c_str (), NULL) && GetLastError () != ERROR_ALREADY_EXISTS)
        throw std::runtime_error ("Error creating directory: " + path);
}

void
WriteFile (const std::string &filename, const std::vector<uint8_t> &data, uint32_t original_crc) {
    std::string::size_type pos = filename.find_last_of ("\\");
    if (pos != std::string::npos) {
        std::string directory = filename.substr (0, pos);
        CreateDirectories (directory);
    }

    std::filesystem::path crc_path = filename;
    crc_path.replace_extension (".crc");
    std::ofstream crc_file (crc_path.string ());
    crc_file << std::to_string (original_crc);
    crc_file.close ();

    std::ofstream file (filename, std::ios::binary);
    file.write (reinterpret_cast<const char *> (data.data ()), data.size ());
}

std::vector<unsigned char>
GZip_Compress (const std::vector<unsigned char> &data) {
    z_stream deflate_stream;
    deflate_stream.zalloc   = Z_NULL;
    deflate_stream.zfree    = Z_NULL;
    deflate_stream.opaque   = Z_NULL;
    deflate_stream.avail_in = data.size ();
    deflate_stream.next_in  = const_cast<Bytef *> (data.data ());

    deflateInit2 (&deflate_stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);

    std::vector<unsigned char> compressed_data;
    compressed_data.resize (deflateBound (&deflate_stream, data.size ()));

    deflate_stream.avail_out = compressed_data.size ();
    deflate_stream.next_out  = compressed_data.data ();

    deflate (&deflate_stream, Z_FINISH);
    deflateEnd (&deflate_stream);

    compressed_data.resize (deflate_stream.total_out);
    return compressed_data;
}

// Function to pad data according to PKCS7
std::vector<uint8_t>
Pad_Data (const std::vector<uint8_t> &data, size_t block_size) {
    size_t padding                   = block_size - (data.size () % block_size);
    std::vector<uint8_t> padded_data = data;
    padded_data.insert (padded_data.end (), padding, static_cast<uint8_t> (padding));
    return padded_data;
}

std::vector<uint8_t>
Hex_To_Bytes (const std::string &hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length (); i += 2) {
        uint8_t byte = static_cast<uint8_t> (std::stoi (hex.substr (i, 2), nullptr, 16));
        bytes.push_back (byte);
    }
    return bytes;
}

std::vector<uint8_t>
EncryptFile (const std::string &input_file, const std::string &hex_key) {
    // Convert the key from hex to bytes
    std::vector<uint8_t> key = Hex_To_Bytes (hex_key);

    // Generate the 128 bits IV
    std::vector<uint8_t> iv (16);
    for (size_t i = 0; i < iv.size (); ++i)
        iv[i] = static_cast<uint8_t> (i);

    // Read the entire file into memory
    std::ifstream file (input_file, std::ios::binary);

    std::vector<uint8_t> data ((std::istreambuf_iterator<char> (file)), std::istreambuf_iterator<char> ());

    // Compress the data
    std::vector<uint8_t> compressed_data = GZip_Compress (data);

    // Pad the compressed data
    std::vector<uint8_t> padded_data = Pad_Data (compressed_data, 16);

    // Encrypt the data
    symmetric_CBC cbc;
    if (cbc_start (find_cipher ("aes"), iv.data (), key.data (), key.size (), 0, &cbc) != CRYPT_OK)
        throw std::runtime_error ("Error initializing CBC");

    std::vector<uint8_t> encrypted_data (padded_data.size ());
    if (cbc_encrypt (padded_data.data (), encrypted_data.data (), padded_data.size (), &cbc) != CRYPT_OK)
        throw std::runtime_error ("Error during encryption");

    cbc_done (&cbc);

    // Return IV concatenated with the encrypted data
    encrypted_data.insert (encrypted_data.begin (), iv.begin (), iv.end ());
    return encrypted_data;
}

bool
IsFumenEncrypted (const std::string &filename) {
    // Check if the filename ends with ".bin"
    if (filename.size () < 4 || filename.substr (filename.size () - 4) != ".bin")
        return true; // If it doesn't we return early, as the file we're seeing isn't a fumen !

    std::ifstream file (filename, std::ios::binary);
    file.seekg (0x214, std::ios::beg);
    std::vector<unsigned char> buffer (24);
    file.read (reinterpret_cast<char *> (buffer.data ()), buffer.size ());

    // Check if the read bytes match the expected pattern
    std::vector<unsigned char> expected_bytes = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    return buffer != expected_bytes;
}

std::string
LayeredFsHandler (const std::string originalFileName, const std::string currentFileName) {
    std::filesystem::path path (originalFileName.c_str ());
    if (!path.is_absolute ()) path = std::filesystem::absolute (path);
    auto originalDataFolder       = std::filesystem::current_path ().parent_path ().parent_path () / "Data" / "x64";
    auto originalLayeredFsFolder  = std::filesystem::current_path ().parent_path ().parent_path () / "Data_mods" / "x64";
    auto encryptedLayeredFsFolder = std::filesystem::current_path ().parent_path ().parent_path () / "Data_mods" / "x64_enc";

    if (path.string ().find (originalDataFolder.string ()) == 0) {
        auto newPath = path.string ();
        auto encPath = path.string ();
        newPath.replace (0, originalDataFolder.string ().length (), originalLayeredFsFolder.string ());
        encPath.replace (0, originalDataFolder.string ().length (), encryptedLayeredFsFolder.string ());

        // The following code handles file redirection and if need be, file encryption.
        // It's a bit of a mess but it works well ! -Kit

        if (std::filesystem::exists (newPath)) { // If a file exists in the datamod folder
            if (IsFumenEncrypted (newPath)) {    // And if it's an encrypted fumen or a different type of file, use it.
                LogMessage (LOG_LEVEL_DEBUG, ("Redirecting " + std::filesystem::relative (path).string ()).c_str ());
                return newPath;
            } else {                                      // Otherwise if it's an unencrypted fumen.
                if (!std::filesystem::exists (encPath)) { // We check if we don't already have a cached file.
                    if (fumenKey.length () == 64) {
                        LogMessage (LOG_LEVEL_DEBUG,
                                    ("Encrypting " + std::filesystem::relative (newPath).string ()).c_str ()); // If we don't we encrypt the file
                        std::ifstream crc_file (newPath, std::ios::binary);
                        std::vector<uint8_t> crc_vector ((std::istreambuf_iterator<char> (crc_file)), std::istreambuf_iterator<char> ());
                        uint32_t crc = CRC32C (0, crc_vector.data (), crc_vector.size ());
                        WriteFile (encPath, EncryptFile (newPath, fumenKey), crc); // And we save it
                    } else {
                        LogMessage (
                            LOG_LEVEL_ERROR,
                            ("Missing or invalid fumen key: " + std::filesystem::relative (newPath).string () + " couldn't be encrypted.").c_str ());
                        encPath = path.string ();
                    }
                } else LogMessage (LOG_LEVEL_DEBUG, ("Using cached file for: " + std::filesystem::relative (newPath).string ()).c_str ());
                return encPath;
            }
        }

        // We check separately for unencrypted json files.
        std::filesystem::path json_path = newPath;
        json_path.replace_extension (".json");
        if (std::filesystem::exists (json_path)) { // If a json file exists in the folder
            bool crcBool = false;

            if (std::filesystem::exists (encPath)) {
                std::ifstream crc_file (json_path, std::ios::binary);
                std::vector<uint8_t> crc_vector ((std::istreambuf_iterator<char> (crc_file)), std::istreambuf_iterator<char> ());
                uint32_t crc = CRC32C (0, crc_vector.data (), crc_vector.size ());
                crcBool      = CheckCRC (encPath, crc);
            }

            if (!std::filesystem::exists (encPath) || crcBool) { // And if it hasn't been encrypted before
                if (datatableKey.length () == 64) {
                    // Encrypt the file
                    LogMessage (LOG_LEVEL_DEBUG, ("Encrypting " + std::filesystem::relative (json_path).string ()).c_str ());
                    std::ifstream crc_file (json_path.string (), std::ios::binary);
                    std::vector<uint8_t> crc_vector ((std::istreambuf_iterator<char> (crc_file)), std::istreambuf_iterator<char> ());
                    uint32_t crc = CRC32C (0, crc_vector.data (), crc_vector.size ());
                    WriteFile (encPath, EncryptFile (json_path.string (), datatableKey), crc); // And save it
                } else {
                    LogMessage (
                        LOG_LEVEL_ERROR,
                        ("Missing or invalid datatable key: " + std::filesystem::relative (newPath).string () + " couldn't be encrypted.").c_str ());
                    encPath = path.string ();
                }
            } else
                // Otherwise use the already encrypted file.
                LogMessage (LOG_LEVEL_DEBUG, ("Using cached file for: " + std::filesystem::relative (json_path).string ()).c_str ());
            return encPath;
        }
    }

    return ""; // we return an empty string, causing the rest of CreateFileAHook to not update the returned value
}

HOOK (HANDLE, CreateFileAHook, PROC_ADDRESS ("kernel32.dll", "CreateFileA"), LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
      LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    std::string originalFileName = std::string (lpFileName);
    std::string currentFileName  = originalFileName;
    LogMessage (LOG_LEVEL_HOOKS, ("CreateFileA: " + originalFileName).c_str ());

    if (!beforeHandlers.empty ()) {
        for (auto handler : beforeHandlers) {
            std::string result = handler->handlerMethod (originalFileName, currentFileName);
            if (result != "") currentFileName = result;
        }
    }

    if (useLayeredFs) {
        std::string result = LayeredFsHandler (originalFileName, currentFileName);
        if (result != "") currentFileName = result;
    }

    if (!afterHandlers.empty ()) {
        for (auto handler : afterHandlers) {
            std::string result = handler->handlerMethod (originalFileName, currentFileName);
            if (result != "") currentFileName = result;
        }
    }

    return originalCreateFileAHook (currentFileName.c_str (), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                                    dwFlagsAndAttributes, hTemplateFile);
}

// HOOK (HANDLE, CreateFileWHook, PROC_ADDRESS ("kernel32.dll", "CreateFileW"), LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
//       LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
//     std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
//     std::string strFileName = converter.to_bytes (lpFileName);

//     std::filesystem::path path (strFileName);
//     if (!path.is_absolute ()) path = std::filesystem::absolute (path);

//     auto originalDataFolder      = std::filesystem::current_path ().parent_path ().parent_path () / "Data" / "x64";
//     auto originalLayeredFsFolder = std::filesystem::current_path ().parent_path ().parent_path () / "Data_mods" / "x64";

//     if (path.string ().find (originalDataFolder.string ()) != std::string::npos) {
//         auto newPath = path.string ();
//         newPath.replace (0, originalDataFolder.string ().length (), originalLayeredFsFolder.string ());

//         if (std::filesystem::exists (newPath)) {
//             std::wstring wNewPath = converter.from_bytes (newPath);
//             std::wcout << L"Redirecting " << lpFileName << L" to " << wNewPath << std::endl;
//             return originalCreateFileWHook (wNewPath.c_str (), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
//                                             dwFlagsAndAttributes, hTemplateFile);
//         }
//     }

//     return originalCreateFileWHook (lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes,
//                                     hTemplateFile);
// }

void
Init () {
    LogMessage (LOG_LEVEL_INFO, "Init LayeredFs patches");

    auto configPath = std::filesystem::current_path () / "config.toml";
    std::unique_ptr<toml_table_t, void (*) (toml_table_t *)> config_ptr (openConfig (configPath), toml_free);
    if (config_ptr) {
        auto layeredFs = openConfigSection (config_ptr.get (), "layeredfs");
        if (layeredFs) useLayeredFs = readConfigBool (layeredFs, "enabled", useLayeredFs);
    }
    register_cipher (&aes_desc);
    INSTALL_HOOK (CreateFileAHook);
    // INSTALL_HOOK (CreateFileWHook);
}

void
RegisterBefore (const std::function<std::string (const std::string, const std::string)> &fileHandler) {
    beforeHandlers.push_back (new RegisteredHandler (fileHandler));
}

void
RegisterAfter (const std::function<std::string (const std::string, const std::string)> &fileHandler) {
    afterHandlers.push_back (new RegisteredHandler (fileHandler));
}

} // namespace patches::LayeredFs
