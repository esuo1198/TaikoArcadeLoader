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
# server ip
server = "127.0.0.1"
# server port
port = "54430"
# dongle serial
chassis_id = "284111080000"
# shop name
shop_id = "TAIKO ARCADE LOADER"
# game version
game_ver = "00.00"
# country code
country_code = "JPN"

[patches]
# patch version
# auto: hash detection (you need to use the original exe)
# JPN00: JPN 00.18
# JPN08: JPN 08.18
# JPN39: JPN 39.06
# CHN00: CHN 00.32 (based on JPN 32.09)
version = "auto"
# unlock all songs
unlock_songs = true

  [patches.chn00]
  # sync test mode language to attract etc
  fix_language = false
  # show demo movie
  demo_movie = true
  # enable one piece collab mode
  mode_collabo025 = false
  # enable ai soshina mode
  mode_collabo026 = false

  [patches.jpn39]
  # sync test mode language to attract etc
  fix_language = false
  # use cn font and chineseS wordlist value
  chs_patch = false
  # more options is now moved to testmode

[emulation]
# If usio emulation is disabled, you need to place bnusio_original.dll (unmodified bnusio.dll) in the executable folder.
usio = true
card_reader = true
qr = true

[graphics]
# window resolution
res = { x = 1920, y = 1080 }
# window mode
windowed = false
# vertical sync
vsync = false
# fps limit
fpslimit = 120

[audio]
# wasapi shared mode
# allows you to have multiple audio sources at once at a cost of having higher latency.
wasapi_shared = true
# use asio audio mode
asio = false
# asio driver name
# to find it, open up regedit then navigate to HKEY_LOCAL_MACHINE\SOFTWARE\ASIO
# the name is case sensitive.
asio_driver = ""

[qr]
# qr image path
image_path = ""

  # qr data used for other events (ex. gaiden, custom folder)
  [qr.data]
  # qr serial
  serial = ""
  # qr type
  # 0: default (serial only)
  # 5: custom folder
  type = 0
  # song noes used for custom folder
  song_no = []

[drum]
# input interval (if using taiko drum controller, should be set to 0)
wait_period = 4

[controller]
# use analog input
analog_input = false

[keyboard]
# auto change to english ime mode
auto_ime = false
# use jp layout scan code (if using jp layout keyboard, must be set to true)
jp_layout = false

[layeredfs]
# replace assets from the game using a layered file system.
# For example if you want to edit the wordlist, add your edited version like so:
# .\Data_mods\x64\datatable\wordlist.bin
enabled = false
# AES encryption keys needed to dynamically encrypt datatable files and fumens.
# keys need to be provided in an hexlified form. A missing or incorrect key will crash the game.
# keys are not needed if you provide already encrypted files.
datatable_key = ""
fumen_key = ""
```

## TestMode options (JPN39 only)

TaikoArcadeLoader offers several patches to select in TestMode  

The follow options are available in "MOD MANAGER" menu:
- FREEZE TIMER (stop timer count down)
- KIMETSU MODE (enable collabo024, will show a blank title)
- ONE PIECE MODE (enable collabo025)
- AI SOSHINA MODE (enable collabo026)
- AOHARU MODE (enable aprilfool001)
- INSTANT RESULT (send result per song)  

Enhanced original option:
- Louder volume (Speaker Volume is now up to 300%, **WARNING: May damage your speakers**)

## Building

TaikoArcadeLoader can be a bit tricky to build if you've never done it before.  
Go to the TaikoArcadeLoader folder and run the following commands:

```bash
apt install -y npm mingw-w64 ninja-build nasm pip 7zip cmake
pip3 install meson

npm install n -g && n latest && npm install --global xpm@latest
npx xpm init && npx xpm install @xpack-dev-tools/mingw-w64-gcc@latest

# make sure you entered TaikoArcadeLoader folder
export PATH=`pwd`/xpacks/.bin:$PATH
make setup
```

Once that's done, run `make` to build TAL.  
The output will be written in the `out` folder.