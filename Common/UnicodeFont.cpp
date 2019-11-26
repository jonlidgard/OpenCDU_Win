#include "stdafx.h"
#include "UnicodeFont.h"

/****************************************************************************
*                              CreateUnicodeFont
* Inputs:
*       CFont & font: Font to be created
* Result: BOOL
*       TRUE if successful
*       FALSE if error
* Effect: 
*       Creates a Unicode-compatible font; uses Arial Unicode MS
* Notes:
*       In a production piece of code I would provide a CFontDialog option and
*       save the font information in the Registry
****************************************************************************/

BOOL CreateUnicodeFont(CWnd * w, CFont & font)
    {
     CFont * f = w->GetFont();
     ASSERT(f != NULL);
     LOGFONT lf;
     f->GetLogFont(&lf);
     StringCchCopy(lf.lfFaceName, sizeof(lf.lfFaceName)/sizeof(TCHAR), _T("Courier New"));
     return font.CreateFontIndirect(&lf);
    } // CreateUnicodeFont
