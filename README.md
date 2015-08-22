# LittleGPTracker
This is a fork of m-.-n's LGPT for Symbian (S60v3) devices. Built and tested on a Nokia E63 (S60v3 FP1).

At present, everything works - except the keyboard becomes unresponsive when audio is running. To be fixed.

## Runtime dependencies
You need the Open C/C++ libraries installed; these come with the Qt installer, amongst other places. The separate SIS files are also available in the Open C/C++ plugin (see below).

## Compile dependencies
You need:

- an S60 SDK matching your phone (v3, v3 FP1, v3 FP2) - these are not backwards compatible but somewhat forwards
- a suitable version of the GCCE compiler
- the Open C/C++ plug-in
- SDL-S60v3 from here: http://repo.or.cz/w/SDL.s60v3.git

Unfortunately the Nokia site with the SDKs and tools is now gone. Most of the necessary files can be found here:
http://n8delight.blogspot.com.au/2014/05/for-developers-re-up-of-symbian.html
I also found a copy of Open C/C++ on 4shared (look for s60_open_c_cpp_plug_in_v1_7_en).
The Qt SDK 1.1.x includes compiler, SDK and plug-ins all together, which are suitable for FP2 and above. This can be found here:
ftp://ftp.informatik.hu-berlin.de/pub/Mirrors/ftp.troll.no/QT/qtsdk/

To build:
- Install the SDK, which will add it to your PATH or give you a startup link to do so (QtSDK does this).
- Install the compiler, put its bin directory on your PATH
- Install Open C/C++ over the SDK
- `cd SDL-s60v3\symbian`; `bldmake bldfiles`; `abld build gcce`
  This installs the resulting files *into the SDK tree* (eek).
- cd LittleGPTracker\projects; make
  Assuming the platform is set to S60, this should give you `lgpt.sis` which you can install.

### Notes
There is a "make run" target which will use `runonphone.exe` (available in the QtSDK, amongst other places). You need the App TRK installed and running on your phone.

I had to patch a few bits of the FP1 SDK to get the compiler to work!
Notably, I had to replace macros defining `va_list`, `va_start`, `va_end` etc. using `__builtin_va_{list,start,end}`.

If you use a newer SDK than your phone you might get lucky, or you might not. The FP2/QtSDK built the project nicely, but it refused to run on my phone with "Feature not supported".

There's a debugger `fdb` in the FShell project at http://fshell.sf.net which can give you stack traces; use `fdb -m e:\` to point it at `lgpt.exe.map` with all the symbols.
