General Notes

All code for P3D is now 64 bit.
It is recommended that apps are developed as standalone .EXEs rather than .dlls in order to prevent crashing
P3D due to app bugs.
Apps can communicate with P3D via the SimConnect API or the PDK - which is the P3D API. This app uses SimConnect.
Download the latest SimConnect lib & header from P3D website. As of writing, this is for P3D V4.
Read the Notes from the P3D SDK site about setting up Visual Studio. (Latest - Visual Studio 2019)

This app is basically run as a Win32 MFC Application but compiled to 64 bit.
Win32 is a bit of a misnomer as it is basically the Windows API. There is no Win64 API.
