// EdgeBrowserAppDlg.cpp
// WebView2 Application Dialog

#include "stdafx.h"
#include "afxdialogex.h"
//#include "framework.h"
#include "EdgeBrowserApp.h"
#include "EdgeBrowserAppDlg.h"
#include "winuser.h"
#include "ViewComponent.h"
#include "Resource.h"

//#include "pch.h"
#include <ShObjIdl_core.h>
#include <Shellapi.h>
#include <ShlObj_core.h>
//#include <wrl.h>
//#include <winrt/windows.system.h>

#ifdef __windows__
#undef __windows__
#endif

static constexpr UINT s_runAsyncWindowMessage = WM_APP;

// CEdgeBrowserAppDlg dialog

CEdgeBrowserAppDlg::CEdgeBrowserAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EDGEBROWSERAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_hInstance = GetModuleHandle(NULL);

}

void CEdgeBrowserAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEdgeBrowserAppDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_GO, &CEdgeBrowserAppDlg::OnBnClickedButtonGo)
END_MESSAGE_MAP()

void CEdgeBrowserAppDlg::OnSize(UINT a, int b, int c)
{
	ResizeEverything();
}

// CEdgeBrowserAppDlg message handlers

BOOL CEdgeBrowserAppDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HRESULT hresult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	SetWindowLongPtr(this->GetSafeHwnd(), GWLP_USERDATA, (LONG_PTR)this);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetDlgItemText(IDC_STATIC_NAV, L"");
	SetWindowPos(NULL, 0, 0, 500, 480, SWP_NOMOVE | SWP_NOZORDER);

	InitializeWebView();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CEdgeBrowserAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{

	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEdgeBrowserAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEdgeBrowserAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
// Register the Win32 window class for the app window.

void CEdgeBrowserAppDlg::ResizeEverything()
{
	RECT availableBounds = { 0 };
	GetClientRect(&availableBounds);
	// ClientToScreen(&availableBounds);

	availableBounds.bottom -= 25; // bottom panel

	if (auto view = GetComponent<ViewComponent>())
	{
		view->SetBounds(availableBounds);
	}
}

void CEdgeBrowserAppDlg::RunAsync(std::function<void()> callback)
{
	auto* task = new std::function<void()>(callback);
	PostMessage(s_runAsyncWindowMessage, reinterpret_cast<WPARAM>(task), 0);
}

void CEdgeBrowserAppDlg::InitializeWebView()
{

	CloseWebView();
	m_dcompDevice = nullptr;


	HRESULT hr2 = DCompositionCreateDevice2(nullptr, IID_PPV_ARGS(&m_dcompDevice));
	if (!SUCCEEDED(hr2))
	{
		AfxMessageBox(L"Attempting to create WebView using DComp Visual is not supported.\r\n"
			"DComp device creation failed.\r\n"
			"Current OS may not support DComp.\r\n"
			"Create with Windowless DComp Visual Failed", MB_OK);
		return;
	}


#ifdef USE_WEBVIEW2_WIN10
	m_wincompCompositor = nullptr;
#endif
	LPCWSTR subFolder = nullptr;
	auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
	options->put_AllowSingleSignOnUsingOSPrimaryAccount(FALSE);


	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(subFolder, nullptr, options.Get(), Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(this, &CEdgeBrowserAppDlg::OnCreateEnvironmentCompleted).Get());
	//HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(subFolder, nullptr, options.Get(), Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(this, &CEdgeBrowserAppDlg::OnCreateEnvironmentCompleted).Get());

	if (!SUCCEEDED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			TRACE("Couldn't find Edge installation. Do you have a version installed that is compatible with this ");
		}
		else
		{
			AfxMessageBox(L"Failed to create webview environment");
		}
	}
}

HRESULT CEdgeBrowserAppDlg::DCompositionCreateDevice2(IUnknown* renderingDevice, REFIID riid, void** ppv)
{
	HRESULT hr = E_FAIL;
	static decltype(::DCompositionCreateDevice2)* fnCreateDCompDevice2 = nullptr;
	if (fnCreateDCompDevice2 == nullptr)
	{
		HMODULE hmod = ::LoadLibraryEx(L"dcomp.dll", nullptr, 0);
		if (hmod != nullptr)
		{
			fnCreateDCompDevice2 = reinterpret_cast<decltype(::DCompositionCreateDevice2)*>(
				::GetProcAddress(hmod, "DCompositionCreateDevice2"));
		}
	}
	if (fnCreateDCompDevice2 != nullptr)
	{
		hr = fnCreateDCompDevice2(renderingDevice, riid, ppv);
	}
	return hr;
}

void CEdgeBrowserAppDlg::CloseWebView(bool cleanupUserDataFolder)
{

	if (m_controller)
	{
		m_controller->Close();
		m_controller = nullptr;
		m_webView = nullptr;
	}
	m_webViewEnvironment = nullptr;
	if (cleanupUserDataFolder)
	{
		//Clean user data        
	}
}

HRESULT CEdgeBrowserAppDlg::OnCreateEnvironmentCompleted(HRESULT result, ICoreWebView2Environment* environment)
{
	m_webViewEnvironment = environment;
	m_webViewEnvironment->CreateCoreWebView2Controller(this->GetSafeHwnd(), Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(this, &CEdgeBrowserAppDlg::OnCreateCoreWebView2ControllerCompleted).Get());

	return S_OK;
}

HRESULT CEdgeBrowserAppDlg::OnCreateCoreWebView2ControllerCompleted(HRESULT result, ICoreWebView2Controller* controller)
{
	if (result == S_OK)
	{
		m_controller = controller;
		wil::com_ptr<ICoreWebView2> coreWebView2;
		m_controller->get_CoreWebView2(&coreWebView2);
		coreWebView2.query_to(&m_webView);

		NewComponent<ViewComponent>(
			this, m_dcompDevice.get(),
#ifdef USE_WEBVIEW2_WIN10
			m_wincompCompositor,
#endif
			m_creationModeId == IDM_CREATION_MODE_TARGET_DCOMP);

		//! [DocumentTitleChanged]
		// Register a handler for the DocumentTitleChanged event.
		// This handler just announces the new title on the window's title bar.
		m_webView->add_DocumentTitleChanged(
			Microsoft::WRL::Callback<ICoreWebView2DocumentTitleChangedEventHandler>(
				[this](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
					wil::unique_cotaskmem_string title;
					sender->get_DocumentTitle(&title);
					SetWindowText(title.get());
					return S_OK;
				})
			.Get(),
					&m_documentTitleChangedToken);
		//! [/DocumentTitleChanged]

		//! [SourceChanged]
		// Register a handler for the SourceChanged event.
		m_webView->add_SourceChanged(
			Microsoft::WRL::Callback<ICoreWebView2SourceChangedEventHandler>(
				[this](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
					wil::unique_cotaskmem_string source;
					sender->get_Source(&source);
					SetDlgItemText(IDC_EDIT_URL, source.get());
					return S_OK;
				})
			.Get(),
					&m_sourceChangedToken);
		//! [/SourceChanged]

		//! [AcceleratorKeyPressed]
		// Register a handler for the AcceleratorKeyPressed event.
		GetWebViewController()->add_AcceleratorKeyPressed(
			Microsoft::WRL::Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
				[this](ICoreWebView2Controller* sender, ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT {
					UINT key;
					COREWEBVIEW2_PHYSICAL_KEY_STATUS stat;
					args->get_VirtualKey(&key);
					args->get_PhysicalKeyStatus(&stat);
					// Go to address bar
					if (key == VK_F6 || (key == 'L' && GetKeyState(VK_CONTROL) < 0))
					{
						args->put_Handled(TRUE);
						if (!stat.IsKeyReleased)
						{
							GotoDlgCtrl(GetDlgItem(IDC_EDIT_URL));
						}
					}
					// Stop page loading
					else if (key == VK_ESCAPE)
					{
						CString s;
						GetDlgItemText(IDC_STATIC_NAV, s);
						if (s.Compare(L"Loading...") == 0)
						{
							args->put_Handled(TRUE);
							if (!stat.IsKeyReleased)
							{
								GetWebView()->Stop();
							}
						}
					}
					else
					{
						args->put_Handled(FALSE);
					}
					return S_OK;
				})
			.Get(),
					&m_acceleratorKeyPressedToken);
		//! [/AcceleratorKeyPressed]

		//! [NavigationStarting]
		// Register a handler for the NavigationStarting event.
		GetWebView()->add_NavigationStarting(
			Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
				[this](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
					SetDlgItemText(IDC_STATIC_NAV, L"Loading...");
					return S_OK;
				})
			.Get(),
					&m_navigationStartingToken);
		//! [/NavigationStarting]

		//! [NavigationCompleted]
		// Register a handler for the NavigationCompleted event.
		GetWebView()->add_NavigationCompleted(
			Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
				[this](ICoreWebView2* sender, IUnknown* args) -> HRESULT {
					// clear loading indicator
					SetDlgItemText(IDC_STATIC_NAV, L"");
					return S_OK;
				})
			.Get(),
					&m_navigationCompletedToken);
		//! [/NavigationCompleted]

		//ICoreWebView2Settings* settings;
		//GetWebView()->get_Settings(&settings);
		//settings->put_IsBuiltInErrorPageEnabled(TRUE);
		//settings->put_IsStatusBarEnabled(TRUE);

		// init WebView2 by dummy navigation
		Navigate(L"about:blank");

		// https://www.whatsmyua.info/
		// https://www.virustotal.com/gui/home/upload
		if (lstrlen(theApp.m_lpCmdLine))
		{
			Navigate(theApp.m_lpCmdLine);
		}
	}
	else
	{
		TRACE("Failed to create webview");
	}
	return S_OK;
}

void CEdgeBrowserAppDlg::Navigate(LPCTSTR url)
{
	HRESULT hresult = GetWebView()->Navigate(url);
	if (hresult == S_OK)
	{
		TRACE("Web Page Opened Successfully");
		ResizeEverything();
		GetWebViewController()->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
	}
	else
	{
		CString s;
		s.Format(L"<html><head><title>Navigation Error: %s</title></head><body><h2>Navigation Error</h2>%#x<br><hr>%s</body></html>", url, hresult, url);
		GetWebView()->NavigateToString(s);
	}
}

void CEdgeBrowserAppDlg::OnBnClickedButtonGo()
{
	CString url;
	GetDlgItemText(IDC_EDIT_URL, url);
	Navigate(url);
}

BOOL CEdgeBrowserAppDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_F6)
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			GetWebViewController()->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
		}
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
