#pragma once

#include "PMDG_NGX_SDK.h"
//#include "SimCommThread.h"
#include "PMDG_NGX_SDK.h"
//#include "PMDG_777X_SDK.h" // Don't include this - it overwrites the 737 #pragma warning( push )
#pragma warning( disable : 4245)
#pragma warning( disable : 2220) // using DWORD for a negative value
#include "SimConnect.h"
#pragma warning( pop ) 

#include <map>
#include <string>
#include <iterator>

static enum DATA_REQUEST_ID {
	DATA_REQUEST,
	CDU_REQUEST,
	CONTROL_REQUEST,
	AIR_PATH_REQUEST
};

static enum EVENT_ID {
	EVENT_SIM_START,	// used to track the loaded aircraft

	EVENT_CDU_L_EXEC_LIGHT,
	EVENT_CDU_L_MSG_LIGHT,
};


#define PMDG737_CDU_L_OFFSET 0
#define PMDG737_CDU_R_OFFSET EVT_CDU_R_L1 - EVT_CDU_L_L1
#define PMDG777_CDU_L_OFFSET THIRD_PARTY_EVENT_ID_MIN + 328 - EVT_CDU_L_L1 
#define PMDG777_CDU_C_OFFSET THIRD_PARTY_EVENT_ID_MIN + 653 - EVT_CDU_L_L1 
#define PMDG777_CDU_R_OFFSET THIRD_PARTY_EVENT_ID_MIN + 401 - EVT_CDU_L_L1

//#include "StdAfx.h"

//void CALLBACK xDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);

class SimComm
{
public:
	SimComm();
	virtual ~SimComm();
	int OpenConnection();
	int CloseConnection();

//	CSimCommThread *pSimCommThread;
	CWinThread *pSimCommListenerThread;
	void SetSendThreadID(DWORD id) { m_sendThreadID = id; }
	DWORD GetSendThreadID() { return m_sendThreadID; }
	void SetTarget(CWnd * w) { target = w; }
	CWnd * GetTarget() { return target; }
	HANDLE GetSimConnectHandle() { return hSimConnect; }
	BOOL GetThreadRunState() { return threadRunState; }
	//	void sendToSimFromNetwork(WPARAM, LPARAM);
	void SendToSimFromNetwork(CString s);
	void Dispatch();
	void SetCduOffset(int index, int offset) { offsets[index] = offset; ResetAnnunciators(); }
	int GetCduOffset(int index) { return offsets[index]; }
	void SetPollTimeMs(int value) { pollTimeMs = value > 0 && value < 1001? value : 50; }
	int GetPollTimeMs() { return pollTimeMs; }
	void SetSendFlags(int value) { sendEnableFlags = value & 3; }
	int GetSendFlags() { return sendEnableFlags; }
	//	void SetLogSends(BOOL value) { logSends = value; }

protected:
	static void CALLBACK DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);

private:
	CMutex m_mutex;
	int sendEnableFlags; // bit 0 annun lights, bit 1 cdu screen data
	int pollTimeMs;
	CWnd * target;
	DWORD m_threadID;
	DWORD m_sendThreadID;
	int offsets[8];
	HANDLE  hSimConnect;
	DWORD offsetIndex;
	int GetCduOffset() { return offsets[offsetIndex]; }
	BOOL threadRunState;
	BOOL quitting;
	PMDG_NGX_Control Control;
//	DWORD cduOffset; // holds an offset for controlling left or right cdu
	DWORD offset777L; // holds an offset for the
	std::map<std::string, DWORD>keymap;
	bool AircraftRunning;
	bool CDU_annunEXEC[3];
	bool CDU_annunMSG[3];
	bool CDU_annunCALL[3];
	bool CDU_annunFAIL[3];
	bool CDU_annunOFST[3];
	bool CDU_annunDSPY[3];


	void SetupCommunication();
	void ResetAnnunciators();
	void Process(SIMCONNECT_RECV *pData, DWORD cbData);
	void ProcessCDUData(PMDG_NGX_CDU_Screen *pS);
	void ProcessNGXData(PMDG_NGX_Data *pS);
	void SendBytesToNetwork(CByteArray *msg);
	void SendStringToNetwork(CString s);
	void sendEventToPMDG(DWORD event);
	void genMap(void);
	DWORD translateCode(CString code);

};

