
// launcherDlg.h : header file
//

#pragma once
#include "afxwin.h"


// ClauncherDlg dialog
class ClauncherDlg : public CDialogEx
{
// Construction
public:
	ClauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LAUNCHER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox m_videoDriverCombo;

	void init();
	void save_ini();
	CComboBox m_screenSizeCombo;
	afx_msg void OnCbnSelchangeCombo2();
	CButton m_fullscreenCheck;
	afx_msg void OnBnClickedCheck1();
};
