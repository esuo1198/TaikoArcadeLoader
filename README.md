# TaikoArcadeLoader

This is a loader for Taiko no Tatsujin Nijiiro ver.  
It currently supports the following versions:

* JPN 00.18
* JPN 08.18
* JPN 39.06
* CHN 00.32 (based on JPN 32.09)

**Attention**: As far as we know, the game can't run on graphic card equals or lower than GTX 7xx series(GTX 7xx is not supported either), so if the game crashes before it shows `avaliableRefreshRate`, that most likely means your graphic card is not supported!

## Setup

First of all, you may rename your game's ``bnusio.dll`` into ``bnusio_original.dll`` if you don't want TaikoArcadeLoader to emulate it.

Then, if you downloaded a pre-made ``dist.zip`` file:
- Extract its contents into the same directory as ``Taiko.exe``
If you compiled TaikoArcadeLoader yourself:
- Copy the content of the ``dist`` directory into the same directory as ``Taiko.exe``

If your game hangs on a black screen at launch for more than a minute, try to start Taiko.exe as Administrator.

## Building Manually

To compile TaikoArcadeLoader, you'll need at least:
- [CMake](https://github.com/Kitware/CMake/releases/tag/v3.25.3) 3.25 <= version < 4.0
- [MSVC](https://aka.ms/vs/17/release/vs_BuildTools.exe).

Loading this project in CLion or VSCode with the CMake Tools addon should then allow you to build the project.  
Do note that the ``.sln`` files created after you run the configure command CAN be opened using Visual Studio or Rider.  
If you want to build yourself, here are some instructions on how to do this from a ``cmd`` prompt.  

Clone this repository, open a ``cmd`` in its directory and run the following commands:

```bash
# Load the MSVC environment (Change this to your actual vcvarsall.bat path)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

# Configure the build folder (this is only needed the first time)
cmake -B build -S . -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build TaikoArcadeLoader
cmake --build build --config Release --target bnusio
```

The compiled DLL of TaikoArcadeLoader (named ``bnusio.dll``) will be created in the `dist` folder.

## Configuration

### The ``config.toml`` file

It contains various settings for you to adjust so the game works best on your device.

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
unlock_songs = true         # not active for JPN39 (see TestMode)

[patches.chn00]             # These patches are only available for version CHN00
fix_language = false        # Sync test mode language to attract etc
demo_movie = true           # Show demo movie
mode_collabo025 = false     # Enable one piece collab mode
mode_collabo026 = false     # Enable ai soshina mode

[patches.jpn39]             # These patches are only available for version JPN39
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
cursor = true
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



#### TestMode options (JPN39 only)

In JPN39, TaikoArcadeLoader offers several patches to select in TestMode.

TestMode can be entered at any time by pressing F1 (or another key if you've changed it) after the first loading screen.

A new option "MOD MANAGER" will be added by TaikoArcadeLoader, which has the following options:

* FIX LANGUAGE (sync test mode language to attract etc)
* UNLOCK SONGS (show all of the songs)
* FREEZE TIMER (stop timer count down)
* KIMETSU MODE (enable collabo024, will show a blank title)
* ONE PIECE MODE (enable collabo025)
* AI SOSHINA MODE (enable collabo026)
* AOHARU MODE (enable aprilfool001)
* INSTANT RESULT (send result per song)  

Enhanced original option:

* Louder volume (Speaker Volume is now up to 300%, **WARNING: May damage your speakers**)
* Attract demo (Only available if FIX LANGUAGE is ON)

### The ``keyconfig.toml`` file

It contains the keybindings configuration for each of the game's inputs.

The available key names are documented at the bottom of the file.

```toml
EXIT = ["ESCAPE"]

TEST = ["F1"]
SERVICE = ["F2"]
DEBUG_UP = ["UPARROW"]
DEBUG_DOWN = ["DOWNARROW"]
DEBUG_ENTER = ["ENTER"]

COIN_ADD = ["ENTER", "SDL_START"]
CARD_INSERT_1 = ["P"]
CARD_INSERT_2 = []
QR_DATA_READ = ["Q"]
QR_IMAGE_READ = ["W"]

P1_LEFT_BLUE = ["D", "SDL_LTRIGGER"]
P1_LEFT_RED = ["F", "SDL_LSTICK_PRESS"]
P1_RIGHT_RED = ["J", "SDL_RSTICK_PRESS"]
P1_RIGHT_BLUE = ["K", "SDL_RTRIGGER"]
P2_LEFT_BLUE = ["Z"]
P2_LEFT_RED = ["X"]
P2_RIGHT_RED = ["C"]
P2_RIGHT_BLUE = ["V"]

# ESCAPE F1 through F12 
# ` 1 through 0 -= BACKSPACE ^ YEN
# TAB QWERTYUIOP [ ] BACKSLASH @
# CAPS_LOCK ASDFGHJKL ;' ENTER :
# SHIFT ZXCVBNM , . SLASH
# CONTROL L_WIN ALT SPACE R_WIN MENU
# SCROLL_LOCK PAUSE INSERT DELETE HOME END PAGE_UP PAGE_DOWN
# UPARROW LEFTARROW DOWNARROW RIGHTARROW
# NUM0 through NUM9 NUM_LOCK DIVIDE MULTIPLY SUBTRACT ADD DECIMAL
# SCROLL_UP SCROLL_DOWN
# SDL_A SDL_B SDL_X SDL_Y
# SDL_BACK SDL_GUIDE SDL_START
# SDL_LSHOULDER SDL_LTRIGGER SDL_RSHOULDER SDL_RTRIGGER
# SDL_DPAD_UP SDL_DPAD_LEFT SDL_DPAD_DOWN SDL_DPAD_RIGHT
# SDL_MISC SDL_PADDLE1 SDL_PADDLE2 SDL_PADDLE3 SDL_PADDLE4 SDL_TOUCHPAD
# SDL_LSTICK_UP SDL_LSTICK_LEFT SDL_LSTICK_DOWN SDL_LSTICK_RIGHT SDL_LSTICK_PRESS
# SDL_RSTICK_UP SDL_RSTICK_LEFT SDL_RSTICK_DOWN SDL_RSTICK_RIGHT SDL_RSTICK_PRESS
```

### The ``gamecontrollerdb.txt``

This file lists bindings between many controllers' numbered keys and their named key equivalent.

## Finding access codes

The default configuration lets you connect the game to a local server (for example, a [TaikoLocalServer](https://github.com/asesidaa/TaikoLocalServer) instance) hosted on the same computer.

By default, TaikoArcadeLoader will create a ``card.ini`` file on its first launch containing two access code / chip ID pairs.

Those are the access codes to be used to register accounts in your local server for play data respectively related to the emulated cards associated to pressing the ``CARD_INSERT_1`` and ``CARD_INSERT_2`` keybindings in-game.