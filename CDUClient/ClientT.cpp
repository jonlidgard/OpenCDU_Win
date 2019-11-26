// ClientT.cpp : implementation file
//
// Microsoft Technical Support, Developer Support
// Copyright (c) 1998 Microsoft Corporation. All rights reserved.
// Rewritten to Best Practice by Joseph M. Newcomer, Mar 2007

#include "stdafx.h"
#include "ClientT.h"
#include "Port.h"
#include "Messages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientThread

IMPLEMENT_DYNCREATE(CClientThread, CSocketThread)

/****************************************************************************
*                        CClientThread::CClientThread
*                        CClientThread::~CClientThread
* Effect: 
*       Constructor, destructor
****************************************************************************/

CClientThread::CClientThread() :
   m_port(0)
{
}

CClientThread::~CClientThread()
{
}

/****************************************************************************
*                         CClientThread::InitInstance
* Result: BOOL
*       TRUE if successful
*       FALSE if error
* Effect: 
*       Creates the socket and connects it to the server
****************************************************************************/

BOOL CClientThread::InitInstance()
   {
    if(!CSocketThread::InitInstance())
       return FALSE;
    
    if(!m_socket.Create())
       { /* failed */
        DWORD err = ::GetLastError();
        ASSERT(target != NULL);
        if(target != NULL)
           target->PostMessage(UWM_NETWORK_ERROR, (WPARAM)err, (LPARAM)::GetCurrentThreadId());
        return FALSE;
       } /* failed */

    if(m_port == 0)
       m_port = PORT_NUM; // default port number
    // Try to connect to the peer
    if (m_socket.Connect(m_ServerName, m_port) == 0)
       { /* connect failed */
        DWORD err = ::GetLastError();
        if (err != WSAEWOULDBLOCK)
           { /* serious failure */
            //error in the connection process, terminate myself
            ASSERT(target != NULL);
            if(target != NULL)
               target->PostMessage(UWM_NETWORK_ERROR, (WPARAM)err, (LPARAM)::GetCurrentThreadId());
            return FALSE;
           } /* serious failure */
       } /* connect failed */

    return TRUE;
   }

/****************************************************************************
*                         CClientThread::ExitInstance
* Result: int
*       Exit code to report
* Effect: 
*       Notifies the main window that the thread has finished
****************************************************************************/

int CClientThread::ExitInstance()
   {
    return CSocketThread::ExitInstance();
   }

/****************************************************************************
*                                 Message Map
****************************************************************************/

BEGIN_MESSAGE_MAP(CClientThread, CSocketThread)
        //{{AFX_MSG_MAP(CClientThread)
                // NOTE - the ClassWizard will add and remove mapping macros here.
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClientThread message handlers
