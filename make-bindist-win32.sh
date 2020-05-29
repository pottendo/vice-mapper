#!/bin/sh

BUILDPATH=`pwd`/$1

echo "Generating a $WINXX GTK3 port binary distribution in $BUILDPATH..."
rm -rf ${BUILDPATH}
mkdir $BUILDPATH

cp *.exe gui.glade $BUILDPATH
strip $BUILDPATH/*.exe
cp `ntldd -R $BUILDPATH/mapper.exe|gawk '/\\\\bin\\\\/{print $3;}'|cygpath -f -` $BUILDPATH
cd $MINGW_PREFIX
cp bin/lib{croco-0.6-3,lzma-5,rsvg-2-2,xml2-2}.dll $BUILDPATH
cp --parents lib/gdk-pixbuf-2.0/2.*/loaders.cache lib/gdk-pixbuf-2.0/2.*/loaders/libpixbufloader-{png,svg,xpm}.dll $BUILDPATH
# GTK3 accepts having only scalable icons,
# which reduces the bindist size considerably.
cp --parents -a share/icons/Adwaita/{index.*,scalable} $BUILDPATH
rm -r $BUILDPATH/share/icons/Adwaita/scalable/emotes
cp --parents share/icons/hicolor/index.theme $BUILDPATH
cp --parents share/glib-2.0/schemas/gschemas.compiled $BUILDPATH
cp bin/gspawn-win??-helper*.exe $BUILDPATH
cd - >/dev/null

exit 0
