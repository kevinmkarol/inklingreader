Inkling OSC for Mac
=================
This fork of roelj/inklingreader extends the online_mode functionality to broadcast the inkling data over a network via OSC.  This new network broadcast mode has been tested for both Raspberry Pi and Mac.  This will allow other programms to recieve Inkling data in real time and it used however they wish.

Network mode can be invoked using:<br>
./inklingreader -n ipAddress:port<br>
where ipAddress is a full ipAddress (127.0.0.1 for localhost) and port is the port number the OSC reveiver will be listening on.

Additionally, to facilitate less technical users a file called BEGINNER\_SETUP\_GUIDE.txt has been added which lists what I believe are currently all of the dependencies for the project (as of 2/14/16).  While this will not be updated, it should get users reasonably close to getting the project up and running

-Kevin M. Karol

This project also relies on these wonderfull OSC and USB HID libraries:<br>
https://github.com/mhroth/tinyosc.git<br>
https://github.com/signal11/hidapi.git


(Documentation for roelj/inklingreader begins here)

GNU InklingReader
=================

This GNU project is an attempt to create a GNU/Linux-friendly version of the
Wacom Inkling SketchManager. Instead of integrating with proprietary programs,
this program aims to be compatible with free software alternatives like Inkscape.

The program can:

* Display WPI files
* Merge WPI files (command-line only)
* Export to Inkscape SVG (preserving layers), PDF, PNG and JSON
* Convert all WPI files in a directory to SVG
* Automatically assign colors upon pressing the "new layer" multiple times
* Give control over stroke pressure
* Export timing information in JSON
* Interact with the timeline of your drawing
* Use the Wacom Inkling as a mouse device

Integration with Inkscape
-------------------------

There's an Inkscape extension available that allows you to open WPI files with 
Inkscape "directly". The extension uses InklingReader to convert the WPI format 
to SVG in the background.

Installing the extension is quite easy. Copy the files in res/inkscape-extension
to your Inkscape extension directory. On GNU/Linux that is:
<pre>~/.config/inkscape/extensions/</pre>

Online-mode
-----------

The online-mode feature (use the Wacom Inkling as a mouse device) is in a rough
shape at the moment. It only works for GNU/Linux because it uses the uinput
kernel module. To enable it, you need super user rights (root) to be able to
activate the online-mode of the device and to create a virtual mouse device.

Enabling it can be done with the following commands:
<pre>
su -                # Become a super user
modprobe uinput     # Make sure the uinput kernel module is loaded.
./inklingreader -j  # Start the program with the option to enable online-mode.
</pre>

Screenshot
----------

![InklingReader screenshot](http://roelj.com/Inkling_5.png)

Dependencies
------------

Make sure you have the following libraries, development packages and build 
tools installed:

* GCC or CLANG
* Automake
* Autoconf
* Make
* GTK+ (3.10 or later)
* librsvg-2.0
* libusb 1.0


Build instructions
------------------
When you have resolved the dependencies listed above you can build 
the program by running:
<pre>
autoreconf -i
./configure
make
</pre>

To compile with CLANG:
<pre>
autoreconf -i
./configure CC=clang
make
</pre>

Additionally you can add compiler flags:
<pre>
autoreconf -i
./configure CFLAGS="-Wall -O2 -march=native"
make
</pre>

Optionally you can generate developer documentation using Doxygen.
<pre>
cd doc/
doxygen
</pre>

Tested distributions
--------------------

The software is known to run on:

* Debian 7.x
* CrunchBang Linux 11
* Fedora 20 or later
* Mageia 4
* Ubuntu 14.04 or later
* Mac OS X 10.7.5, clang (based on LLVM 3.1svn)
* Windows 8.1 (with MinGW and GTK 32-bit all-in-one bundle)

Since the program uses only widely available libraries it should run OK 
on almost every GNU/Linux distribution. If you have problems getting it
to work on yours, let us know.

Contributing
------------

InklingReader can use help! If you're interested in helping, here's a list 
of things you could do:

* Package InklingReader for any distribution
* Request and/or implement features

Contact
-------

You can join the [bug-inklingreader -at- gnu.org](https://lists.gnu.org/mailman/listinfo/bug-inklingreader)
mailing list to report bugs.

For general discussion, use the
[inklingreader-discuss -at- gnu.org](https://lists.gnu.org/mailman/listinfo/inklingreader-discuss)
mailing list.

License
-------

This project's code and the graphics in the "res/" folder are licensed under
the GPLv3 or any later version.
