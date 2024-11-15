project('TaikoArcadeLoader', 'c', 'cpp', version: '1.0.0', default_options : ['c_std=c11', 'cpp_std=c++23'])
warning_level = 3
debug = true
optimization = 3
b_lto = true
b_pgo = 'use'
cmake = import('cmake')
opt_var = cmake.subproject_options()
opt_var.set_override_option('cpp_std', 'c++23')

cpp = meson.get_compiler('cpp')

add_project_arguments(
	cpp.get_supported_arguments(
		'-D_WIN32_WINNT=_WIN32_WINNT_WIN10',
	),
	language: 'cpp',
)

add_project_link_arguments(
	cpp.get_supported_arguments(
		'-static',
		#'-s',
		'-lws2_32',
		'-lssp',
		'-lntdll',
	),
	language: 'cpp',
)

tomlc99 = subproject('tomlc99')
sdl2 = subproject('sdl2', default_options: ['default_library=static', 'test=false', 'use_render=disabled'])
xxhash = subproject('xxhash', default_options: ['default_library=static', 'cli=false'])
opt_var.add_cmake_defines({'SAFETYHOOK_FETCH_ZYDIS': true})
safetyhook_proj = cmake.subproject('safetyhook', options: opt_var)
safetyhook_dep = safetyhook_proj.dependency('safetyhook')
zydis_dep = safetyhook_proj.dependency('Zydis')
stb = subproject('stb')
opt_var.add_cmake_defines({'BUILD_EXAMPLES': false})
zxing_proj = cmake.subproject('zxing', options: opt_var)
zxing_dep = zxing_proj.dependency('ZXing')
zlib_proj = subproject('zlib')
zlib_dep = zlib_proj.get_variable('zlib_dep')
libtomcrypt = subproject('libtomcrypt')
libtomcrypt_dep = libtomcrypt.get_variable('tomcrypt_dep')
opt_var = cmake.subproject_options()
opt_var.set_override_option('cpp_std', 'c++23')
opt_var.add_cmake_defines({'PUGIXML_WCHAR_MODE': true})
pugixml = cmake.subproject('pugixml', options: opt_var)
pugixml_dep = pugixml.get_variable('pugixml_static_dep')

library(
	'bnusio',
	link_with: [
		tomlc99.get_variable('tomlc99_lib'),
		sdl2.get_variable('sdl2'),
		xxhash.get_variable('xxhash'),
		zlib_proj.get_variable('zlib_lib'),
		libtomcrypt.get_variable('tomcryptlib'),
	],
	link_args : '-Wl,--allow-multiple-definition',
	include_directories: [
		'src',
		tomlc99.get_variable('tomlc99_inc'),
		sdl2.get_variable('core_inc'),
		xxhash.get_variable('inc'),
		zlib_proj.get_variable('zlib_inc'),
		libtomcrypt.get_variable('core_inc'),
	],
	dependencies: [
		stb.get_variable('stb_dep'),
		zxing_dep,
		safetyhook_dep,
		zydis_dep,
		zlib_dep,
		libtomcrypt_dep,
		pugixml_dep,
	],
	sources : [
		'src/dllmain.cpp',
		'src/helpers.cpp',
		'src/logger.cpp',
		'src/poll.cpp',
		'src/bnusio.cpp',
		'src/patches/amauth.cpp',
		'src/patches/dxgi.cpp',
		'src/patches/fpslimiter.cpp',
		'src/patches/audio.cpp',
		'src/patches/qr.cpp',
		'src/patches/layeredfs.cpp',
		'src/patches/testmode.cpp',
		'src/patches/versions/JPN00.cpp',
		'src/patches/versions/JPN08.cpp',
		'src/patches/versions/JPN39.cpp',
		'src/patches/versions/CHN00.cpp',
	],
	name_prefix: ''
)
