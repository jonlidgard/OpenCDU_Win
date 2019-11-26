// CDUServerDlg.cpp : implementation file
// Rewritten to Best Practice by Joseph M. Newcomer, Mar 2007
//

#include "stdafx.h"
#include "resource.h"
#include "CDUServerDlg.h"
#include "messages.h"
#include "port.h"
#include "Convert.h"
#include "UnicodeFont.h"
#include "ErrorString.h"
#include "About.h"
#include "SimComm.h"
#include "Shlwapi.h"
#pragma comment(lib, "shlwapi.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue)
{
	strValue = strDefaultValue;
	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		strValue = szBuffer;
	}
	return nError;
}


/////////////////////////////////////////////////////////////////////////////
// CCDUServerDlg dialog

/****************************************************************************
*                      CCDUServerDlg::CCDUServerDlg
* Inputs:
*       CWnd * parent:
* Effect: 
*       Constructor
****************************************************************************/

CCDUServerDlg::CCDUServerDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CCDUServerDlg::IDD, pParent),
        m_open(0),
        m_close(0),
        m_running(FALSE),
        m_MainWndIsClosing(FALSE)
{
        //{{AFX_DATA_INIT(CCDUServerDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
        // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
        m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/****************************************************************************
*                       CCDUServerDlg::DoDataExchange
* Inputs:
*       CDataExchange * pDX:
* Result: void
*       
* Effect: 
*       Binds controls to variables
****************************************************************************/

void CCDUServerDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CCDUServerDlg)
        DDX_Control(pDX, IDC_RECORD, c_Record);                          
        //}}AFX_DATA_MAP
}

/****************************************************************************
*                                 Message Map
****************************************************************************/

BEGIN_MESSAGE_MAP(CCDUServerDlg, CDialog)
        //{{AFX_MSG_MAP(CCDUServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_SIZE()
        //}}AFX_MSGMAP
        ON_MESSAGE(UWM_THREADSTART, OnThreadStart)
        ON_MESSAGE(UWM_THREADCLOSE, OnThreadClose)
        ON_MESSAGE(UWM_NETWORK_DATA, OnNetworkData)    
        ON_MESSAGE(UWM_INFO, OnInfo)
        ON_MESSAGE(UWM_NETWORK_ERROR, OnNetworkError)
        ON_MESSAGE(UWM_SEND_COMPLETE, OnSendComplete)
		ON_MESSAGE(UWM_SIMCONNECT_DATA, OnSimConnectData)
		ON_MESSAGE(UWM_SIMCONNECT_INFO, OnSimConnectInfo)
		ON_MESSAGE(UWM_SIMCONNECT_ERROR, OnSimConnectError)
		ON_LBN_SELCHANGE(IDC_RECORD, &CCDUServerDlg::OnLbnSelchangeRecord)
END_MESSAGE_MAP()


/****************************************************************************
*                       CCDUServerDlg::CleanupThreads                      
* Result: BOOL                                                               
*       TRUE if there were no threads running                                
*       FALSE if threads are being shut down                                 
* Effect:                                                                    
*       Initiates thread shutdown                                            
****************************************************************************/

BOOL CCDUServerDlg::CleanupThreads() 
   {
    INT_PTR size = m_threadIDs.GetSize();
    if(size == 0)                      
       return TRUE;                    

    for (INT_PTR i = 0; i < size; i++)
       { /* scan threads */
        if (!::PostThreadMessage(m_threadIDs[i], UWM_TERM_THREAD, 0, 0))
           { /* failed */
            TRACE(_T("%s: Thread 0x%02x possibly already terminated\n"), AfxGetApp()->m_pszAppName, m_threadIDs[i]);
            m_threadIDs.RemoveAt(i);
           } /* failed */
       } /* scan threads */

    // Note that if PostThreadMessage has failed and all the target threads have
    // been removed from the array, we are done

    if(m_threadIDs.GetSize() == 0)
       return TRUE;
    
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CCDUServerDlg message handlers

/****************************************************************************
*                       CCDUServerDlg::CreateListener
* Result: BOOL
*       TRUE if socket created
*       FALSE if error
* Effect: 
*       Creates a listener socket
****************************************************************************/

BOOL CCDUServerDlg::CreateListener()
    {
     // Create the listener socket

     if(!m_listensoc.Create(portnum))
        { /* failed to create */
         DWORD err = ::GetLastError();
         CString fmt;
         fmt.LoadString(IDS_LISTEN_CREATE_FAILED);
         CString * s = new CString;
         s->Format(fmt, portnum);
         PostMessage(UWM_INFO, (WPARAM)s, ::GetCurrentThreadId());
         PostMessage(UWM_NETWORK_ERROR, (WPARAM)err, ::GetCurrentThreadId());
         return FALSE;
        } /* failed to create */

     { /* success */
      CString fmt;
      fmt.LoadString(IDS_LISTENER_CREATED);
      CString * s = new CString;
      s->Format(fmt, portnum);
      PostMessage(UWM_INFO, (WPARAM)s,::GetCurrentThreadId());
     } /* success */

     m_listensoc.SetTarget(this);

     m_listensoc.Listen();
     return TRUE;
    } // CCDUServerDlg::CreateListener

/****************************************************************************
*                        CCDUServerDlg::OnInitDialog
* Result: BOOL
*       TRUE, always
* Effect: 
*       Initializes the dialog
****************************************************************************/

BOOL CCDUServerDlg::OnInitDialog()
   {
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
       {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
           {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
           }
       }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);                 // Set big icon
    SetIcon(m_hIcon, FALSE);                // Set small icon
        
    // TODO: Add extra initialization here
        
#ifdef _UNICODE
    if(CreateUnicodeFont(this, font))
       { /* got a font */
        c_Record.SetFont(&font);
       } /* got a font */
#endif

    CString s;

    updateControls();
	pSimComm = new SimComm();
	pSimComm->SetTarget(this);

	ReadIniFile();
	m_running = CreateListener();

	pSimComm->OpenConnection();
	updateControls();
		
	PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
   }


/****************************************************************************
*                        CCDUServerDlg::ReadIniFile
* Result: void
*
* Effect:
*       Reads an ini file from the same path as apps .exe file
*       Called <appname>.ini. Writes the file back once read with defaults for
*       entries that don't exist.
****************************************************************************/

void CCDUServerDlg::ReadIniFile()
{
//	WritePrivateProfileStringW(NULL, NULL, NULL, L"cducomm.ini");
//	TCHAR   inBuf[80];
	HKEY hKey;
	std::wstring appPath = L"";
	std::wstring s;


	// Get the location of P3d
	if (RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Lockheed Martin\\Prepar3D v3"), 0, KEY_READ,
		&hKey
		) == ERROR_SUCCESS)
	{
		GetStringRegKey(hKey, L"AppPath", appPath, L"");
	}

	TCHAR szAppPath[MAX_PATH] = L"";
	DWORD length = GetModuleFileName(NULL, szAppPath, MAX_PATH);
//	std::wstring strAppName;
	CString strAppName;
	CString strIniName;
	strAppName = szAppPath;

	PathRemoveFileSpec(szAppPath);
	int pos = strAppName.ReverseFind('\\') + 1;
	strAppName= strAppName.Mid(pos);
	strAppName = strAppName.Left(strAppName.GetLength() - 4);
	strAppName += ".ini";
//	strAppName = strAppName.substr(strAppName.rfind(L"\\") + 1);


	// Buffer to hold combined path.
	TCHAR buffer_1[MAX_PATH] = L"";
	TCHAR *lpStr1;
	lpStr1 = buffer_1;

	PathCombine(lpStr1, szAppPath, strAppName);

	logNetworkTraffic = GetPrivateProfileInt(TEXT("General"),
		TEXT("LogNetworkTraffic"),
		0,
		lpStr1);
	logNetworkTraffic = logNetworkTraffic != 0 ? 1 : 0;

	pSimComm->SetPollTimeMs( GetPrivateProfileInt(TEXT("General"),
		TEXT("SimConnectPollTimeMs"),
		50,
		lpStr1));

	pSimComm->SetSendFlags(GetPrivateProfileInt(TEXT("General"),
		TEXT("SendEnableFlags"),
		3,
		lpStr1));

	portnum = GetPrivateProfileInt(TEXT("General"),
		TEXT("NetworkPort"),
		PORT_NUM,
		lpStr1);

	// bits 0&1 are L,R,C,? CDU , bit 2 is 777
		
	int offset = GetPrivateProfileInt(TEXT("Offsets"),
		TEXT("CDU737Left"),
		PMDG737_CDU_L_OFFSET,
		lpStr1);
		
	pSimComm->SetCduOffset(0, offset);

	offset = GetPrivateProfileInt(TEXT("Offsets"),
		TEXT("CDU737Right"),
		PMDG737_CDU_R_OFFSET,
		lpStr1);

	pSimComm->SetCduOffset(1, offset);

	offset = GetPrivateProfileInt(TEXT("Offsets"),
		TEXT("CDU777Left"),
		PMDG777_CDU_L_OFFSET,
		lpStr1);

	pSimComm->SetCduOffset(4, offset);

	offset = GetPrivateProfileInt(TEXT("Offsets"),
		TEXT("CDU777Right"),
		PMDG777_CDU_R_OFFSET,
		lpStr1);

	pSimComm->SetCduOffset(5, offset);

	offset = GetPrivateProfileInt(TEXT("Offsets"),
		TEXT("CDU777Centre"),
		PMDG777_CDU_C_OFFSET,
		lpStr1);

	pSimComm->SetCduOffset(6, offset);

	// Write the values back to the ini file
	if (appPath.length() > 0) {

		s = std::to_wstring(logNetworkTraffic);
		WritePrivateProfileString(TEXT("General"),
			TEXT("LogNetworkTraffic"),
			s.c_str(),
			lpStr1);

		s = std::to_wstring(pSimComm->GetPollTimeMs());
		WritePrivateProfileString(TEXT("General"),
			TEXT("SimConnectPollTimeMs"),
			s.c_str(),
			lpStr1);

		s = std::to_wstring(pSimComm->GetSendFlags());
		WritePrivateProfileString(TEXT("General"),
			TEXT("SendEnableFlags"),
			s.c_str(),
			lpStr1);

		s = std::to_wstring(portnum);
		WritePrivateProfileString(TEXT("General"),
			TEXT("NetworkPort"),
			s.c_str(),
			lpStr1);

		s = std::to_wstring(pSimComm->GetCduOffset(0));
		WritePrivateProfileString(TEXT("Offsets"),
			TEXT("CDU737Left"),
			s.c_str(),
			lpStr1);

		s = std::to_wstring(pSimComm->GetCduOffset(1));
		WritePrivateProfileString(TEXT("Offsets"),
			TEXT("CDU737Right"),
			s.c_str(),
			lpStr1);


		s = std::to_wstring(pSimComm->GetCduOffset(4));
		WritePrivateProfileString(TEXT("Offsets"),
			TEXT("CDU777Left"),
			s.c_str(),
			lpStr1);

		s = std::to_wstring(pSimComm->GetCduOffset(5));
		WritePrivateProfileString(TEXT("Offsets"),
			TEXT("CDU777Right"),
			s.c_str(),
			lpStr1);

		s = std::to_wstring(pSimComm->GetCduOffset(6));
		WritePrivateProfileString(TEXT("Offsets"),
			TEXT("CDU777Centre"),
			s.c_str(),
			lpStr1);

	}
}

/****************************************************************************
*                        CCDUServerDlg::OnSysCommand
* Inputs:
*       UINT nID: Menu ID
*       LPARAM lParam: unused, pass to superclass
* Result: void
*       
* Effect: 
*       Processes a system menu click
****************************************************************************/

void CCDUServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
        if ((nID & 0xFFF0) == IDM_ABOUTBOX)
        {
                CAboutDlg dlgAbout;
                dlgAbout.DoModal();
        }
        else
        {
                CDialog::OnSysCommand(nID, lParam);
        }
}

/****************************************************************************
*                          CCDUServerDlg::OnPaint
* Result: void
*       
* Effect: 
*       If you add a minimize button to your dialog, you will need the code
*       below to draw the icon.  For MFC applications using the document/view
*       model, this is automatically done for you by the framework.
****************************************************************************/

void CCDUServerDlg::OnPaint() 
{
        if (IsIconic())
        {
                CPaintDC dc(this); // device context for painting

                SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

                // Center icon in client rectangle
                int cxIcon = GetSystemMetrics(SM_CXICON);
                int cyIcon = GetSystemMetrics(SM_CYICON);
                CRect rect;
                GetClientRect(&rect);
                int x = (rect.Width() - cxIcon + 1) / 2;
                int y = (rect.Height() - cyIcon + 1) / 2;

                // Draw the icon
                dc.DrawIcon(x, y, m_hIcon);
        }
        else
        {
                CDialog::OnPaint();
        }
}

/****************************************************************************
*                      CCDUServerDlg::OnQueryDragIcon
* Result: HCURSOR
*       Handle of cursor to use as drag icon
* Effect: 
*       The system calls this to obtain the cursor to display while the user
*       drags the minimized window.
****************************************************************************/

HCURSOR CCDUServerDlg::OnQueryDragIcon()
{
        return (HCURSOR) m_hIcon;
}

/****************************************************************************
*                       CCDUServerDlg::OnThreadStart
* Inputs:
*       WPARAM unused
*       LPARAM lParam: (LPARAM)(DWORD) the thread ID
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Increments the open-thread count.  Adds the thread ID to the
*       array of thread IDs
****************************************************************************/

LRESULT CCDUServerDlg::OnThreadStart(WPARAM, LPARAM lParam)
   {
    m_open++;

    DWORD ThreadID = (DWORD)lParam;
    m_threadIDs.Add(ThreadID);  // save thread refs so we can post msg to quit later
	if (pSimComm != NULL)
		pSimComm->SetSendThreadID(ThreadID);
    return 0;
   }


/****************************************************************************
*                       CCDUServerDlg::OnThreadClose
* Inputs:
*       WPARAM unused
*       LPARAM: lParam: (LPARAM)(DWORD) thread ID
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Records that a thread has closed.  If a thread has closed and
*       a close operation has been deferred, completes the close transaction
****************************************************************************/

LRESULT CCDUServerDlg::OnThreadClose(WPARAM, LPARAM lParam)
   {
    m_open--;
    m_close++;
                
    // remove dwThreadID from m_threadIDs
    DWORD dwThreadID = (DWORD)lParam;

	if (pSimComm != NULL) {
		if (pSimComm->GetSendThreadID() == dwThreadID)
			pSimComm->SetSendThreadID(NULL);
	}

    INT_PTR size = m_threadIDs.GetSize();
    for (INT_PTR i = 0; i < size; i++)
       { /* scan threads */
        if (m_threadIDs[i] ==  dwThreadID)
           {
            m_threadIDs.RemoveAt(i);
            TRACE(_T("%s: Thread 0x%02x is removed\n"), AfxGetApp()->m_pszAppName, dwThreadID);
            break;
           }
       } /* scan threads */

   // If we got a WM_CLOSE and there were open threads,  
   // we deferred the close until all the threads terminated
   // Now we try again  
    if(m_MainWndIsClosing && m_open == 0) 
       PostMessage(WM_CLOSE);  // try the close again
    return 0;
   }

/****************************************************************************
*                        CCDUServerDlg::OnNetworkData
* Inputs:
*       WPARAM: (WPARAM)(CByteArray *) The data to display
*       LPARAM: (LPARAM)(DWORD) the thread ID of the thread that sent it
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Displays the string received
* Notes:
*       Deletes the string
****************************************************************************/

LRESULT CCDUServerDlg::OnNetworkData(WPARAM wParam, LPARAM lParam)
   {	   
	// a new message has been received. Update the UI
    CByteArray * data = (CByteArray *)wParam;

	// Strip out the int32 holding the length of the message at the front
	// Added by QT's DataStrea when serializing a QString or QByteArray.

	union byteint
	{
		byte b[sizeof UINT32];
		int i;
	};

	byteint bi;
	/*
	for (int i = 0; i < 4; i++)
		bi.b[i] = data->GetAt(i);
	data->RemoveAt(0, 4);
	*/
    CString s = ConvertReceivedDataToString(*data);
	OutputDebugString(s);
	pSimComm->SendToSimFromNetwork(s);

	#define SERVER_STRING_INFO_LIMIT 63

	if (logNetworkTraffic != 0)
	{
		CString msg;
		msg.Format(_T("CDU control:%-*s"), SERVER_STRING_INFO_LIMIT, s);
		c_Record.AddString(msg);
		delete data;
	}

    return 0;
   }

/****************************************************************************
*                          CCDUServerDlg::OnClose
* Result: void
*       
* Effect: 
*       The program has received a WM_CLOSE message and needs to shut down,
*       so this initiates a shutdown
****************************************************************************/

void CCDUServerDlg::OnClose() 
   {
	pSimComm->CloseConnection();
    m_MainWndIsClosing = TRUE;
    updateControls();

    if(!CleanupThreads())
       { /* threads running */
        TRACE(_T("%s: CCDUServerDlg::OnClose: deferring close\n"), AfxGetApp()->m_pszAppName);
        return;
       } /* threads running */

	delete pSimComm;
    CDialog::OnOK();
   }

/****************************************************************************
*                            CCDUServerDlg::OnOK
*                          CCDUServerDlg::OnCancel
* Result: void
*       
* Effect: 
*       Does nothing.  This is so that an <enter> key will not accidentally
*       terminate the program
****************************************************************************/

void CCDUServerDlg::OnOK() 
   {
   }

void CCDUServerDlg::OnCancel() 
   {
   }

/****************************************************************************
*                       CCDUServerDlg::updateControls                      
* Result: void                                                               
*                                                                            
* Effect:                                                                    
*       Updates the controls                                                 
****************************************************************************/

void CCDUServerDlg::updateControls()
    {
     EnableWindow(!m_MainWndIsClosing);
    } // CCDUServerDlg::updateControls


/****************************************************************************
*                           CCDUServerDlg::OnSize
* Inputs:
*       UINT nType:
*       int cx:
*       int cy:
* Result: void
*       
* Effect: 
*       Resizes the listbox
****************************************************************************/

void CCDUServerDlg::OnSize(UINT nType, int cx, int cy) 
   {
    CDialog::OnSize(nType, cx, cy);
        
    if(c_Record.GetSafeHwnd() != NULL)
       { /* resize it */
        CRect r;
        c_Record.GetWindowRect(&r);
        ScreenToClient(&r);
        c_Record.SetWindowPos(NULL, 0, 0,
                              cx - r.left,
                              cy - r.top,
                              SWP_NOMOVE | SWP_NOZORDER);
       } /* resize it */
   }

/****************************************************************************
*                           CCDUServerDlg::OnInfo
* Inputs:
*       WPARAM: (WPARAM)(CString *) String to display
*       LPARAM: (LPARAM)(DWORD) thread id of thread providing info
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Logs the info string
****************************************************************************/

LRESULT CCDUServerDlg::OnInfo(WPARAM wParam, LPARAM lParam)
    {
     CString * s = (CString *)wParam;
/*	 if ((DWORD)lParam != 0) {
		 CString id;
		 id.Format(_T("%x: "), (DWORD)lParam);
		 *s = id + *s;
	 }
 */    c_Record.AddString(*s);
     delete s;

     return 0;
    } // CCDUServerDlg::OnInfo


/****************************************************************************
*                       CCDUServerDlg::OnNetworkError
* Inputs:
*       WPARAM: (WPARAM)(DWORD) error code
*       LPARAM: (LPARAM)(DWORD) thread that issued the error
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Records the error
****************************************************************************/

LRESULT CCDUServerDlg::OnNetworkError(WPARAM wParam, LPARAM lParam)
    {
     DWORD err = (DWORD)wParam;
     CString msg = ErrorString(err);
     CString id;
     id.Format(_T("%x: "), (DWORD)lParam);
     c_Record.AddString(id + msg);
     return 0;
    } // CCDUServerDlg::OnNetworkError

/****************************************************************************
*                       CCDUServerDlg::OnSendComplete
* Inputs:
*       WPARAM: unused
*       LPARAM: (WPARAM)(DWORD) Thread ID of thread that sent data
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Logs the data-sent event
****************************************************************************/

LRESULT CCDUServerDlg::OnSendComplete(WPARAM, LPARAM lParam)
    {
		if (logNetworkTraffic != 0)
		{
			DWORD threadid = (DWORD)lParam;
			CString fmt;
			fmt.LoadString(IDS_DATA_SENT);
			CString s;
			s.Format(fmt, threadid);
			c_Record.AddString(s);
		}
     return 0;
    } // CCDUServerDlg::OnSendComplete

LRESULT CCDUServerDlg::OnSimConnectData(WPARAM wParam, LPARAM lParam)
{
	CString *s = (CString *)wParam;
	CByteArray msg;
	ConvertStringToSendData(*s, msg);
//	Send(msg, ::GetCurrentThreadId());
	delete s;
	return 0;
}

LRESULT CCDUServerDlg::OnSimConnectInfo(WPARAM code, LPARAM lParam)
{
	switch (code)
	{
	case IDX_737_AIRCRAFT_RUNNING:
		c_Record.AddString("PMDG 737 Aircraft Running.");
		break;
	case IDX_777_AIRCRAFT_RUNNING:
		c_Record.AddString("PMDG 777 Aircraft Running.");
		break;
	case IDX_AIRCRAFT_NOT_RUNNING:
		c_Record.AddString("No supported aircraft running.");
		break;
	case IDX_CONNECTED:
		c_Record.AddString("Connected to Flight Simulator.");
		break;
	case IDX_DISCONNECTED:
//		c_Record.AddString("Disconnected from Flight Simulator.");
		PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);

		break;
	default:
		break;
	}
	return 0;
}

LRESULT CCDUServerDlg::OnSimConnectError(WPARAM code, LPARAM lParam)
{
	switch (code)
	{
	case IDX_FAILED_TO_SEND_CONTROL:
		c_Record.AddString("ERROR: Failed to send control to SimConnect.");
		break;
	case IDX_FAILED_TO_SEND_TO_NETWORK:
		c_Record.AddString("ERROR: Failed to send SimConnect event over network.");
		break;
	case IDX_FAILED_TO_SUBSCRIBE_TO_CDU:
		c_Record.AddString("ERROR: Check EnableCDUBroadcast.0 = 1 in 737NGX_Options.ini.");
		break;
	case IDX_AIRCRAFT_NOT_RUNNING:
		c_Record.AddString("ERROR: No supported aircraft running.");
		break;
	case IDX_UNABLE_TO_CONNECT:
		c_Record.AddString("ERROR: Unable to connect to Flight Simulator.");
		break;
		
	default:
		break;
	}
	return 0;
}


void CCDUServerDlg::OnLbnSelchangeRecord()
{
	// TODO: Add your control notification handler code here
}
