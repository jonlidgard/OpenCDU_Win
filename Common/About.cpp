#include "stdafx.h"
#include "resource.h"
#include "About.h"

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
   {
        //{{AFX_DATA_INIT(CAboutDlg)
        //}}AFX_DATA_INIT
   }

/****************************************************************************
*                          CAboutDlg::DoDataExchange
* Inputs:
*       CDataExchange * pDX:
* Result: void
*       
* Effect: 
*       Binds controls to variables
****************************************************************************/

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
   {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
   }

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
      //{{AFX_MSG_MAP(CAboutDlg)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/****************************************************************************
*                           CAboutDlg::OnInitDialog
* Result: BOOL
*       TRUE, always
* Effect: 
*       Initializes the dialog based on the compiler configuration settings
****************************************************************************/

BOOL CAboutDlg::OnInitDialog() 
   {
    CDialog::OnInitDialog();
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

