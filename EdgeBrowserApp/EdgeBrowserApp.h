// EdgeBrowserApp.h : main header file for the PROJECT_NAME application
//

#pragma once

//#ifndef __AFXWIN_H__
//	#error "include 'pch.h' before including this file for PCH"
//#endif
//#include "stdafx.h"
#include "resource.h"		// main symbols
//#include "pch.h"

#ifdef __windows__
#undef __windows__
#endif

// CEdgeBrowserAppApp:
// See EdgeBrowserApp.cpp for the implementation of this class
//

class CEdgeBrowserAppApp : public CWinApp
{
public:
	CEdgeBrowserAppApp();

	// Overrides
	//BOOL ProcessMessageFilter(int code, LPMSG pMsg);

public:
	virtual BOOL InitInstance();
	//static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CEdgeBrowserAppApp theApp;
