// CDUClientDlg.h : header file
//

#if !defined(AFX_CDUCLIENTDLG_H__1337A90A_C2AC_11D1_92AD_00C04FAD895E__INCLUDED_)
#define AFX_CDUCLIENTDLG_H__1337A90A_C2AC_11D1_92AD_00C04FAD895E__INCLUDED_

#include "ConnectS.h"
#include "ClientT.h"
#include "Log.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CCDUClientDlg dialog

class CCDUClientDlg : public CDialog
{
// Construction
    public:
       CCDUClientDlg(CWnd* pParent = NULL);  // standard constructor

    protected:// Dialog Data
       CClientThread* m_pClientThread;
       CString m_ServerName;
       BOOL m_Connected;
       BOOL m_Connecting;
       BOOL m_MainWndIsClosing;
       BOOL m_Sending;
       
    protected: // controls
        //{{AFX_DATA(CCDUClientDlg)
        enum { IDD = IDD_CDUCLIENT_DIALOG };
        CLog    c_Record;
        CStatic x_Port;
        CStatic x_ServerName;
        CEdit   c_Port;
        CEdit   c_Message;
        CButton c_Send;
        CButton c_Disconnect;
        CStatic c_ConnectionStatus;
        CButton c_Connect;
        CEdit   c_ServerName;
        //}}AFX_DATA

        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CCDUClientDlg)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:
        HICON m_hIcon;
        CFont font;
        BOOL CleanupThread();
		void OuputCDUScreen(CByteArray *data);
        void updateControls();
        // Generated message map functions
        //{{AFX_MSG(CCDUClientDlg)
        virtual BOOL OnInitDialog();
        afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        afx_msg void OnConnect();
        afx_msg void OnDisconnect();
        afx_msg void OnSend();
        afx_msg void OnClose();
        afx_msg void OnButtonClose();
        afx_msg void OnChangeMessage();
        afx_msg void OnChangePort();
        afx_msg void OnChangeServername();
        afx_msg void OnSize(UINT nType, int cx, int cy);
        //}}AFX_MSG

        LRESULT OnInfo(WPARAM, LPARAM);
        LRESULT OnConnectionClose(WPARAM, LPARAM);   // a connection has been closed
        LRESULT OnNetworkData(WPARAM, LPARAM); // received new message
        LRESULT OnConnectionMade(WPARAM, LPARAM); // pending connection has been established
        LRESULT OnNetworkError(WPARAM, LPARAM);   // a network error occurred

        DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedConnectionstatus();
	afx_msg void OnLbnSelchangeRecord();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDUCLIENTDLG_H__1337A90A_C2AC_11D1_92AD_00C04FAD895E__INCLUDED_)
