
if [ ! -d psp2wpp-remote-conf ]; then
  mkdir psp2wpp-remote-conf
  cp ./build/psp2wpp_remote_conf.exe ./psp2wpp-remote-conf
  cp ./resource/libgcc_s_seh-1.dll ./psp2wpp-remote-conf
  cp ./resource/libstdc++-6.dll ./psp2wpp-remote-conf
  cp ./resource/libwinpthread-1.dll ./psp2wpp-remote-conf
  cp ./resource/font.ttf ./psp2wpp-remote-conf
  cp ./resource/waveparam.bin ./psp2wpp-remote-conf
  cp ./resource/imgui.ini ./psp2wpp-remote-conf
fi
