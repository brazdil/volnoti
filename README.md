Volnoti
=========

Introduction
------------

Volnoti is a lightweight volume notification daemon for GNU/Linux and
other POSIX operating systems. It is based on GTK+ and D-Bus and should
work with any sensible window manager. The original aim was to create
a volume notification daemon for lightweight window managers like LXDE 
or XMonad. It is known to work with a wide range of WMs, including 
GNOME, KDE, Xfce, LXDE, XMonad, i3 and many others.

Dependencies
------------

You need the following libraries to compile CryptoSMS yourself. Please 
install them through the package manager of your distribution, or follow
installation instructions on the projects' websites.

 - [Android SDK](http://developer.android.com) 
   with Android 2.3.3 (API 9) SDK Platform
 - [Apache Ant](http://ant.apache.org/)
 - [Leiningen](https://github.com/technomancy/leiningen)

Compilation from Git
--------------------

Start by downloading the source code from GitHub:

    $ git clone git://github.com/davidbrazdil/volnoti.git
    $ cd volnoti

Let Autotools create the configuration scripts:

    $ ./prepare.sh
    
Then just follow the basic GNU routine:

    $ ./configure --prefix=/usr
    $ make
    $ sudo make install
    
You can have the `.tar.gz` source archive prepared simply by calling 
a provided script:

    $ ./package.sh

Compilation from source archive
-------------------------------

Download the `.tar.gz` source archive from the GitHub page, and then
extract its contents by calling:

    $ tar xvzf volnoti-\*.tar.gz

Then just follow the basic GNU routine:

    $ ./configure --prefix=/usr
    $ make
    $ sudo make install

Running the application
-----------------------

Theming
-------

Credits
-------

