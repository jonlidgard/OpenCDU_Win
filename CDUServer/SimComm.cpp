#include "stdafx.h"
#include "SimComm.h"
#include "Convert.h"
#include "messages.h"
#include "ErrorString.h"
#include "CDUServerDlg.h"


UINT SimConnectListener(LPVOID pParam)
{
	SimComm* pSimComm = (SimComm*)pParam;

	if (pSimComm == NULL)
		return 1;   // if pObject is not valid  

	while (pSimComm->GetThreadRunState())
	{
		pSimComm->Dispatch();
		Sleep(pSimComm->GetPollTimeMs());
	}
	return 0;   // thread completed successfully  
}

SimComm::SimComm()
{
	pollTimeMs = 50; // default
	sendEnableFlags = 3; // default - send everything
	m_sendThreadID = NULL;
	genMap();
}

SimComm::~SimComm()
{
}

int SimComm::OpenConnection()
{
	hSimConnect = NULL;
	AircraftRunning = false;
	quitting = false;
	offsetIndex = 0; // always start with left cdu
	ResetAnnunciators();

	if (FAILED(SimConnect_Open(&hSimConnect, "PMDG NGX Test", NULL, 0, 0, 0)))
	{ // failed 
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_UNABLE_TO_CONNECT, 0);

		return FALSE;
	} // failed 

	if (target != NULL)
		target->PostMessage(UWM_SIMCONNECT_INFO, IDX_CONNECTED, 0);

	ASSERT(hSimConnect != NULL);

	// Post a message to the main thread so that it can update the number of
	// open connections
	ASSERT(target != NULL);
	if (target == NULL)
		return FALSE;

//	target->PostMessage(UWM_THREADSTART, 0, (LPARAM)m_nThreadID);
	threadRunState = true;
	pSimCommListenerThread = AfxBeginThread(SimConnectListener, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	pSimCommListenerThread->m_bAutoDelete = false;
	pSimCommListenerThread->ResumeThread();
	SetupCommunication();
	return true;
}

int SimComm::CloseConnection()
{
/*
	if (quitting)
		return 0;
	quitting = true;
*/
	/*
	if (pSimCommListenerThread != NULL)
	{
		threadRunState = false;
		::WaitForSingleObject(pSimCommListenerThread->m_hThread, 5000);
		pSimCommListenerThread = NULL;
	}

	if (hSimConnect != NULL)
	{
		SimConnect_Close(hSimConnect);
//		hSimConnect = NULL;
	}
	*/
	if (target != NULL)
	{
		target->PostMessage(UWM_SIMCONNECT_INFO, IDX_DISCONNECTED, 0);
	}
	return 0;
}


void SimComm::Dispatch()
{
	if (hSimConnect) {
		::SimConnect_CallDispatch(hSimConnect, &SimComm::DispatchCallback, this);
	}
}

// static function
void CALLBACK SimComm::DispatchCallback(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext)
{
	SimComm *pThis = reinterpret_cast<SimComm*>(pContext);
	if (pThis->GetSimConnectHandle() != NULL)
	{
		pThis->Process(pData, cbData);
	}
}

void SimComm::Process(SIMCONNECT_RECV* pData, DWORD cbData)
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
				target->PostMessage(UWM_SIMCONNECT_INFO, IDX_SIM_STARTED, 0);
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
			// bits 0&1 are L,R,C,? CDU , bit 2 is 777
			CString s = evt->szString;
			OutputDebugString(s);
			if (strstr(evt->szString, "PMDG 737") != NULL) {
				AircraftRunning = true;
				// Clear bit 2
				offsetIndex ^= (-0 ^ offsetIndex) & (1 << 2);
				ResetAnnunciators();
				if (target != NULL)
					target->PostMessage(UWM_SIMCONNECT_INFO, IDX_737_AIRCRAFT_RUNNING, 0);
			}
			else if (strstr(evt->szString, "PMDG 777") != NULL) {
				AircraftRunning = true;
				// Set bit 2
				offsetIndex ^= (-1 ^ offsetIndex) & (1 << 2);
				ResetAnnunciators();
				if (target != NULL)
					target->PostMessage(UWM_SIMCONNECT_INFO, IDX_777_AIRCRAFT_RUNNING, 0);
			}
			else {
				if (AircraftRunning) {
					AircraftRunning = false;
					ResetAnnunciators();
					if (target != NULL)
						target->PostMessage(UWM_SIMCONNECT_INFO, IDX_AIRCRAFT_NOT_RUNNING, 0);
				}
			}
		}

		break;
	}

	case SIMCONNECT_RECV_ID_QUIT:
	{
		CloseConnection();
		break;
	}

	default:
		break;
	}
}

void SimComm::ResetAnnunciators() {
	int i = 0;
	for (; i < sizeof(CDU_annunEXEC); i++) {
		CDU_annunEXEC[i] = 0;
		CDU_annunCALL[i] = 0;
		CDU_annunFAIL[i] = 0;
		CDU_annunMSG[i] = 0;
		CDU_annunOFST[i] = 0;
		CDU_annunDSPY[i] = 0;
	}
	SendStringToNetwork("X0");
	SendStringToNetwork("C0");
	SendStringToNetwork("F0");
	SendStringToNetwork("M0");
	SendStringToNetwork("O0");
	SendStringToNetwork("D0");
}

void SimComm::ProcessCDUData(PMDG_NGX_CDU_Screen *pS)
{
	if ((sendEnableFlags & 2) == 0)
		return;
	DWORD screenSize = sizeof(PMDG_NGX_CDU_Screen);
	CByteArray bytes;
	bytes.RemoveAll();
	bytes.SetSize(screenSize);
	memcpy(bytes.GetData(), pS, screenSize);
	SendBytesToNetwork(&bytes);
}

void SimComm::ProcessNGXData(PMDG_NGX_Data *pS)
{
	// SendEnableFlags is set in the ini file
	// Check bit 0 set to send
	if ((sendEnableFlags & 1) == 0)
		return;

	CString s = "";
	CByteArray msg;
	// test the data access:
	// get the state of an annunciator light and display it
	int cdu = offsetIndex & 3; // get bottom 2 bits
	
	// EXEC
	if (pS->CDU_annunEXEC[cdu] != CDU_annunEXEC[cdu])
	{
		CDU_annunEXEC[cdu] = pS->CDU_annunEXEC[cdu];
		if (CDU_annunEXEC[cdu])
		{
			s = "+X";
		}
		else
		{
			s = "-X";
		}
	}

	// CALL
	if (pS->CDU_annunCALL[cdu] != CDU_annunCALL[cdu])
	{
		CDU_annunCALL[cdu] = pS->CDU_annunCALL[cdu];
		if (CDU_annunCALL[cdu])
		{
			s = "-C";

		}
		else
		{
			s = "+C";
		}
	}

	// FAIL
	if (pS->CDU_annunFAIL[cdu] != CDU_annunFAIL[cdu])
	{
		CDU_annunFAIL[cdu] = pS->CDU_annunFAIL[cdu];
		if (CDU_annunFAIL[cdu])
		{
			s = "-F";
		}
		else
		{
			s = "+F";
		}
	}

	// MSG
	if (pS->CDU_annunMSG[cdu] != CDU_annunMSG[cdu])
	{
		CDU_annunMSG[cdu] = pS->CDU_annunMSG[cdu];
		if (CDU_annunMSG[cdu])
		{
			s = "-M";
		}
		else
		{
			s = "+M";
		}
	}

	// OFST
	if (pS->CDU_annunOFST[cdu] != CDU_annunOFST[cdu])
	{
		CDU_annunOFST[cdu] = pS->CDU_annunOFST[cdu];
		if (CDU_annunOFST[cdu])
		{
			s = "-O";
		}
		else
		{
			s = "+O";
		}
	}
	if (s.GetLength() > 0) {
		SendStringToNetwork(s);
	}
}

void SimComm::SendBytesToNetwork(CByteArray *msg)
{
	if (m_sendThreadID == NULL)
		return;

	CByteArray * packet = new CByteArray();
	packet->Append(*msg);
	ASSERT(packet != NULL);
	if (packet == NULL)
		return;
//	TRACE(_T("%s: CConnectSoc::Send: sent UWM_SEND_DATA(%p)\n"), AfxGetApp()->m_pszAppName, packet);
	if (!::PostThreadMessage(m_sendThreadID, UWM_SEND_DATA, (WPARAM)packet, 0))
	{ // post failed 
		ASSERT(FALSE);
		delete packet;
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_FAILED_TO_SEND_TO_NETWORK, 0);
		return;
	} // post failed 

}

void SimComm::SendStringToNetwork(CString s)
{
	CByteArray msg;
	ConvertStringToSendData(s, msg);
	SendBytesToNetwork(&msg);
}

void SimComm::SendToSimFromNetwork(CString s)
{
	DWORD event = translateCode(s);

	if (event > 0)
		sendEventToPMDG(event);
	else
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_UNRECOGNISED_CONTROL_EVENT, 0);
}

void SimComm::genMap(void){
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
	keymap["MENU"] = EVT_CDU_L_MENU;
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
	keymap["CDU_R"] = 1;
	keymap["CDU_C"] = 2;
}

DWORD SimComm::translateCode(CString code) {

	// String arrives as '[n<nnn>]code' where n is length of code

	// Convert a TCHAR string to a LPCSTR
//	int length = RemoveLength(&code);
	CT2CA pszConvertedAnsiString(code);

	// Error decoding length data from string
//	if (length == 0 || length > 5)
//		return 0;

	// construct a std::string using the LPCSTR input
	std::string strStd(pszConvertedAnsiString);
	// Check the keymap first

	std::map<std::string, DWORD>::iterator it = keymap.find(strStd);
	if (it != keymap.end()) {
		if (it->second >= EVT_CDU_L_L1)
			return it->second + GetCduOffset();
		DWORD tmp = offsetIndex;
		// if we're here it's a control code to switch to left or right cdu
		// so set bits 0 & 1 accordingly
		offsetIndex ^= (-(it->second & 1) ^ offsetIndex) & (1 << 0);
		offsetIndex ^= (-(it->second>>1 & 1) ^ offsetIndex) & (1 << 1);
		if (tmp != offsetIndex)
			ResetAnnunciators();
		return 0;
	}

	// Next try the numbers (we don't match a 0 here - it's in the keymap
	char c = toupper(code[0]);
	if (c >= '1' && c <= '9') {
		return EVT_CDU_L_1 + c - '1' + GetCduOffset();
	}

	// Lastly the letters
	if (c >= 'A' && c <= 'Z') {
		return EVT_CDU_L_A + c - 'A' + GetCduOffset();
	}

	// not recoginized
	return 0;
}

void SimComm::sendEventToPMDG(DWORD event) {

	// Test the first control method: use the control data area.
	ASSERT(target != NULL);

	if (AircraftRunning)
	{

		// Send a command only if there is no active command request and previous command has been processed by the NGX
		if (Control.Event == 0)
		{
			Control.Event = event;
			Control.Parameter = MOUSE_FLAG_LEFTSINGLE;

			if (SimConnect_SetClientData(hSimConnect, PMDG_NGX_CONTROL_ID, PMDG_NGX_CONTROL_DEFINITION,
				0, 0, sizeof(PMDG_NGX_Control), &Control) == E_FAIL ) {
				if (target != NULL)
					target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_FAILED_TO_SEND_CONTROL, 0);
			}

		}
		else {
			if (target != NULL)
				target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_PREVIOUS_CONTROL_NOT_PROCD, 0);
		}
	}
	else {
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_AIRCRAFT_NOT_RUNNING, 0);
	}
}

void SimComm::SetupCommunication()
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
	if (hr == E_FAIL) {
		if (target != NULL)
			target->PostMessage(UWM_SIMCONNECT_ERROR, IDX_FAILED_TO_SUBSCRIBE_TO_CDU, 0);
	}

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

	// 3) Request current aircraft .air file path
	hr = SimConnect_RequestSystemState(hSimConnect, AIR_PATH_REQUEST, "AircraftLoaded");
	// also request notifications on sim start and aircraft change
	hr = SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_START, "SimStart");
}
