OpenCDU is a project to link a homemade CDU to the PMDG 737 running on Prepar 3D (v4).
It used to run as a 32bit app on previous versions as well as FSX but is now compiled
as a 64 bit app to work with the latest versions of P3D.

The homebuilt CDU uses a Rasperry Pi 3B and talks to the PC running the flight sim over
ethernet/wifi. The code for this is in the OpenCDU-rPi repository.

CDUServer64 talks to the sim via th SimConnect API & doesn't require any 3rd party
apps.

CDUTestClient is used to check the comms is working

It is configured to only work with the PMDG 737 aircraft but could easily be
modified to work with the 777, 747 or 737NGX by modifying the offsets referred to
in the appropriate PMDG SDK header file.

OpenCDU consists of two apps: CDUServer64 & CDUTestClient



Build Notes 
All code for P3D is now 64 bit.
It is recommended that apps are developed as standalone .EXEs rather than .dlls in order to prevent crashing
P3D due to app bugs.
Apps can communicate with P3D via the SimConnect API or the PDK - which is the P3D API. This app uses SimConnect.
Download the latest SimConnect lib & header from P3D website. As of writing, this is for P3D V4.
Read the Notes from the P3D SDK site about setting up Visual Studio. (Latest - Visual Studio 2019)

This app is basically run as a Win32 MFC Application but compiled to 64 bit.
Win32 is a bit of a misnomer as it is basically the Windows API. There is no Win64 API.

For some reason CDUServer only builds as a Release version - the Debug version treats some warnings as errors & fails - needs looking into.
