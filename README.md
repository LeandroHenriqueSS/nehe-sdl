
    $ brew install sdl sdl_image sdl_mixer sdl_ttf smpeg
    $ ./configure
    $ export CFLAGS="-framework OpenGL"
    $ make

-----

## Old README

    The text for the tutorials can be found at:
    http://nehe.gamedev.net/opengl.asp

    If you are compiling natively under Win32 using VC++, open VisualC.zip

    If you are cross-compiling to Win32 from Linux on a PC, get tools from:
    http://www.devolution.com/~slouken/SDL/Xmingw32/
    and pass configure the arguments "--host=i386-linux --target=i386-mingw32"

    If you are building under Linux, Solaris, or BeOS:

    Usage:

    ./configure
    make lessonX

    (where X is numeric)
    to build tutorials 1-n individually.

    Use:

    ./configure
    make all

    to build all tutorials at once.


    Credits:
    Jeff Molofee (nehe@home.com) for writing the tutorials.
    Alfred (alfred@mazuma.net.au) for various fixes and improvements.
    Richard Campbell (ulmont@bellsouth.net) for porting to MesaGL.

    -Sam Lantinga  (slouken@devolution.com)
