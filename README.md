# vice-mapper
Little GUI to manage maps out of vice screenshots

# build vice
../vice-emu-code/vice/configure -C --enable-native-tools --enable-native-gtk3ui --host=mingw32-gtk3 --enable-x64 2>&1 |tee mycfg-64.log
export MINGW_PREFIX=/mingw32
make -j12 2>&1 | tee make-32.log
make bindist

copy .DLLs for mapper - check with ntldd mapper.exe
