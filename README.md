# TaikoArcadeLoader

This is a loader for Taiko no Tatsujin Nijiiro ver.  
It currently supports the following versions:

* JPN 00.18
* JPN 08.18
* JPN 39.06
* CHN 00.32 (based on JPN 32.09)

## Setup

Copy the extracted contents of `dist.zip` to the same directory as Taiko.exe  
If your game hangs on a black screen at launch for more than a minute, Start Taiko.exe as Administrator.

### config.toml

```toml
[amauth]
server = "127.0.0.1"
port = "54430"
chassis_id = "284111080000"
shop_id = "TAIKO ARCADE LOADER"
game_ver = "00.00"
country_code = "JPN"

[patches]
version = "auto"            # Patch version
                            # | - auto: hash detection (you need to use the original exe otherwise it will not load).
                            # | - JPN00: For use with Taiko JPN 00.18
                            # | - JPN08: For use with Taiko JPN 08.18
                            # | - JPN39: For use with Taiko JPN 39.06
                            # | - CHN00: For use with Taiko CHN 00.32
unlock_songs = true

[patches.chn00]             # These patches are only available for version CHN00
fix_language = false        # Sync test mode language to attract etc
demo_movie = true           # Show demo movie
mode_collabo025 = false     # Enable one piece collab mode
mode_collabo026 = false     # Enable ai soshina mode

[patches.jpn39]             # These patches are only available for version JPN39
fix_language = false        # Sync test mode language to attract etc
chs_patch = false           # Use Chinese font and Simplified Chinese values from the wordlist
                            # More options are available in the ModManager, in the TestMode menu (Default key is F1)

[emulation]
usio = true                 # If usio emulation is disabled, you need to place bnusio_original.dll (unmodified bnusio.dll) in the executable folder.
card_reader = true          # Disable this if you have an original namco card reader
accept_invalid = false      # Enable this if you want to accept cards incompatible with the original readers 
qr = true                   # Disable this if you have an original namco qr code scanner

[graphics]
res = { x = 1920, y = 1080 }
windowed = false
vsync = false
fpslimit = 120

[audio]
wasapi_shared = true        # Wasapi shared mode, allows you to have multiple audio sources at once at a cost of having higher latency.
asio = false                # Use asio audio mode
asio_driver = "ASIO4ALL v2" # Asio driver name
                            # | If you're not using asio4all, open up regedit then navigate to HKEY_LOCAL_MACHINE\SOFTWARE\ASIO for your driver's name.
                            # | It is case sensitive.

[qr]
image_path = ""             # Path to the image of the QR Code you want to use

[qr.data]                   # qr data used for other events (ex. gaiden, custom folder)
serial = ""                 # qr serial
type = 0                    # qr type
                            # | 0: default (serial only)
                            # | 5: custom folder
song_no = []                # Song noes used for custom folder

[controller]
wait_period = 4             # Input interval (if using taiko drum controller, should be set to 0)
analog_input = false        # Use analog input (you need a compatible controller, this allows playing small and big notes like on arcade cabinets)

[keyboard]
auto_ime = false            # Automatically change to english ime mode upon game startup
jp_layout = false           # Use jp layout scan code (if using jp layout keyboard, must be set to true)

[layeredfs]
enabled = false             # Replace assets from the game using a layered file system.
                            # | For example if you want to edit the wordlist, add your edited version like so:
                            # | .\Data_mods\x64\datatable\wordlist.json 
                            # | You can provide both unencrypted and encrypted files. 

[logging]
log_level = "INFO"          # Log level, Can be either "NONE", "ERROR", "WARN", "INFO", "DEBUG" and "HOOKS"
                            # | Keep this as low as possible (Info is usually more than enough) as more logging will slow down your game
log_to_file = false         # Log to file, set this to true to save the logs from your last session to TaikoArcadeLoader.log
                            # |Again, if you do not have a use for this (debugging mods or whatnot), turn it off.
```

## TestMode options (JPN39 only)

TaikoArcadeLoader offers several patches to select in TestMode  

The follow options are available in "MOD MANAGER" menu:

* FREEZE TIMER (stop timer count down)
* KIMETSU MODE (enable collabo024, will show a blank title)
* ONE PIECE MODE (enable collabo025)
* AI SOSHINA MODE (enable collabo026)
* AOHARU MODE (enable aprilfool001)
* INSTANT RESULT (send result per song)  

Enhanced original option:

* Louder volume (Speaker Volume is now up to 300%, **WARNING: May damage your speakers**)

## Building Manually

To compile TaikoArcadeLoader, you'll need to install [MSVC](https://aka.ms/vs/17/release/vs_BuildTools.exe).

Loading this project in cliON or vscode with the cmake tools addon should then allow you to build the project.  
If you want to build yourself, here are some instructions on how to do this from a cmd shell.

Clone this repository, open *cmd* and run the following commands:

```bash
# Load the MSVC environment (Change this to your actual vcvarsall.bat path)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

# Configure the build folder (this is only needed the first time)
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build TaikoArcadeLoader
cmake --build build --config Release --target bnusio
```

The compiled build of TaikoArcadeLoader will be written in the `dist` folder.
