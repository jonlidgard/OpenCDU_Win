// CDUServerDlg.h : header file
//

#if !defined(AFX_CDUSERVERDLG_H__2E3300EA_C298_11D1_92AD_00C04FAD895E__INCLUDED_)
#define AFX_CDUSERVERDLG_H__2E3300EA_C298_11D1_92AD_00C04FAD895E__INCLUDED_

#include "Listens.h"    // Added by ClassView
#include "resource.h"
#include "Log.h"
#include "SimComm.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CCDUServerDlg dialog

class CCDUServerDlg : public CDialog
{
// Construction
    public:
       CCDUServerDlg(CWnd* pParent = NULL);  // standard constructor
       // socket member that listens for new connections
       CListensoc m_listensoc;

    protected:
       CFont font;
       int m_close;
	   SimComm *pSimComm;
	   BOOL logNetworkTraffic;
	   void ReadIniFile();
       BOOL m_running;  // listener has been created
       BOOL CreateListener();
       // a count of the number of open connections
       int m_open;

       CDWordArray m_threadIDs;
       BOOL m_MainWndIsClosing; 

// Dialog Data
        //{{AFX_DATA(CCDUServerDlg)
        enum { IDD = IDD_CDUSERVER_DIALOG };
//        CStatic x_Port;
//        CButton c_Run;
//       CButton c_Close;
        CLog        c_Record;
        //}}AFX_DATA

        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CCDUServerDlg)
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support
        //}}AFX_VIRTUAL

// Implementation
protected:
        HICON m_hIcon;
        BOOL CleanupThreads();
		UINT portnum;
        void updateControls();
        
        // Generated message map functions
        //{{AFX_MSG(CCDUServerDlg)
        virtual BOOL OnInitDialog();
        afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
        afx_msg void OnClose();
        virtual void OnOK();
        virtual void OnCancel();
        afx_msg void OnSize(UINT nType, int cx, int cy);
        //}}AFX_MSG

        LRESULT OnInfo(WPARAM, LPARAM);
        LRESULT OnThreadStart(WPARAM, LPARAM);  
        LRESULT OnThreadClose(WPARAM, LPARAM);
        LRESULT OnNetworkData(WPARAM, LPARAM);
        LRESULT OnNetworkError(WPARAM, LPARAM);
        LRESULT OnSendComplete(WPARAM, LPARAM);
		LRESULT OnSimConnectError(WPARAM, LPARAM);
		LRESULT OnSimConnectInfo(WPARAM, LPARAM);
		LRESULT OnSimConnectData(WPARAM, LPARAM);

        DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLbnSelchangeRecord();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDUSERVERDLG_H__2E3300EA_C298_11D1_92AD_00C04FAD895E__INCLUDED_)
