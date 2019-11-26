
#include "stdafx.h"
#include "SimCommThread.h"
#include "messages.h"
#include "SimComm.h"
#include "Convert.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



IMPLEMENT_DYNCREATE(CSimCommThread, CWinThread)

CSimCommThread::CSimCommThread() :
target(NULL)
{
}

CSimCommThread::~CSimCommThread()
{
}

/****************************************************************************
*                         CSimCommThread::InitInstance
* Result: BOOL
*       TRUE if successful, let thread execute
*       FALSE if falure
* Effect:
*       ¶
****************************************************************************/

BOOL CSimCommThread::  InitInstance()
{
	quit = 0;
	hSimConnect = NULL;
	AircraftRunning = false;

	NGX_CDU0ExecLight = false;
	NGX_CDU0MsgLight = false;

	cduOffset = 0; // always start with left cdu
	
	if (FAILED(SimConnect_Open(&hSimConnect, "PMDG NGX Test", NULL, 0, 0, 0)))
	{ // failed 
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_UNABLE_TO_CONNECT, (LPARAM)m_nThreadID);

		return FALSE;
	} // failed 

	if (target != NULL)
		target->PostMessage(UWM_SIMCONNECT_INFO, IDX_CONNECTED, (LPARAM)m_nThreadID);

	ASSERT(hSimConnect != NULL);

	// Post a message to the main thread so that it can update the number of
	// open connections
	ASSERT(target != NULL);
	if (target == NULL)
		return FALSE;
		
	target->PostMessage(UWM_THREADSTART, 0, (LPARAM)m_nThreadID);

	SetupCommunication();

return TRUE;
}

/****************************************************************************
*                         CSimCommThread::ExitInstance
* Result: int
*       Desired return code
* Effect:
*       Notifies the target that the thread has closed
****************************************************************************/

int CSimCommThread::ExitInstance()
{
	quit = 1;
	if (hSimConnect != NULL)
	{
		SimConnect_Close(hSimConnect);
		hSimConnect = NULL;
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_INFO, IDX_DISCONNECTED, (LPARAM)m_nThreadID);
	}

	ASSERT(target != NULL);
	if (target != NULL)
	{
		OutputDebugStringW(L"\nExitInstance posting UWM_THREADCLOSE!\n");
		target->PostMessage(UWM_THREADCLOSE, 0, (LPARAM)m_nThreadID);
	}

	return CWinThread::ExitInstance();
}

/****************************************************************************
*                                 Message Map
****************************************************************************/

BEGIN_MESSAGE_MAP(CSimCommThread, CWinThread)
	//{{AFX_MSG_MAP(CSimCommThread)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_THREAD_MESSAGE(UWM_TERM_THREAD, OnTermThread)
	ON_THREAD_MESSAGE(UWM_SIMCONNECT_DATA, OnNetworkData)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimCommThread message handlers

/****************************************************************************
*                         CSimCommThread::OnTermThread
* Inputs:
*       WPARAM: ignored
*       LPARAM: ignored
* Result: void
*
* Effect:
*       Requests the thread shut down
****************************************************************************/

void CSimCommThread::OnTermThread(WPARAM, LPARAM)
{
	OutputDebugStringW(L"\nCSimCommThread::OnTermThread!\n");
	::PostQuitMessage(0);
}


void CSimCommThread::OnNetworkData(WPARAM wParam, LPARAM lParam)
{
	// a new message has been received. Send control to SimConnect
	CString s = *(CString *)wParam;
//	CByteArray * data = (CByteArray *)wParam;

//	CString s = ConvertReceivedDataToString(*data);
	DWORD event = translateCode(s);

	if (event > 0)
		sendEventToPMDG(event);
	else
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_UNRECOGNISED_CONTROL_EVENT, (LPARAM)m_nThreadID);

//	delete data;
}




void CSimCommThread::ProcessCDUData(PMDG_NGX_CDU_Screen *pS)
{
	OutputDebugStringW(L"\nCDU Screen updated!\n");

}


void CSimCommThread::ProcessNGXData(PMDG_NGX_Data *pS)
{
	// test the data access:
	// get the state of an annunciator light and display it
	if (pS->CDU_annunEXEC[0] != NGX_CDU0ExecLight)
	{
		NGX_CDU0ExecLight = pS->CDU_annunEXEC[0];
		if (NGX_CDU0ExecLight)
			OutputDebugStringW(L"\nCDU EXEC LIGHT: [ON]");
		else
			OutputDebugStringW(L"\nCDU EXEC LIGHT: [OFF]");
	}

	if (pS->CDU_annunMSG[0] != NGX_CDU0MsgLight)
	{
		NGX_CDU0MsgLight = pS->CDU_annunMSG[0];
		if (NGX_CDU0MsgLight)
			OutputDebugStringW(L"\nCDU MSG LIGHT: [ON]");
		else
			OutputDebugStringW(L"\nCDU MSG LIGHT: [OFF]");
	}


}


// This function is called when NGX data changes
// Refer to SimConnect docs for details about this implementation

BOOL CSimCommThread::OnIdle(LONG lCount)
{
//	::SimConnect_CallDispatch(hSimConnect, &CSimCommThread::DispatchCallback, this);
//	Sleep(5);
	OutputDebugStringW(L"\nOnIdle\n");

	return(0);
}

/*
int CSimCommThread::Run()
{
	MSG msg;
	while (quit == 0)
	{
		while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
//			AfxGetApp()->PumpMessage();
//			Sleep(1);
					if (!AfxGetApp()->PumpMessage())
			{
				::PostQuitMessage(0);
				return(0);
			}
		}

		// receive and process the NGX data
		::SimConnect_CallDispatch(hSimConnect, &CSimCommThread::DispatchCallback, this);
		
//		OutputDebugStringW(L"\nRun!\n");

		Sleep(10);
	}
/*	ASSERT(target != NULL);

	if (hSimConnect != NULL) {
		SimConnect_Close(hSimConnect);
		hSimConnect = NULL;
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_INFO, IDX_DISCONNECTED, (LPARAM)m_nThreadID);
	}
*/
/*	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		//			AfxGetApp()->PumpMessage();
		//			Sleep(1);
		if (!AfxGetApp()->PumpMessage())
		{
			::PostQuitMessage(0);
			return(0);
		}
	}
	::PostQuitMessage(0);
	return(0);
}
*/

void CALLBACK CSimCommThread::DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext)
{
	CSimCommThread *pThis = reinterpret_cast<CSimCommThread*>(pContext);
	pThis->Process(pData, cbData);
}

void CSimCommThread::Process(SIMCONNECT_RECV* pData, DWORD cbData)
{
	switch (pData->dwID)
	{
	case SIMCONNECT_RECV_ID_CLIENT_DATA: // Receive and process the NGX data block
	{
		SIMCONNECT_RECV_CLIENT_DATA *pObjData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;

		switch (pObjData->dwRequestID)
		{
		case DATA_REQUEST:
		{
			PMDG_NGX_Data *pS = (PMDG_NGX_Data*)&pObjData->dwData;
			ProcessNGXData(pS);
			break;
		}
		case CDU_REQUEST:
		{
			PMDG_NGX_CDU_Screen *pS = (PMDG_NGX_CDU_Screen*)&pObjData->dwData;
			ProcessCDUData(pS);
			break;
		}
		case CONTROL_REQUEST:
		{
			// keep the present state of Control area to know if the server had received and reset the command
			PMDG_NGX_Control *pS = (PMDG_NGX_Control*)&pObjData->dwData;
			Control = *pS;
			break;
		}
		}
		break;
	}

	case SIMCONNECT_RECV_ID_EVENT:
	{
		SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;
		switch (evt->uEventID)
		{
		case EVENT_SIM_START:	// Track aircraft changes
		{
			if (target != NULL)
				target->PostMessage(UWM_SIMCONNECT_INFO, IDX_SIM_STARTED, (LPARAM)m_nThreadID);
			HRESULT hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
			break;
		}

		}
		break;
	}

	case SIMCONNECT_RECV_ID_SYSTEM_STATE: // Track aircraft changes
	{
		SIMCONNECT_RECV_SYSTEM_STATE *evt = (SIMCONNECT_RECV_SYSTEM_STATE*)pData;
		if (evt->dwRequestID == AIR_PATH_REQUEST)
		{
			if (strstr(evt->szString, "PMDG 737") != NULL) {
				AircraftRunning = true;
				if (target != NULL)
					target->PostMessage(UWM_SIMCONNECT_INFO, IDX_737_AIRCRAFT_RUNNING, (LPARAM)m_nThreadID);
//				receivedFromNetwork("A");
			}
			else {
				if (AircraftRunning) {
					if (target != NULL)
						target->PostMessage(UWM_SIMCONNECT_INFO, IDX_AIRCRAFT_NOT_RUNNING, (LPARAM)m_nThreadID);
				}
				AircraftRunning = false;
			}
		}
		break;
	}

	case SIMCONNECT_RECV_ID_QUIT:
	{
		quit = 1;
		break;
	}

	default:
		printf("\nReceived:%d", pData->dwID);
		break;
	}
}


void CSimCommThread::SetupCommunication()
{
	HRESULT hr;

	// 1) Set up data connection

		// Associate an ID with the PMDG data area name
		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_NGX_DATA_NAME, PMDG_NGX_DATA_ID);
		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_NGX_CDU_0_NAME, PMDG_NGX_CDU_0_ID);

		// Define the data area structure - this is a required step
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_NGX_DATA_DEFINITION, 0, sizeof(PMDG_NGX_Data), 0, 0);
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_NGX_CDU_0_DEFINITION, 0, sizeof(PMDG_NGX_CDU_Screen), 0, 0);

		// Sign up for notification of data change.  
		// SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED flag asks for the data to be sent only when some of the data is changed.
		hr = SimConnect_RequestClientData(hSimConnect, PMDG_NGX_DATA_ID, DATA_REQUEST, PMDG_NGX_DATA_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);

		hr = SimConnect_RequestClientData(hSimConnect, PMDG_NGX_CDU_0_ID, CDU_REQUEST, PMDG_NGX_CDU_0_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);


		// 2) Set up control connection

		// First method: control data area
		Control.Event = 0;
		Control.Parameter = 0;

		// Associate an ID with the PMDG control area name
		hr = SimConnect_MapClientDataNameToID(hSimConnect, PMDG_NGX_CONTROL_NAME, PMDG_NGX_CONTROL_ID);

		// Define the control area structure - this is a required step
		hr = SimConnect_AddToClientDataDefinition(hSimConnect, PMDG_NGX_CONTROL_DEFINITION, 0, sizeof(PMDG_NGX_Control), 0, 0);

		// Sign up for notification of control change.  
		hr = SimConnect_RequestClientData(hSimConnect, PMDG_NGX_CONTROL_ID, CONTROL_REQUEST, PMDG_NGX_CONTROL_DEFINITION,
			SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED, 0, 0, 0);


		// Second method: Create event IDs for controls that we are going to operate
		//		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_LOGO_LIGHT_SWITCH, "#69754");		//EVT_OH_LIGHTS_LOGO
		//		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_FLIGHT_DIRECTOR_SWITCH, "#70010");	//EVT_MCP_FD_SWITCH_L
		//		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_HEADING_SELECTOR, "#70022");		//EVT_MCP_HEADING_SELECTOR


		// 3) Request current aircraft .air file path
		hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
		// also request notifications on sim start and aircraft change
		hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");


		// 4) Assign keyboard shortcuts
		//		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_KEYBOARD_A);
		//		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_KEYBOARD_B);
		//		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_KEYBOARD_C);
		//		hr = SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_KEYBOARD_D);

		//		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYBOARD, EVENT_KEYBOARD_A);
		//		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYBOARD, EVENT_KEYBOARD_B);
		//		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYBOARD, EVENT_KEYBOARD_C);
		//		hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_KEYBOARD, EVENT_KEYBOARD_D);

		//		hr = SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP_KEYBOARD, SIMCONNECT_GROUP_PRIORITY_HIGHEST);

		//		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "shift+ctrl+a", EVENT_KEYBOARD_A);
		//		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "shift+ctrl+b", EVENT_KEYBOARD_B);
		//		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "shift+ctrl+c", EVENT_KEYBOARD_C);
		//		hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "shift+ctrl+d", EVENT_KEYBOARD_D);

		//		hr = SimConnect_SetInputGroupState(hSimConnect, INPUT0, SIMCONNECT_STATE_ON);


		// 5) Main loop
//		while (quit == 0)
//		{
			// receive and process the NGX data
//			SimConnect_CallDispatch(hSimConnect, xDispatchProc, NULL);

//			Sleep(1);
//		}

//		hr = SimConnect_Close(hSimConnect);
}



void CSimCommThread::genMap(void){
	keymap["L1"] = EVT_CDU_L_L1;
	keymap["L2"] = EVT_CDU_L_L2;
	keymap["L3"] = EVT_CDU_L_L3;
	keymap["L4"] = EVT_CDU_L_L4;
	keymap["L5"] = EVT_CDU_L_L5;
	keymap["L6"] = EVT_CDU_L_L6;

	keymap["R1"] = EVT_CDU_L_R1;
	keymap["R2"] = EVT_CDU_L_R2;
	keymap["R3"] = EVT_CDU_L_R3;
	keymap["R4"] = EVT_CDU_L_R4;
	keymap["R5"] = EVT_CDU_L_R5;
	keymap["R6"] = EVT_CDU_L_R6;

	keymap["INIT"] = EVT_CDU_L_INIT_REF;
	keymap["RTE"] = EVT_CDU_L_RTE;
	keymap["CLB"] = EVT_CDU_L_CLB;
	keymap["CRZ"] = EVT_CDU_L_CRZ;
	keymap["DES"] = EVT_CDU_L_DES;
	keymap["LEGS"] = EVT_CDU_L_LEGS;
	keymap["DEP"] = EVT_CDU_L_DEP_ARR;
	keymap["HOLD"] = EVT_CDU_L_HOLD;
	keymap["PROG"] = EVT_CDU_L_PROG;
	keymap["EXEC"] = EVT_CDU_L_EXEC;
	keymap["N1"] = EVT_CDU_L_N1_LIMIT;
	keymap["FIX"] = EVT_CDU_L_FIX;
	keymap["PREV"] = EVT_CDU_L_PREV_PAGE;
	keymap["NEXT"] = EVT_CDU_L_NEXT_PAGE;
	keymap["."] = EVT_CDU_L_DOT;
	keymap["0"] = EVT_CDU_L_0;
	keymap["-"] = EVT_CDU_L_PLUS_MINUS;
	keymap[" "] = EVT_CDU_L_SPACE;
	keymap["DEL"] = EVT_CDU_L_DEL;
	keymap["/"] = EVT_CDU_L_SLASH;
	keymap["CLR"] = EVT_CDU_L_CLR;
	keymap["BRT"] = EVT_CDU_L_BRITENESS;
	keymap["CDU_L"] = 0;
	keymap["CDU_R"] = EVT_CDU_R_L1 - EVT_CDU_L_L1;

}

int  CSimCommThread::RemoveLength(CString *s)
{
	int length;
	CString x = *s;
	int openingBracket = x.Find('[');
	ASSERT(openingBracket == 0);
	int closingBracket = x.Find(']');
	ASSERT (closingBracket > 1 && closingBracket <6);
	int firstNum = openingBracket + 1;
	if (firstNum > 0 && closingBracket < 6) {
		CString n = x.Mid(firstNum, closingBracket - firstNum);
		*s = x.Mid(closingBracket + 1);
		return _ttoi(n);
	}
	return 0;
}


DWORD CSimCommThread::translateCode(CString code) {

	// String arrives as '[n<nnn>]code' where n is length of code

	// Convert a TCHAR string to a LPCSTR
	int length = RemoveLength(&code);
	CT2CA pszConvertedAnsiString(code);
	
	// Error decoding length data from string
	if (length == 0 || length > 5)
		return 0;

	// construct a std::string using the LPCSTR input
	std::string strStd(pszConvertedAnsiString);
	// Check the keymap first

	std::map<std::string, DWORD>::iterator it = keymap.find(strStd);
	if (it != keymap.end()) {
		if (it->second >= EVT_CDU_L_L1)
			return it->second + cduOffset;

		// if we're here it's a control code to switch to left or right cdu
		cduOffset = it->second;
		return 0;
	}

	// Next try the numbers (we don't match a 0 here - it's in the keymap
	char c = toupper(code[0]);
	if (c >= '1' && c <= '9') {
		return EVT_CDU_L_1 + c - '1' + cduOffset;
	}

	// Lastly the letters
	if (c >= 'A' && c <= 'Z') {
		return EVT_CDU_L_A + c - 'A' + cduOffset;
	}

	// not recoginized
	return 0;
}



void CSimCommThread::sendEventToPMDG(DWORD event) {

	// Test the first control method: use the control data area.
	ASSERT(target != NULL);

	if (1) //AircraftRunning)
	{

		// Send a command only if there is no active command request and previous command has been processed by the NGX
		if (Control.Event == 0)
		{
			Control.Event = event;
			Control.Parameter = MOUSE_FLAG_LEFTSINGLE;

			if (!SimConnect_SetClientData(hSimConnect, PMDG_NGX_CONTROL_ID, PMDG_NGX_CONTROL_DEFINITION,
				0, 0, sizeof(PMDG_NGX_Control), &Control)) {
				if (target != NULL)
					target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_FAILED_TO_SEND_CONTROL, (LPARAM)m_nThreadID);
			}

		}
		else {
			if (target != NULL)
				target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_PREVIOUS_CONTROL_NOT_PROCD, (LPARAM)m_nThreadID);
		}
	}
	else {
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_AIRCRAFT_NOT_RUNNING, (LPARAM)m_nThreadID);
	}
}
