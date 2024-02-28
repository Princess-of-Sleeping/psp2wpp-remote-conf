# psp2wpp-remote-conf

A tool for configuring waveparam on Windows by connecting to PS Vita via USB.

# Install

Download the latest release page. Just unzip it afterwards.

# Usage

0. To use this tool, you will also need a separate [psp2waveviewer](https://github.com/Princess-of-Sleeping/psp2waveviewer/) application.

- See the [Feature section of the README](https://github.com/Princess-of-Sleeping/psp2waveviewer/?tab=readme-ov-file#feature) for psp2wpp remote configuration mode of psp2waveviewer.

1. If you are using plugins that use USB on your PS Vita, you will need to temporarily disable them. like udcd-uvc.skprx or PSMLogUSB.skprx

2. After starting psp2wpp-remote-conf.exe, connect PS Vita and PC using USB.

3. Start psp2waveviewer in psp2wpp remote configuration mode on PS Vita.

- It doesn't matter which order 2 or 3 comes first.

4. Enjoy it :)

- If the connection between PS Vita and PC is unstable due to `Bad sequence`, disconnect the USB and connect again.


Note : The saved waveparam.bin can be used as is with the [psp2wpp plugin](https://github.com/Princess-of-Sleeping/psp2wpp).

# USB Driver for PC

If psp2wpp-remote-conf does not recognize the PS Vita and PC connection via USB, drivers such as libusb may not be installed.

In that case, install the driver using your preferred method.

The PS Vita will freeze completely on you installing the driver, but please do not turn off the PS Vita until the driver installation is complete on your PC.

Once the driver installation is complete, press and hold the power button on your PS Vita to force it to restart.

# Credits

[Paddel06](https://github.com/Paddel06) for Beta testing.

[Cat](https://github.com/isage) And [CreepNT](https://github.com/CreepNT) for USB helping.

[Dear ImGui](https://github.com/ocornut/imgui) for Settings GUI.
