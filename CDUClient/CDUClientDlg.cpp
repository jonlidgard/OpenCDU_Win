// CDUClientDlg.cpp : implementation file
//
// Rewritten to Best Practice by Joseph M. Newcomer, Mar 2007

#include "stdafx.h"
#include "resource.h"
#include "CDUClientDlg.h"
#include "Messages.h"
#include "Convert.h"
#include "UnicodeFont.h"
#include "port.h"
#include "ErrorString.h"
#include "About.h"
#include "PMDG_NGX_SDK.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




/////////////////////////////////////////////////////////////////////////////
// CCDUClientDlg dialog

/****************************************************************************
*                      CCDUClientDlg::CCDUClientDlg
* Inputs:
*       CWnd * pParent: Parent window
* Effect: 
*       Constructor.  Initializes data members
****************************************************************************/

CCDUClientDlg::CCDUClientDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CCDUClientDlg::IDD, pParent),
        m_pClientThread(NULL),
        m_Connected(FALSE),
        m_Connecting(FALSE),
        m_MainWndIsClosing(FALSE),
        m_Sending(FALSE)
{
        //{{AFX_DATA_INIT(CCDUClientDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
        // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
        m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/****************************************************************************
*                       CCDUClientDlg::DoDataExchange
* Inputs:
*       CDataExchange * pDX:
* Result: void
*       
* Effect: 
*       Binds controls to variables
****************************************************************************/

void CCDUClientDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CCDUClientDlg)

		DDX_Control(pDX, IDC_RECORD, c_Record);
        DDX_Control(pDX, IDC_PORT_CAPTION, x_Port);
        DDX_Control(pDX, IDC_SERVERNAME_CAPTION, x_ServerName);
        DDX_Control(pDX, IDC_PORT, c_Port);
        DDX_Control(pDX, IDC_MESSAGE, c_Message);
        DDX_Control(pDX, IDC_SEND, c_Send);
        DDX_Control(pDX, IDC_DISCONNECT, c_Disconnect);
        DDX_Control(pDX, IDC_CONNECTIONSTATUS, c_ConnectionStatus);
        DDX_Control(pDX, IDC_CONNECT, c_Connect);
        DDX_Control(pDX, IDC_SERVERNAME, c_ServerName);
        //}}AFX_DATA_MAP
}

/****************************************************************************
*                                 Message Map
****************************************************************************/

BEGIN_MESSAGE_MAP(CCDUClientDlg, CDialog)
        //{{AFX_MSG_MAP(CCDUClientDlg)
        ON_WM_SYSCOMMAND()
        ON_WM_PAINT()
        ON_WM_QUERYDRAGICON()
        ON_BN_CLICKED(IDC_CONNECT, OnConnect)
        ON_BN_CLICKED(IDC_DISCONNECT, OnDisconnect)
        ON_BN_CLICKED(IDC_SEND, OnSend)
        ON_WM_CLOSE()
        ON_EN_CHANGE(IDC_MESSAGE, OnChangeMessage)
        ON_EN_CHANGE(IDC_PORT, OnChangePort)
        ON_EN_CHANGE(IDC_SERVERNAME, OnChangeServername)
        ON_WM_SIZE()
        //}}AFX_MSG_MAP
        ON_MESSAGE(UWM_CONNECTIONCLOSE, OnConnectionClose)
        ON_MESSAGE(UWM_NETWORK_DATA, OnNetworkData)
        ON_MESSAGE(UWM_CONNECTIONMADE, OnConnectionMade)
        ON_MESSAGE(UWM_NETWORK_ERROR, OnNetworkError)
        ON_MESSAGE(UWM_INFO, OnInfo)
		ON_STN_CLICKED(IDC_CONNECTIONSTATUS, &CCDUClientDlg::OnStnClickedConnectionstatus)
		ON_LBN_SELCHANGE(IDC_RECORD, &CCDUClientDlg::OnLbnSelchangeRecord)
END_MESSAGE_MAP()


/****************************************************************************
*                       CCDUClientDlg::CleanupThread
* Result: BOOL
*       TRUE if there are no active threads
*       FALSE if we have to wait for the thread to close
* Effect: 
*       Forces the client thread to close
****************************************************************************/

BOOL CCDUClientDlg::CleanupThread()
   {
    if(m_pClientThread == NULL)
       return TRUE;   // no thread

    if (!m_pClientThread->PostThreadMessage(UWM_TERM_THREAD, 0, 0))
       TRACE(_T("%s: Thread 0x%02x possibly already terminated\n"), AfxGetApp()->m_pszAppName,m_pClientThread->m_nThreadID);

    return FALSE;
   }

/////////////////////////////////////////////////////////////////////////////
// CCDUClientDlg message handlers

/****************************************************************************
*                        CCDUClientDlg::OnInitDialog
* Result: BOOL
*       TRUE, always
* Effect: 
*       Initializes the dialog
****************************************************************************/

BOOL CCDUClientDlg::OnInitDialog()
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
        
#ifdef _UNICODE

    if(CreateUnicodeFont(this, font))
       { /* set unicode font */
        c_Message.SetFont(&font);
        c_Record.SetFont(&font);
       } /* set unicode font */
#endif

    // Note that in a production program, the port value would be read from
    // some external configuration state, such as the Registry
    UINT port = PORT_NUM;

    CString s;
	c_ServerName.SetWindowText(TEXT("localhost"));
    s.Format(_T("%d"), port);
    c_Port.SetWindowText(s);
    
    updateControls();

    return TRUE;  // return TRUE  unless you set the focus to a control
   }

/****************************************************************************
*                        CCDUClientDlg::OnSysCommand
* Inputs:
*       UINT nID:
*       LPARAM lParam:
* Result: void
*       
* Effect: 
*       Handles WM_SYSCOMMAND
****************************************************************************/

void CCDUClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
*                          CCDUClientDlg::OnPaint
* Result: void
*       
* Effect: 
*       If you add a minimize button to your dialog, you will need the code below
*       to draw the icon.  For MFC applications using the document/view model,
*       this is automatically done for you by the framework.
****************************************************************************/

void CCDUClientDlg::OnPaint() 
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
*                      CCDUClientDlg::OnQueryDragIcon
* Result: HCURSOR
*       
* Effect: 
*       The system calls this to obtain the cursor to display while the user drags
*       the minimized window.
****************************************************************************/

HCURSOR CCDUClientDlg::OnQueryDragIcon()
   {
    return (HCURSOR) m_hIcon;
   }

/////////////////////////////////////////////////////////////

/****************************************************************************
*                         CCDUClientDlg::OnConnect
* Result: void
*       
* Effect: 
*       Handles the connection request button
****************************************************************************/

void CCDUClientDlg::OnConnect() 
   {
    TRACE(_TEXT("%s: CCDUClientDlg::OnConnect()\n"), AfxGetApp()->m_pszAppName);

    CString str;
    c_ServerName.GetWindowText(str);

    // Create a thread to handle the connection. The thread is created suspended so that we can
    // set variables in CClientThread before it starts executing.
    m_pClientThread = (CClientThread*)AfxBeginThread(RUNTIME_CLASS(CClientThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    if (m_pClientThread == NULL)
       { /* failed to start */
        CString fmt;
        fmt.LoadString(IDS_THREAD_CREATE_FAILED);
        CString s;
        s.Format(fmt, str);
        c_ConnectionStatus.SetWindowText(s);
        c_Record.AddString(s);
        TRACE(_T("%s: %s\n"), AfxGetApp()->m_pszAppName, s);
        return;
       } /* failed to start */

    m_pClientThread->SetTarget(this);
    m_pClientThread->SetServerName(str);

    CString s;
    c_Port.GetWindowText(s);
    UINT port = _tcstoul(s, NULL, 0);
    m_pClientThread->SetPort(port);

    m_ServerName = str;

    // Now start the thread.
    m_Connecting = TRUE;
    s.LoadString(IDS_CONNECTING);
    c_ConnectionStatus.SetWindowText(s);
    c_Record.AddString(s);
    
    m_pClientThread->ResumeThread();
    updateControls();
   }

/****************************************************************************
*                        CCDUClientDlg::OnDisconnect
* Result: void
*       
* Effect: 
*       Handles the disconnect button
****************************************************************************/

void CCDUClientDlg::OnDisconnect() 
   {
    // If we have a client thread in a tcp connection, post a user message
    // to the client thread to signal a tear down of the connection is requested.
    // The client thread will close the socket and terminates itself
    TRACE(_T("%s: CDUServerDlg::OnDisconnect\n"), AfxGetApp()->m_pszAppName);
    
    if (m_pClientThread != NULL)
       { /* shut it down */
        TRACE(_T("%s: CCDUServerDlg::OnDisconnect: Sent UWM_TERM_THREAD\n"), AfxGetApp()->m_pszAppName);
        
        if (!m_pClientThread->PostThreadMessage(UWM_TERM_THREAD, 0, 0))
           TRACE(_T("%s: Thread 0x%02x possibly already terminated\n"), AfxGetApp()->m_pszAppName, m_pClientThread->m_nThreadID);
       } /* shut it down */
   }

/****************************************************************************
*                     CCDUClientDlg::OnConnectionClose
* Inputs:
*       WPARAM: ignored
*       LPARAM: (LPARAM)(DWORD) thread ID of thread that closed
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Reports that the connection has closed and the client thread has
*       terminated.
****************************************************************************/

LRESULT CCDUClientDlg::OnConnectionClose(WPARAM, LPARAM lParam)
   {
    TRACE(_T("%s: CCDUServerDlg::OnConnectionClose\n"), AfxGetApp()->m_pszAppName);
    m_pClientThread = NULL;
    m_Connected = FALSE;
    m_Connecting = FALSE;

    CString s;
    s.LoadString(IDS_CLOSED);
    c_ConnectionStatus.SetWindowText(s);
    CString msg;
    msg.Format(_T("%x: %s"), (DWORD)lParam, s);
    
    c_Record.AddString(msg);
    
    updateControls();
    if(m_MainWndIsClosing)
       PostMessage(WM_CLOSE);
    return 0;
   }

/****************************************************************************
*                      CCDUClientDlg::OnConnectionMade
* Inputs:
*       WPARAM: ignored
*       LPARAM: Thread ID of thread making connection (ignored)
* Result: LRESULT
*       LOgically void, 0, always
* Effect: 
*       Notes that the connection has been closed by the server
****************************************************************************/

LRESULT CCDUClientDlg::OnConnectionMade(WPARAM, LPARAM)
   {
    // callback from the client socket thread to signify a connection has
    // been established. 

    TRACE(_T("%s: CCDUServerDlg::OnConnectionMade\n"), AfxGetApp()->m_pszAppName);
    m_Connected = TRUE;
    m_Connecting = FALSE;
    m_Sending = FALSE;
    CString s;
    s.LoadString(IDS_CONNECTED_TO);
    CString msg;
    msg.Format(_T("%s \"%s\""), s, m_ServerName);
    c_ConnectionStatus.SetWindowText(msg);
    c_Record.AddString(msg);

    updateControls();
    return 0;
   }

/****************************************************************************
*                           CCDUClientDlg::OnSend
* Result: void
*       
* Effect: 
*       Handles the Send button
* Notes:
*       There is no need to check for the thread being legal or connected
*       because the Send button would not be enabled if the socket were
*       not connected in the thread
****************************************************************************/

void CCDUClientDlg::OnSend() 
   {
    TRACE(_T("%s: CCDUServerDlg::OnSend\n"), AfxGetApp()->m_pszAppName);
    CString str;
    c_Message.GetWindowText(str);
    CString log;

#define INFO_LIMIT 63
    log.Format(_T("=> %s"), str);
    c_Record.AddString(log);

	CByteArray data;
	ConvertStringToSendData(str, data);


	CByteArray packet;

	packet.Append(data);

	m_pClientThread->Send(packet);
 //   m_Sending = TRUE;
    updateControls();
   }

/****************************************************************************
*                        CCDUClientDlg::OnNetworkData
* Inputs:
*       WPARAM: (WPARAM)(CByteArray *) The data to display
*       LPARAM: (LPARAM)(DWORD) the thread that sent it (ignored)
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Displays the string received
* Notes:
*       Deletes the string
****************************************************************************/

void CCDUClientDlg::OuputCDUScreen(CByteArray *data)
{
	union cduMap{
		char cduBytes[sizeof(PMDG_NGX_CDU_Screen)];
		PMDG_NGX_CDU_Screen screen;
	};

	cduMap cdu;
	PMDG_NGX_CDU_Cell cell;

	memcpy(cdu.cduBytes, data->GetData(), sizeof(PMDG_NGX_CDU_Screen));

	int x, y;
	CString line;
	c_Record.AddString(_T(""));
	c_Record.AddString(_T("<= ____________________________"));

	for (y = 0; y < CDU_ROWS; y++)
	{
		line = "";
		for (x = 0; x < CDU_COLUMNS; x++)
		{
			cell = cdu.screen.Cells[x][y];
			line.AppendChar(cell.Symbol);
		}
		c_Record.AddString(_T("<= | ") + line + _T(" |"));
	}
	c_Record.AddString(_T("<= |__________________________|"));
	c_Record.AddString(_T(""));
}

LRESULT CCDUClientDlg::OnNetworkData(WPARAM wParam, LPARAM)
   {
    CByteArray * data = (CByteArray *)wParam;

	// Strip out the int32 holding the length of the message at the front
	// Added by QT's DataStrea when serializing a QString or QByteArray.

/*	union byteint
	{
		byte b[sizeof UINT32];
		int i;
	};

	byteint bi;

	for (int i = 0; i < 4; i++)
		bi.b[i] = data->GetAt(i);
	data->RemoveAt(0, 4);
*/
//	if (bi.i == 0x3f1)
	
	if (data->GetSize() == 0x3f1)
		{
		OuputCDUScreen(data);
	}
	else
	{
		CString s = ConvertReceivedDataToString(*data);
		c_Record.AddString(_T("<= \"") + s + _T("\""));
	}

    delete data;
    m_Sending = FALSE;
    updateControls();
    return (LRESULT)0;
   }

/****************************************************************************
*                          CCDUClientDlg::OnClose
* Result: void
*       
* Effect: 
*       Initiates shutdown of the thread and closes the app
****************************************************************************/

void CCDUClientDlg::OnClose() 
   {
    TRACE(_T("%s: CCDUServerDlg::OnClose\n"), AfxGetApp()->m_pszAppName);
    m_MainWndIsClosing = TRUE;
    updateControls();

    if(!CleanupThread())
       { /* defer shutdown */
        TRACE(_T("%s: CCDUClientDlg::OnClose: deferring close\n"), AfxGetApp()->m_pszAppName);
        return;
       } /* defer shutdown */
    CDialog::OnOK();
   }

/****************************************************************************
*                       CCDUClientDlg::OnButtonClose
* Result: void
*       
* Effect: 
*       Close button clicked
****************************************************************************/

void CCDUClientDlg::OnButtonClose() 
   {
    PostMessage(WM_CLOSE);
   }

/****************************************************************************
*                       CCDUClientDlg::updateControls
* Result: void
*       
* Effect: 
*       updates the control states
****************************************************************************/

void CCDUClientDlg::updateControls()
    {
     CString port;
     c_Port.GetWindowText(port);

     // This is to illustrate how to implement a constraint.
     // I have arbitrarily chosen to not allow the selection
     // of a port in the Reserved or Well-Known-Ports area
     // (0..1023)
     UINT portnum;
     portnum = _tcstoul(port, NULL, 0);
     
     CString srv;
     c_ServerName.GetWindowText(srv);
     srv.TrimLeft();
     srv.TrimRight();
     
     //----------------
     // [_Connect_]
     //----------------
     c_Connect.EnableWindow(!m_Connecting && !m_Connected && !srv.IsEmpty() && portnum > 1023 && portnum <= 65535);
     //----------------
     // [_Disconnect_]
     //----------------
     c_Disconnect.EnableWindow(m_Connected);

     //----------------
     // Server [ name ]
     //----------------
     BOOL changeable = !m_Connected && !m_Connecting;
     c_ServerName.SetReadOnly(!changeable);
     x_ServerName.EnableWindow(changeable); 

     //----------------
     // Port [ number ]
     //----------------
     c_Port.SetReadOnly(!changeable); 
     x_Port.EnableWindow(changeable);  

     //----------------
     // [_Send_]
     //----------------
     CString msg;
     c_Message.GetWindowText(msg);
     c_Send.EnableWindow(m_Connected && !m_Sending && !msg.IsEmpty());
    } // CCDUClientDlg::updateControls

/****************************************************************************
*                      CCDUClientDlg::OnChangeMessage
* Result: void
*       
* Effect: 
*       Forces an update of control state
****************************************************************************/

void CCDUClientDlg::OnChangeMessage() 
   {
    updateControls();
   }

/****************************************************************************
*                        CCDUClientDlg::OnChangePort
* Result: void
*       
* Effect: 
*       Forces a control update
****************************************************************************/

void CCDUClientDlg::OnChangePort() 
   {
    updateControls();
   }

/****************************************************************************
*                     CCDUClientDlg::OnChangeServerName
* Result: void
*       
* Effect: 
*       Updates the controls
****************************************************************************/

void CCDUClientDlg::OnChangeServername() 
   {
    updateControls();
   }

/****************************************************************************
*                       CCDUClientDlg::OnNetworkError
* Inputs:
*       WPARAM: (WPARAM)(DWORD) error code
*       LPARAM: (LPARAM)(DWORD) thread ID of issuing thread (ignored)
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Reports a network error
****************************************************************************/

LRESULT CCDUClientDlg::OnNetworkError(WPARAM wParam, LPARAM)
    {
     DWORD err = (DWORD)wParam;
     CString msg = ErrorString(err);
     c_ConnectionStatus.SetWindowText(msg);
     c_Record.AddString(msg);
     return 0;
    } // CCDUClientDlg::OnNetworkError

/****************************************************************************
*                           CCDUClientDlg::OnInfo
* Inputs:
*       WPARAM: (WPARAM)(CString *) String to display
*       LPARAM: (LPARAM)(DWORD) thread ID of thread
* Result: LRESULT
*       Logically void, 0, always
* Effect: 
*       Logs the info string
****************************************************************************/

LRESULT CCDUClientDlg::OnInfo(WPARAM wParam, LPARAM lParam)
    {
     CString * s = (CString *)wParam;
     CString id;
     id.Format(_T("%x: "), (DWORD)lParam);
     *s = id + *s;
     c_Record.AddString(*s);
     delete s;

     return 0;
    } // CCDUClientDlg::OnInfo

/****************************************************************************
*                           CCDUClientDlg::OnSize
* Inputs:
*       UINT nType:
*       int cx: new width
*       int cy: new height
* Result: void
*       
* Effect: 
*       Resizes the logging control
****************************************************************************/

void CCDUClientDlg::OnSize(UINT nType, int cx, int cy) 
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


void CCDUClientDlg::OnStnClickedConnectionstatus()
{
	// TODO: Add your control notification handler code here
}


void CCDUClientDlg::OnLbnSelchangeRecord()
{
	// TODO: Add your control notification handler code here
}
