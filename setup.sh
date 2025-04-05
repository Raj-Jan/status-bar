
w="$(pkg-config wayland-protocols --variable=pkgdatadir)"
p="stable/xdg-shell/xdg-shell.xml"
o="code/wayland/protocol"

if [ ! -d $o ]; then
	mkdir $o
fi

wayland-scanner client-header "$w/$p" $o/xdg-shell.h
wayland-scanner private-code "$w/$p" $o/xdg-shell.c

w="$(pkg-config wlr-protocols --variable=pkgdatadir)"
p="unstable/wlr-layer-shell-unstable-v1.xml"

wayland-scanner client-header "$w/$p" $o/wlr-layer-shell.h
wayland-scanner private-code "$w/$p" $o/wlr-layer-shell.c

o="code/draw/icon"

if [ ! -d $o ]; then
	mkdir $o
fi

python convert_icon.py icon/vscode.png $o/vscode.h
python convert_icon.py icon/discord.png $o/discord.h
python convert_icon.py icon/firefox.png $o/firefox.h
python convert_icon.py icon/steam.png $o/steam.h
python convert_icon.py icon/spotify.png $o/spotify.h
python convert_icon.py icon/blender.png $o/blender.h
python convert_icon.py icon/krita.png $o/krita.h
python convert_icon.py icon/osu.png $o/osu.h
