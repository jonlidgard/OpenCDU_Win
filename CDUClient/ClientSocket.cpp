// ClientSocket.cpp : implementation file
//
// Rewritten to Best Practice by Joseph M. Newcomer, Mar 2007

#include "stdafx.h"
#include "ClientSocket.h"
#include "Convert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket()
{
}

CClientSocket::~CClientSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CAsyncSocket)
        //{{AFX_MSG_MAP(CClientSocket)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif  // 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions

/****************************************************************************
*                        CClientSocket::ProcessReceive
* Inputs:
*       CByteArray & data: Raw data received
* Result: void
*       
* Effect: 
*       This processes the received data
****************************************************************************/

void CClientSocket::ProcessReceive(CByteArray & data)
    {
     SendDataToOwner(data);
    } // CClientSocket::ProcessReceive
