x="wayland-client cairo pangocairo dbus-1 libcurl"
a="`pkg-config --cflags --libs $x`"
i="-Icode"
i="$i code/*.c"
i="$i code/draw/*.c"
i="$i code/draw/common/*.c"
i="$i code/input/*.c"
i="$i code/utils/*.c"
i="$i code/wayland/*.c"
i="$i code/wayland/protocol/*.c"

mkdir build

gcc $a -lm -std=c17 -D_GNU_SOURCE -g -o build/run $i
