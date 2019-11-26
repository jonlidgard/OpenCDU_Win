// CDUClient.h : main header file for the CDUCLIENT application
//

#if !defined(AFX_CDUCLIENT_H__1337A908_C2AC_11D1_92AD_00C04FAD895E__INCLUDED_)
#define AFX_CDUCLIENT_H__1337A908_C2AC_11D1_92AD_00C04FAD895E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCDUClientApp:
// See CDUClient.cpp for the implementation of this class
//

class CCDUClientApp : public CWinApp
{
public:
	CCDUClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDUClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCDUClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDUCLIENT_H__1337A908_C2AC_11D1_92AD_00C04FAD895E__INCLUDED_)
