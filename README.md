#### MachinTruc is a GPU accelerated non linear video editor.


![Screenshot](http://hftom.fr/machintruc-2.jpg)



Deps:
- ffmpeg (libavformat, libavcodec, libavutil, libavfilter, libswresample, libswscale)
- libexif
- OpenMP
- SDL2 (sound output)
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


MachinTruc is licensed under the GNU GPL v2.

Author : Christophe Thommeret <hftom@free.fr>