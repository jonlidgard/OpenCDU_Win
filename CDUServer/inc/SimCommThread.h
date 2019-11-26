
#pragma once

#include "PMDG_NGX_SDK.h"
#include "SimConnect.h"
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


class CSimCommThread : public CWinThread {
protected:
	DECLARE_DYNCREATE(CSimCommThread)
public:
	CSimCommThread();
	virtual ~CSimCommThread();

	void SetupCommunication();

	void SetTarget(CWnd * w) { target = w; }
protected:
	static void CALLBACK DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
	void Process(SIMCONNECT_RECV* pData, DWORD cbData);


	// Target of messages which are posted
	CWnd * target;
	HANDLE  hSimConnect;
	virtual BOOL InitInstance();
	virtual int ExitInstance();
//	virtual int Run();
	virtual BOOL OnIdle(LONG lClount);

	int     quit;
	bool    AircraftRunning;
	bool NGX_CDU0ExecLight;
	bool NGX_CDU0MsgLight;

	PMDG_NGX_Control Control;
	DWORD cduOffset; // holds an offset for controlling left or right cdu

	std::map<std::string, DWORD>keymap;

	// this variable keeps the state of one of the NGX switches
	// NOTE - add these lines to <FSX>\PMDG\PMDG 737 NGX\737NGX_Options.ini: 
	//
	//[SDK]
	//EnableDataBroadcast=1
	//
	// to enable the data sending from the NGX.


	void sendEventToPMDG(DWORD event);
	void genMap(void);
	DWORD translateCode(CString code);
	int  RemoveLength(CString *s);
	void ProcessCDUData(PMDG_NGX_CDU_Screen *pS);
	void ProcessNGXData(PMDG_NGX_Data *pS);
	void sendCharToCDU();

	afx_msg void OnTermThread(WPARAM, LPARAM);
	afx_msg void OnNetworkData(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()





};