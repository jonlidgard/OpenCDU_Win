# OpenCDU
OpenCDU is a project to link a homemade CDU to the legacy [PMDG](https://pmdg.com) 737NG running on Prepar 3D (v4).
It used to run as a 32bit app on previous versions as well as FSX but is now compiled
as a 64 bit app to work with the latest versions of P3D.

The homebuilt CDU uses a Rasperry Pi 3B and talks to the PC running the flight sim over
ethernet/wifi. The code for this is in the [OpenCDU-rPi repository](https://github.com/jonlidgard/OpenCDU_rPi).

CDUServer64 talks to the sim via th SimConnect API & doesn't require any 3rd party
apps.

CDUTestClient is used to check the comms is working

It is configured to only work with the PMDG 737 aircraft but could easily be
modified to work with the 777, 747 or the new 737NGXU by modifying the offsets referred to
in the appropriate PMDG SDK header file.

OpenCDU consists of two apps: CDUServer64 & CDUTestClient

The code is heavily based on an example server written by Joseph M. Newcomer and reproduced here with kind permission.
Please visit his website for an excellent article on the correct way to write client/server code.
http://www.flounder.com/kb192570.htm

The PMDG SDK header files for the 737 are reproduced with kind permission from Precision Manuals.


### Build Notes
All code for P3D is now 64 bit.
Build the app using Visual Studio 2019 (Community Edition is free) as a Release x64 version.

It is recommended that apps are developed as standalone .EXEs rather than .dlls in order to prevent crashing
P3D due to app bugs. This app is developed as a .EXE
Apps can communicate with P3D via the SimConnect API or the PDK - which is the P3D API. This app uses SimConnect.
Read the Notes from the P3D SDK site about setting up Visual Studio. (Latest - Visual Studio 2019)

This app is a Win32 MFC Application, compiled to 64 bit.
Win32 is a bit of a misnomer as it is basically **the** Windows API. There is no Win64 API.

It may be necessary to update the SimConnect library & header - download the latest version 
from https://www.prepar3d.com/support/sdk/ & copy the updated files into SimConnect folder

### Configuring P3D
Copy the CDUServer64.exe file to your P3D Modules directory
Remember to edit the EXE.xml file to make sure the server is launched when FSX/P3D starts e.g.:
```
<Launch.Addon>
  <Name>CDUServer</Name>
  <Disabled>False</Disabled> 
  <Path>C:\P3Dv4\Modules\CDUServer64.exe</Path> 
 </Launch.Addon>
``` 
### Known Bugs
Multiple build warnings
For some reason CDUServer only builds as a Release version - the Debug version treats some warnings as errors & fails.
