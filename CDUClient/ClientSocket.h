#if !defined(AFX_CLIENTSOCKET_H__50DD2836_D1A3_4D8E_B696_6B76684F6572__INCLUDED_)
#define AFX_CLIENTSOCKET_H__50DD2836_D1A3_4D8E_B696_6B76684F6572__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClientSocket.h : header file
//

#include "Connects.h"

/////////////////////////////////////////////////////////////////////////////
// CClientSocket command target

class CClientSocket : public CConnectSoc
{
public:
        CClientSocket();
        virtual ~CClientSocket();
protected:
        virtual void ProcessReceive(CByteArray & data);
// Overrides
protected:
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CClientSocket)
        //}}AFX_VIRTUAL

        // Generated message map functions
        //{{AFX_MSG(CClientSocket)
                // NOTE - the ClassWizard will add and remove member functions here.
        //}}AFX_MSG
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTSOCKET_H__50DD2836_D1A3_4D8E_B696_6B76684F6572__INCLUDED_)
