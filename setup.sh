
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

python convert_icon.py icon/bolt.png $o/bolt.h
python convert_icon.py icon/power-off.png $o/power-off.h
python convert_icon.py icon/rotate-left.png $o/rotate-left.h
python convert_icon.py icon/gear.png $o/gear.h
python convert_icon.py icon/moon.png $o/moon.h

python convert_icon.py icon/backward.png $o/backward.h
python convert_icon.py icon/forward.png $o/forward.h
python convert_icon.py icon/pause.png $o/pause.h
python convert_icon.py icon/play.png $o/play.h

python convert_icon.py icon/microchip.png $o/microchip.h
python convert_icon.py icon/graphics.png $o/graphics.h
python convert_icon.py icon/layers.png $o/layers.h
python convert_icon.py icon/cubes.png $o/cubes.h
python convert_icon.py icon/house.png $o/house.h
