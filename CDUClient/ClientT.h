// ClientT.h : header file
//
// of Microsoft Technical Support, Developer Support
// Copyright (c) 1998 Microsoft Corporation. All rights reserved.

#if !defined(AFX_CLIENTT_H__B7C54BD2_A555_11D0_8996_00AA00B92B2E__INCLUDED_)
#define AFX_CLIENTT_H__B7C54BD2_A555_11D0_8996_00AA00B92B2E__INCLUDED_

#include "ConnectS.h"   // Added by ClassView
#include "SocketThread.h"
#include "ClientSocket.h"
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CClientThread thread

class CClientThread : public CSocketThread
{
        DECLARE_DYNCREATE(CClientThread)
protected:
        CClientThread();           // protected constructor used by dynamic creation

// Operations
public:
        void SetServerName(const CString & name) { m_ServerName = name; }
        void SetPort(UINT p) { m_port = p; }
// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CClientThread)
        protected:
        virtual BOOL InitInstance();
        virtual int ExitInstance();
        //}}AFX_VIRTUAL

// Implementation
protected:
        virtual CConnectSoc * GetSocket() { return &m_socket; }
        virtual ~CClientThread();
        CClientSocket m_socket;
        CString m_ServerName;

        UINT m_port;
        
        // Generated message map functions
        //{{AFX_MSG(CClientThread)
                // NOTE - the ClassWizard will add and remove member functions here.
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTT_H__B7C54BD2_A555_11D0_8996_00AA00B92B2E__INCLUDED_)
