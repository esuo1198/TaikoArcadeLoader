default: dist-no-7z

all:
	@meson compile -C build
	@strip build/bnusio.dll

setup:
	@meson setup build --cross cross-mingw-64.txt

clean-setup:
	@meson setup --wipe build --cross cross-mingw-64.txt

clean:
	@rm -rf out
	@rm -rf build
	@rm -f dist.7z
	@cd subprojects && find . -maxdepth 1 ! -name packagefiles -type d -not -path '.' -exec rm -rf {} +

dist-no-7z: all
	@mkdir -p out/
	@cp build/bnusio.dll out/
	@cp -r dist/* out/

dist: dist-no-7z
	@cd out && 7z a -t7z ../dist.7z .
	@rm -rf out
