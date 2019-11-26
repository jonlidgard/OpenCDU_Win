// CDUServer.h : main header file for the CDUSERVER application
//

#if !defined(AFX_CDUSERVER_H__2E3300E8_C298_11D1_92AD_00C04FAD895E__INCLUDED_)
#define AFX_CDUSERVER_H__2E3300E8_C298_11D1_92AD_00C04FAD895E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCDUServerApp:
// See CDUServer.cpp for the implementation of this class
//

class CCDUServerApp : public CWinApp
{
public:
	CCDUServerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDUServerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCDUServerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDUSERVER_H__2E3300E8_C298_11D1_92AD_00C04FAD895E__INCLUDED_)
