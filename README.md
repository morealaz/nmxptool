# nmxptool: open-source and cross-platform software for Nanometrics seismic data acquisition

Matteo Quintiliani - Istituto Nazionale di Geofisica e Vulcanologia - Italy
Mail bug reports and suggestions to <matteo.quintiliani@ingv.it> 

Website: http://mednet.rm.ingv.it/nmxptool.php

nmxptool and libnmxp have been developed for interacting with NaqsServer and DataServer of Nanometrics Inc.

### INTRODUCTION

The Nanometrics servers NaqsServer and DataServer can provide on-line access
to seismic data and state-of-health information accepting TCP/IP connections
and forwarding the requested data to each client program.  NaqsServer collates
and stores in near-real-time incoming data in ringbuffers whereas DataServer
provides data of the past stored in NaqsServer ringbuffers.  In order to
implement the Nanometrics subscription protocols, the author developed a
software consisting of a library called libnmxp and a tool called nmxptool. The
library exposes a set of documented APIs which allow to communicate with the
Nanometrics servers. The tool, based on libnmxp, allows to retrieve or monitor
real-time data and data of the past.

nmxptool can be used in three different ways:

  * stand-alone to monitor data or save retrieved data in mini-seed records
  * launched as an Earthworm module to redirect data into the EW-rings
  * like a Seed-Link plug-in to feed the SL-server
	

The main contribute, as regards other similar software, is the capability to
manage Raw Stream connections by buffering and sorting all received packets,
included the retransmitted ones, guaranteeing a good compromise between data
continuity and low latency. Besides, nmxptool allows to retrieve Data-On-Demand
with minimum delay after request. Software is open-source and released under
GNU Library General Public License. It has been written in C language using the
GNU Build Tools (automake, autoconf and configure script) and taking in account
the cross-platform compilation aspects, in fact, it can run on almost all the
Unix-like operating systems, Mac OS X, Windows and either 32-bit or 64-bit
architectures.

### DEPENDENCIES

Optional libraries:

  * libmseed-2.3 or later: <http://www.iris.edu/software/libraries/>
  
    The Mini-SEED library. A C library framework for manipulating and
    managing SEED data records.  Author: Chad Trabant, IRIS DMC

  * Earthworm 6.2 or later: <http://www.isti2.com/ew/>

  * Seedlink, SeisComP - <http://www.gfz-potsdam.de/geofon/seiscomp/>

    Seedlink is a system for near real time seismic data distribution.

  * POSIX Threads library (pthread)
  
    On Windows, nmxptool has been compiled with "Pthreads-w32 release 2.8.0"
    
    <http://sourceware.org/pthreads-win32/>

  Read section INSTALLATION below.


### QUICK INSTALLATION

	tar xvfz nmxptool-X.X.X.tar.gz
	
	cd nmxptool-X.X.X
	./configure [ --prefix=... ] [ SEISCOMPDIR=... ]
	make
	src/nmxptool --version
	
	make install
	     OR
	make install-ew-bin
	make install-ew-conf (ONLY THE FIRST TIME)
	     OR
	make install-seiscomp2-bin
	make install-seiscomp2-templates (ONLY THE FIRST TIME)
	     OR
	make install-seiscomp3-bin
	make install-seiscomp3-templates (ONLY THE FIRST TIME)
	     OR
	cp src/nmxptool <where_you_want>
	
	nmxptool --help

For binary installation see section INSTALLATION BINARIES below.


### INSTALLATION

nmxptool and libnmxp have been developed using GNU Build Tools
(automake, autoconf and configure script) taking in account the
POSIX Cross-Platform aspects. So you should be able to compile
and install them everywhere you can launch the following commands:

	./configure --enable-FEATURE1 ... VAR1=... VAR2=...
	make
	make install

Options for nmxptool 'configure' script:

Disabling optional Features

	--disable-libmseed      disable saving data in mini-SEED records
	--disable-ew            do not compile nmxptool as Earthworm module
	--disable-seedlink      do not compile nmxptool as Seedlink plug-in

  Some influential environment variables:

	EW_HOME     Earthworm home directory
	EW_VERSION  Earthworm version directory
	EW_PARAMS   Earthworm configuration files directory
	GLOBALFLAGS C compiler flags for Earthworm

'configure' adds -I${EW_HOME}/${EW_VERSION}/include to CFLAGS

Examples:

  - 'configure' tries to compile all features:
```
./configure	CFLAGS="-O2 -Wall -pipe -I/<anywhere>/libmseed" \
  				LDFLAGS="-L/<anywhere>/libmseed"
```

#### Enable only Earthworm feature and set related variables:
```
./configure	--disable-libmseed --disable-seedlink \
					CFLAGS="-O2 -Wall -pipe" \
					EW_HOME="/home/ew" \
					EW_VERSION="v7.2" \
					EW_PARAMS="${EW_HOME}/${EW_VERSION}/params" \
					GLOBALFLAGS="-D_SPARC -D_SOLARIS"
```
or, for example
```
GLOBALFLAGS="-m32 -Dlinux -D__i386 -D_LINUX -D_INTEL -D_USE_SCHED -D_USE_PTHREADS -D_USE_TERMIOS"
GLOBALFLAGS="-D_MACOSX -D_INTEL -D_USE_PTHREADS -D_USE_SCHED "
GLOBALFLAGS="-D_WINNT -D_INTEL -D_CRT_SECURE_NO_DEPRECATE -D_USE_32BIT_TIME_T"
```

Use the last one to compile Earthworm feature under Windows-MinGW.

#### Enable only libmseed and seedlink:
```
./configure	--disable-ew \
					CFLAGS="-O2 -Wall -pipe -I/<anywhere>/libmseed" \
					LDFLAGS="-L/<anywhere>/libmseed"
```

#### Verify the version and enabled features after compilation:

```
./src/nmxptool --version
```

```
	nmxptool 2.1.5, tool for Nanometrics Protocols
	         Private Data Stream 1.4, Data Access Protocol 1.0
	         Enabled features: libmseed YES, SeedLink YES, Earthworm YES.
	         Using pthread: YES.
```

#### libmseed, The Mini-SEED library - <http://www.iris.edu/manuals/>

    If available within include and library path,
         this library allows to save retrieved data in Mini-SEED records.
         You might add to CFLAGS this `"-I/<anywhere>/libmseed"`
         and to LDFLAGS this `"-L/<anywhere>/libmseed"`,
         do not forget to run 'ranlib libmseed.a' or similars.

#### ew, Earthworm System - http://www.isti2.com/ew/

    nmxptool is included into the official Earthworm distribution since the version 7.2.
         Anyway, you can compile nmxptool outside the EW distribution, for example, to upgrade.
         Before launching 'configure', run the appropriate script from directory 'environment', the compilation depends on the following environment variables:
    
    - $EW_HOME
    - $EW_VERSION
    - $EW_PARAMS
    - $GLOBALFLAGS
    
    'configure' looks for necessary Earthworm object files inside `$EW_HOME/$EW_VERSION/lib`
         and link them to nmxptool in order to enable Earthworm module feature.
         If some of object files are missing then it will attempt to discover sources
     dependents on your operating system and it will compile them before linking.

Install binary and configuration files for Earthworm:
```
make install-ew-bin
make install-ew-conf (ONLY THE FIRST TIME)
```

The first command copies nmxptool binary in $EW_HOME/$EW_VERSION/bin
The second command copies earthworm/nmxptool.d, earthworm/nmxptool.desc in $EW_PARAMS.

If you are upgrading nmxptool then you might do that:

```
make install-ew-doc
```

Last command copies nmxptool_ovr.html and nmxptool_cmd.html into the directories `$EW_HOME/$EW_VERSION/ewdoc/WEB_DOC/ovr` and `$EW_HOME/$EW_VERSION/ewdoc/WEB_DOC/cmd`.

#### seedlink, SeisComP 2.5 and 3 - <http://www.gfz-potsdam.de/geofon/seiscomp/>

Seedlink is a system for near real time seismic data distribution.
Inside the directory 'src' has been copied files 'seedlink_plugin.c' and 'seedlink_plugin.h' from 'plugin.c' and 'plugin.h'belonging to the SeisComP distribution.

If your SeisComP root directory is not equal to `/home/sysop/seiscomp`,
you have to launch the script 'configure' and define `SEISCOMPDIR` in the following way:

```
./configure ... ... SEISCOMPDIR=/<where>/<seiscompdir>/<is>
```

**Install binary and template files for Seiscomp 3**

```
make install-seiscomp3-bin
make install-seiscomp3-templates (ONLY THE FIRST TIME)
```

The first command copies nmxptool binary to `${SEISCOMPDIR}/share/plugins/seedlink/`.

The second command copies the directory `seiscomp3_templates/nmxp` to `${SEISCOMPDIR}/share/templates/seedlink/`.

After, you must run:

````
seiscomp update-config
```

**Install binary and template files for SeisComp 2.5**
```
make install-seiscomp2-bin
make install-seiscomp2-templates (ONLY THE FIRST TIME)
```

The first command copies nmxptool binary to `${SEISCOMPDIR}/acquisition/bin`.

The second command copies the directories 
```
seiscomp2_templates/135_nmxptool
seiscomp2_templates/136_nmxptool_dod
```
to `${SEISCOMPDIR}/acquisition/templates/source/`.

After, you must run:
```
seiscomp config
```

### INSTALLATION BINARIES

Download the binary distribution for your operating system from the
web site <http://mednet.rm.ingv.it>, unpack it and copy the files where you want.


### SUPPORTED PLATFORMS AND 64-Bit ISSUES

nmxptool has been written in C language using the GNU Build Tools
   (automake, autoconf and configure script) and taking in account
   the cross-platform compilation aspects, in fact, it can run on
   almost all the Unix-like operating systems, Mac OS X, Windows
   and either 32-bit or 64-bit architectures.

nmxptool has been successfully compiled and tested on the following operating systems and architectures:

       -------------------------------------------------
      |          |  Intel    Intel    PowerPC   SPARC   |
      |          | 32-bit   64-bit    32-bit   64-bit   |
      |-------------------------------------------------|
      | Linux    |    X        X                        |
      | Solaris  |    X        X                  X     |
      | Mac OS X |             X         X              |
      | FreeBSD  |    X                                 |
      | Windows  |    X                                 |
       -------------------------------------------------

N.B. No test has been done on Earthworm when nmxptool is compiled with 64-bit option (-m64)


### NAQSSERVER CONFIGURATION

  - Data continuity when short disconnections to NaqsServer occur

    Inside the section Datastream of the file Naqs.ini set DataBufferLength to a high value.

		[ Datastream ] 
		Port = 28000              // TCP port for control/data connections to Naqs 
		Password = none           // access password (not used in version 1.3) 
		MaxConnections = 10       // maximum number of simultaneous connections 
		SocketType = Direct       // connection type (Direct or Callback) 
		DataBufferLength = 100    // Buffer length for data channels (# packets)

  - Packet retransmission

    Inside the section NetworkInterface of the file Naqs.ini enable RetxRequest.
    If RetxRequest is not enabled then MaxTolerableLatency is ineffective.

		[ NetworkInterface ] 
		Port = 32000           // UDP port for incoming NMX data (usually 32000) 
		SendDelay = 250        // milliseconds to delay after each send 
		RetxRequest = Enabled
		MulticastGroup = 224.1.1.1 


### DOCUMENTATION

  - Print nmxptool help:

		nmxptool --help

  - Earthworm module: earthworm/nmxptool_cmd.html nmxptool_ovr.html


  - SIGNAL HANDLING

    You can send the following signals to nmxptool:

		Signals INT QUIT TERM : Sending these signals to nmxptool causes it
		                        to immediately attempt to gracefully terminate. 
		                        It may take several seconds to complete exiting.
		
		Signal  ALRM          : Print current info about Raw Stream buffer.
		
		Signal  USR1          : Force to close a connection and open again
		                        without quitting the program. Only for connection
		                        in near real-time to NaqsServer.
		
		Signals HUP PIPE      : Ignored. (SIG_IGN)


### HISTORY

Read file HISTORY for release notes.


### ACKNOWLEDGEMENT

In no particular order:

  - Stefano Pintore - INGV (Istituto Nazionale di Geofisica e Vulcanologia)
  - Salvatore Mazza - INGV
  - Marco Olivieri - INGV
  - Luigi Falco - INGV
  - Paul Friberg - ISTI (Instrumental Software Technologies, Inc.)
  - Marian Jusko
  - Efthimios Sokos - (University of Patras - Greece)
  - Roman Racine - (ETH Zurich - Swiss Seismological Service)
  - Jean-Marie Saurel - (Institut de Physique du Globe de Paris)
  - Chad Trabant - IRIS (Incorporated Research Institutions for Seismology)
  - Sandy Stromme - IRIS


### LICENSE

Software is open-source and released under GNU Library General Public License.

Read file COPYING for details.

