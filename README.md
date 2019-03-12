1. First You must clone **with submodules**<br>
git clone --recurse-submodules https://github.com/borneq/qedytor.git<br>
because it uses:<br>
https://github.com/borneq/hisyntax<br>
https://github.com/borneq/DCPCryptCpp<br>
2. Next go to DCPCryptCpp directory, create "build" directory and go to this build directory.<br>
Command [cmake .. ] in Linux will create makefile, but in Windows this will create Visual Studio 
project if is any VS installed. <br>
But we tested with MinGW in Windows, therefore need call [cmake .. -G "MinGW Makefiles"]<br>
first add gcc to path, for example d:\Qt\Qt5.12.0\Tools\mingw730_64\bin\ directory.
next call make , for Linux is enough type "make".<br>
3.libdcpcrypt.a move/copy from build directory to bin, after creating "bin" in qeditor.<br>
4.Go to hisyntax directory and run Qt Creator with qedytor\hisyntax\src\hisyntax.pro 
config it (in Windows was tested with MinGW) build release or debug version.<br>
5.Copy qedytor\hisyntax\build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Release\release\libhisyntax.a 
or qedytor\hisyntax\build-hisyntax-Desktop_Qt_5_12_0_MinGW_64_bit-Debug\debug\libhisyntax.a to bin <br>
6.Open main project qedytor\src\qedytor.pro and build it. <br>
7.Enjoy with builded QEdytor inside Qt Creator IDE.<br>

Installation:
1. Move/copy qedytor.exe (or qedytor in Linux) from qedytor\build-qedytor-Desktop_Qt_5_12_0_MinGW_64_bit-Release\release\ or 
qedytor\build-qedytor-Desktop_Qt_5_12_0_MinGW_64_bit-Debug\debug\
to destination place, for example d:\qteditor\ or another in Linux.<br>
2. In Windows exe will small (about 420 KB) but need some DLL's:<br>
Qt5Core.dll<br>
Qt5Gui.dll<br>
Qt5Widgets.dll<br>
Qt5Network.dll<br>
libstdc++-6.dll<br> 
libgcc_s_seh-1.dll<br>
libwinpthread-1.dll<br> 
(preferred smaller one with no debug information)<br>
3.Need config QEdytor.json in c:\Users\USER\.config\ in Windows, or /home/USER/.config in Linux.<br>
Example config is in qedytor\QEdytor.json <br>
Make directory with syntax and themes and set it in config file by menu: Tools->Config<br>

Features:<br>
Alt+1, Alt+2..Alt+0 - choosing tab <br>
wordWrap,line numbers - from context menu (right mouse) <br>