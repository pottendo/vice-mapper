#!/bin/bash
# be careful, this is heavily depending in pottendo's development machine

BUILDPATH=`pwd`/$1
CROSS=$2
COMPILER=$3
SRCDIR=$4
OBJDUMP=objdump
cwd=`pwd`
MINGW_PREFIX=/mingw64

echo "Generating a $WINXX GTK3 port binary distribution in $BUILDPATH..."
rm -rf ${BUILDPATH}
mkdir $BUILDPATH

cp *.exe ${SRCDIR}/gui.glade $BUILDPATH
strip $BUILDPATH/*.exe

get_dll_deps()
{
  for j in $BUILDPATH/*.dll; do
    dlls=`$OBJDUMP -p $j | gawk '/^\\tDLL N/{print $3;}'`
    for i in $dlls
    do test -e $dlldir/$i&&cp -u $dlldir/$i $BUILDPATH
    done
  done
}

if test x"${CROSS}" != "xtrue" ; then
    # msys2 build
    cp `ntldd -R $BUILDPATH/mapper.exe|gawk '/\\\\bin\\\\/{print $3;}'|cygpath -f -` $BUILDPATH
    cd $MINGW_PREFIX
    cp bin/lib{lzma-5,rsvg-2-2,xml2-2}.dll $BUILDPATH
    cp --parents lib/gdk-pixbuf-2.0/2.*/loaders.cache lib/gdk-pixbuf-2.0/2.*/loaders/libpixbufloader-{png,svg,xpm}.dll $BUILDPATH
    # GTK3 accepts having only scalable icons,
    # which reduces the bindist size considerably.
    cp --parents -a share/icons/Adwaita/{index.*,scalable} $BUILDPATH
    rm -r $BUILDPATH/share/icons/Adwaita/scalable/emotes
    cp --parents share/icons/hicolor/index.theme $BUILDPATH
    cp --parents share/glib-2.0/schemas/gschemas.compiled $BUILDPATH
    cp bin/gspawn-win??-helper*.exe $BUILDPATH
    cp bin/gdbus.exe $BUILDPATH
    cd - >/dev/null
else
    # cross linux->win32 build
    #libm=`$COMPILER -print-file-name=libm.a`
    #echo "libm.a = '$libm'"
    #location=`dirname $libm`
    #loc=`dirname $location`
    loc=${MINGW_PREFIX}
    echo "loc = $loc"
    if test -d "$loc/dll" ; then
	dlldir="$loc/dll"
    else
	dlldir="$loc/bin"
    fi
    dlls=`$OBJDUMP -p mapper.exe | gawk '/^\\tDLL N/{print $3;}'`
    for i in $dlls ; do
	test -e $dlldir/$i&&cp $dlldir/$i $BUILDPATH
    done
    # A few of these libs cannot be found by frankenvice, so perhaps we need to install
    # these or alter this command:
    #cp $dlldir/lib{bz2-1,freetype-6,croco-0.6-3,rsvg-2-2,xml2-2}.dll $BUILDPATH
    cp $dlldir/{zlib1,libwinpthread-1,liblzma-5,librsvg-2-2,libxml2-2}.dll $BUILDPATH
    cp $dlldir/gdbus.exe $BUILDPATH
    gccname=`$COMPILER -print-file-name=libgcc.a`
    gccdir=`dirname $gccname`
    dlls=`find $gccdir -name 'libgcc*.dll' -o -name 'libstdc*.dll'`
    test -n "$dlls"&&cp $dlls $BUILDPATH
    get_dll_deps
    get_dll_deps
    get_dll_deps
    current=`pwd`
    cd $loc
    cp --parents lib/gdk-pixbuf-2.0/2.*/loaders.cache lib/gdk-pixbuf-2.0/2.*/loaders/libpixbufloader-{png,svg,xpm}.dll $BUILDPATH
    # update loaders.cache
    cp --parents -a share/icons/Adwaita/{index.*,scalable} $BUILDPATH
    cp --parents -a share/icons/hicolor/index.theme $BUILDPATH
    cp --parents share/glib-2.0/schemas/gschemas.compiled $BUILDPATH
    cp bin/gspawn-win??-helper*.exe $BUILDPATH
    cd $cwd
fi

exit 0
