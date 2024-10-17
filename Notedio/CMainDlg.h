
// CMainDlg.h: 头文件
//

#pragma once


// CMainDlg 对话框
static UINT WM_FINDREPLACE = ::RegisterWindowMessage(FINDMSGSTRING);
class CMainDlg : public CDialogEx
{
// 构造


public:
	CMainDlg(CWnd* pParent = nullptr);	// 标准构造函数
	LOGFONT m_lf;
	HACCEL m_hAccel;
	void loadFile(CString &fileName);
	int fEncodeType ;
	CString m_fname;
	CString GetSelText();
	CFindReplaceDialog* m_pFRDlg;
	void SearchDown();
	void SearchUp();
	int ReverseFind(const CString& content,const CString& target,int& iStart );
	void ReplaceCurrent();
	void ReplaceAll();
	void Settings();
	void saveAs();
	CBrush m_EditBrush;
	COLORREF m_TxtColor;
	COLORREF m_BkColor;
	


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAINDLG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnFindReplace(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFopen();
	afx_msg void OnFsave();
	afx_msg void OnEcopy();
	afx_msg void OnEpasty();
	afx_msg void OnEundo();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnExit();
	afx_msg void OnCut();
	afx_msg void OnUpper();
	afx_msg void OnXdeffont();
	afx_msg void OnLower();
	afx_msg void OnNewwnd();
	afx_msg void OnFsavea();
	afx_msg void OnNewf();
	afx_msg void OnSearch();
	afx_msg void OnPrintp();
	afx_msg void OnXdate();
	afx_msg void OnSearchOnNet();
	afx_msg void OnSelectall();
	afx_msg void OnXautoline();
	afx_msg void OnXfont();
	afx_msg void OnReplace();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBkcolor();
	afx_msg void OnFontcolor();
};
