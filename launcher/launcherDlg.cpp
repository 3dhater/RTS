
// launcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "launcherDlg.h"
#include "afxdialogex.h"

#include <vector>
#include <filesystem>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct CVideoMode
{
	CString m_title;
	int m_x;
	int m_y;
};
std::vector<CVideoMode> g_videoModes;
int g_screenSizeX = 800;
int g_screenSizeY = 600;
int g_isFullscreen = 0;

// ClauncherDlg dialog

CString g_selectedVideoDriverName;

ClauncherDlg::ClauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LAUNCHER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ClauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_videoDriverCombo);
	DDX_Control(pDX, IDC_COMBO2, m_screenSizeCombo);
	DDX_Control(pDX, IDC_CHECK1, m_fullscreenCheck);
}

BEGIN_MESSAGE_MAP(ClauncherDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO1, &ClauncherDlg::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &ClauncherDlg::OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_CHECK1, &ClauncherDlg::OnBnClickedCheck1)
END_MESSAGE_MAP()


// ClauncherDlg message handlers

BOOL ClauncherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	init();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void ClauncherDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ClauncherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ClauncherDlg::init()
{
	std::vector<std::wstring> vidDrivers;

	for (auto & entry : std::tr2::sys::directory_iterator("bin32/"))
	{
		auto path = entry.path();
		if (path.has_extension())
		{
			auto ex = path.extension();
			if (ex == ".yyvd")
			{
				vidDrivers.push_back(path.filename().generic_wstring());
				m_videoDriverCombo.AddString(path.filename().c_str());
			}
		}
	}

	int cursel = 0;
	wchar_t videoDriverTypeBuffer[50];
	auto r = GetPrivateProfileString(L"video", L"driver", 0, videoDriverTypeBuffer, 50, L".\\params.ini");
	auto err = GetLastError();
	if (err)
	{
		CString str = L"Error: ";
		str.AppendFormat(L"%i", err);
		MessageBox(str.GetBuffer());
	}

	
	std::tr2::sys::path pth;
	pth = videoDriverTypeBuffer;
	for (int i = 0; i < vidDrivers.size(); ++i)
	{
		if (vidDrivers[i] == pth.generic_wstring())
		{
			cursel = i;
			break;
		}
	}

	m_videoDriverCombo.GetLBText(cursel, g_selectedVideoDriverName);
	m_videoDriverCombo.SetCurSel(cursel);

	int i = 0;
	DEVMODE dm;
	while (EnumDisplaySettings(NULL, i, &dm))
	{
		CString str;
		str.AppendFormat(L"%ix%i", dm.dmPelsWidth, dm.dmPelsHeight);
		CVideoMode vm;

		for (auto s : g_videoModes)
		{
			if (s.m_title == str)
				goto skip;
		}

		vm.m_title = str;
		vm.m_x = dm.dmPelsWidth;
		vm.m_y = dm.dmPelsHeight;
		g_videoModes.push_back(vm);

		//m_screenSizeCombo.AddString(str.GetBuffer());
	skip:;
		++i;
	}

	struct {
		bool operator()(const CVideoMode& a, const CVideoMode& b) const { return a.m_x < b.m_x; }
	} customLess;

	std::stable_sort(g_videoModes.begin(), g_videoModes.end(), customLess);

	for (auto s : g_videoModes)
	{
		m_screenSizeCombo.AddString(s.m_title.GetBuffer());
	}

	g_screenSizeX = GetPrivateProfileInt(L"screen", L"x", 800, L".\\params.ini");
	g_screenSizeY = GetPrivateProfileInt(L"screen", L"y", 600, L".\\params.ini");
	for (int i = 0; i < g_videoModes.size(); ++i)
	{
		auto & vm = g_videoModes[i];
		if (vm.m_x == g_screenSizeX && vm.m_y == g_screenSizeY)
		{
			m_screenSizeCombo.SetCurSel(i);
			break;
		}
	}

	g_isFullscreen = GetPrivateProfileInt(L"screen", L"fullscreen", 0, L".\\params.ini");
	if (g_isFullscreen == 1)
	{
		m_fullscreenCheck.SetCheck(1);
	}
}

void ClauncherDlg::save_ini()
{
	WritePrivateProfileString(L"video", L"driver", g_selectedVideoDriverName.GetBuffer(), L".\\params.ini");

	CString str;
	str.AppendFormat(L"%i", g_screenSizeX);
	WritePrivateProfileString(L"screen", L"x", str.GetBuffer(), L".\\params.ini");
	str.Empty();
	str.AppendFormat(L"%i", g_screenSizeY);
	WritePrivateProfileString(L"screen", L"y", str.GetBuffer(), L".\\params.ini");
	str.Empty();
	str.AppendFormat(L"%i", g_isFullscreen);
	WritePrivateProfileString(L"screen", L"fullscreen", str.GetBuffer(), L".\\params.ini");
}
void ClauncherDlg::OnCbnSelchangeCombo1()
{
	m_videoDriverCombo.GetLBText(m_videoDriverCombo.GetCurSel(), g_selectedVideoDriverName);
}


void ClauncherDlg::OnCbnSelchangeCombo2()
{
	auto & vm = g_videoModes[m_screenSizeCombo.GetCurSel()];
	g_screenSizeX = vm.m_x;
	g_screenSizeY = vm.m_y;
}


void ClauncherDlg::OnBnClickedCheck1()
{
	g_isFullscreen = 0;
	if (m_fullscreenCheck.GetState() & BST_CHECKED)
		g_isFullscreen = 1;
}
