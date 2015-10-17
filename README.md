MachinTruc is licensed under the GNU GPL v2.
Author : Christophe Thommeret <hftom@free.fr>

MachinTruc is a non linear video editor.

![Screenshot](http://hftom.fr/machintruc-sshot.jpg)

Deps:
- ffmpeg (libavformat, libavcodec, libavutil, libavfilter, libswresample, libswscale)
- vid.stab (https://github.com/georgmartius/vid.stab)
	I strongly recommend to apply this patch : http://hftom.fr/vidstab.memleak-fix.diff
- SDL (sound output)
- Movit (http://git.sesse.net/?p=movit)
- Qt 5
- X11

Compilation:
- mkdir build
- cd build
- qmake ..
- make

Run:
- ./machintruc
