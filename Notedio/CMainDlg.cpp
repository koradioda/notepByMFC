
// CMainDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Notexl.h"
#include "CMainDlg.h"
#include "afxdialogex.h"
#include "xlutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
static LOGFONT lf{};  // 记录字体的大小

// CMainDlg 对话框

CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NOTEXL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	fEncodeType = -1;
}

void CMainDlg::loadFile(CString &fileName)
{
	FILE* fp = _tfopen(_bstr_t(fileName), _T("rb"));  // 二进制打开文件
	UINT fLen = GetFileLengthxl(fp);  // 计算长度  单位为字节
	// fLen = fLen / 2 + 1;  // + 防止 UTF-8 和 ansii 文件不是双字节
	TCHAR* cFile = new TCHAR[fLen + 3]{ 0 };   // 一个字符一个字符存储
	// if(fread(cFile,sizeof(TCHAR), fLen+1, fp)<0)  
	if (fread(cFile, 1, fLen, fp) < 0)   // 直接按字节数读入缓存区
		AfxMessageBox(_T("文本读取错误！"));;  // 读取所有二进制字节
	unsigned char* c = (unsigned char*)cFile;
	// 判断文件类型
	fEncodeType = GetEncodeType(c);
	TCHAR* str = nullptr;
	switch (fEncodeType)
	{
	case ENCODE_BE:   // unicode BE
		ConvertBig(cFile);
	case ENCODE_LE:  // unicode LE;    unicode le can show without any operate
		SetDlgItemText(IDC_MEDIT, cFile);
		break;
	case ENCODE_BOMU8:  // bom utf-8
		str = UTF8ToUnicode((char*)c + 3);
		break;
	case ENCODE_NOBOMU8:	// without bom utf-8
		str = UTF8ToUnicode((char*)c);
		break;

	case ENCODE_ANSII:  // ansii 编码
		str = ANSIToUnicode((char*)c);
		break;

	default:
	{
		AfxMessageBox(_T("未知编码"));
		fclose(fp);
		delete[] cFile;
	}
	return;
	}

	if (str != nullptr)
	{
		SetDlgItemText(IDC_MEDIT, str);
		delete str;
	}

	fclose(fp);
	delete[] cFile;
}

CString CMainDlg::GetSelText()
{
	int nStar, nEnd;
	auto pedit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pedit->GetSel(nStar, nEnd);  // 获取选中的起始位置
	CString str;
	pedit->GetWindowText(str);
	   // 截取
	return str.Mid(nStar, nEnd - nStar);
}


void CMainDlg::SearchDown()
{
	CString findstr = m_pFRDlg->GetFindString();
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	CString content;
	static int iStart = 0;
	int len = findstr.GetLength();
	pEdit->GetWindowText(content);
	iStart = content.Find(findstr, iStart);
	if (iStart<0 || iStart>content.GetLength())
		AfxMessageBox(_T("找不到:")+ findstr);
	pEdit->SetSel(iStart, iStart + len, TRUE);
	++iStart;
	pEdit->SetFocus();
}

void CMainDlg::SearchUp()
{
	CString findstr = m_pFRDlg->GetFindString();
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	CString content;
	pEdit->GetWindowText(content);
	static int iStart = content.GetLength();
	iStart = ReverseFind(content,findstr, iStart);
	if (iStart < 0 || iStart > content.GetLength())
	{
		AfxMessageBox(_T("找不到:")+findstr);
		iStart = content.GetLength();
		return;
	}
	pEdit->SetSel(iStart, iStart + findstr.GetLength(),TRUE);
	--iStart;
	pEdit->SetFocus();
}

int CMainDlg::ReverseFind(const CString& content, const CString& target, int& iStart)
{
	int nlen = content.GetLength();
	int ntarget = target.GetLength();
	int i = nlen - iStart;
	if (ntarget > nlen || iStart > nlen) 
		return -1;
	if (iStart > nlen - ntarget) 
		iStart = nlen - ntarget;
	for (int i = iStart; i >= 0; --i)
	{
		BOOL find = true;
		for (int j = 0; j < ntarget; ++j)
		{
			if (content[i + j] != target[j])
			{
				find = false;
				break;
			}
			
		}
		if (find)
			return i;
	}
	return -1;
}

void CMainDlg::ReplaceCurrent()
{
	if (!IsWindow(m_pFRDlg->GetSafeHwnd()))
		return;
	CString newstr;
	newstr = m_pFRDlg->GetReplaceString();
	CString oldstr;
	oldstr = m_pFRDlg->GetFindString();
	CString content;

	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pEdit->GetWindowText(content);
	static int iStart = 0;
	iStart = content.Find(oldstr, iStart);
	if (iStart<0 || iStart >content.GetLength())
	{
		AfxMessageBox(_T("未找到:") + oldstr);
		iStart = 0;
		return;
	}
	else {
		//CString before = content.Left(iStart);
		//CString after = content.Mid(iStart + oldstr.GetLength());
		// content.Replace(content.Mid(iStart, oldstr.GetLength()), newstr);
		//content = before + newstr + after;   // 截取+拼接
		pEdit->SetSel(iStart, iStart + oldstr.GetLength());
		// pEdit->SetWindowText(content);
		pEdit->ReplaceSel(newstr);
		pEdit->SetSel(iStart, iStart + newstr.GetLength());
		++iStart;
		pEdit->SetFocus();
	}
}

void CMainDlg::ReplaceAll()
{
	if (!IsWindow(m_pFRDlg->GetSafeHwnd()))
		return;
	CString newstr;
	newstr = m_pFRDlg->GetReplaceString();
	CString oldstr;
	oldstr = m_pFRDlg->GetFindString();
	CString content;

	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pEdit->GetWindowText(content);
	content.Replace(oldstr, newstr);
	pEdit->SetWindowText(content);
	pEdit->SetFocus();
}




void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FOPEN, &CMainDlg::OnFopen)
	ON_COMMAND(ID_FSAVE, &CMainDlg::OnFsave)
	ON_COMMAND(ID_ECOPY, &CMainDlg::OnEcopy)
	ON_COMMAND(ID_EPASTY, &CMainDlg::OnEpasty)
	ON_COMMAND(ID_EUNDO, &CMainDlg::OnEundo)
	ON_WM_DROPFILES()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EXIT, &CMainDlg::OnExit)
	ON_COMMAND(ID_CUT, &CMainDlg::OnCut)
	ON_COMMAND(ID_UPPER, &CMainDlg::OnUpper)
	ON_COMMAND(ID_XDEFFONT, &CMainDlg::OnXdeffont)
	ON_COMMAND(ID_LOWER, &CMainDlg::OnLower)
	ON_COMMAND(ID_NEWWND, &CMainDlg::OnNewwnd)
	ON_COMMAND(ID_FSAVEA, &CMainDlg::OnFsavea)
	ON_COMMAND(ID_NEWF, &CMainDlg::OnNewf)
	ON_COMMAND(ID_SEARCH, &CMainDlg::OnSearch)
	ON_COMMAND(ID_PRINTP, &CMainDlg::OnPrintp)
	ON_COMMAND(ID_XDATE, &CMainDlg::OnXdate)
	ON_COMMAND(ID_SEARCHONNET, &CMainDlg::OnSearchOnNet)
	ON_COMMAND(ID_SELECTALL, &CMainDlg::OnSelectall)
	ON_COMMAND(ID_XAUTOLINE, &CMainDlg::OnXautoline)
	ON_COMMAND(ID_XFONT, &CMainDlg::OnXfont)
	ON_REGISTERED_MESSAGE(WM_FINDREPLACE, &CMainDlg::OnFindReplace)
	ON_COMMAND(ID_REPLACE, &CMainDlg::OnReplace)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_BKCOLOR, &CMainDlg::OnBkcolor)
	ON_COMMAND(ID_FONTCOLOR, &CMainDlg::OnFontcolor)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序

void CMainDlg::Settings()
{

}

void CMainDlg::saveAs()
{
	CString content;
	GetDlgItemText(IDC_MEDIT, content);
	TCHAR* source = new TCHAR[content.GetLength() + 3]{ 0 };
	wcscpy(source, content.GetString());

	switch (fEncodeType)
	{
	case ENCODE_BE:			// BE
		ConvertBig(source);   // edit is encode_le, decode ot BE  //转换为BE；
		// break;  // convertTobig(); 转换后直接保存
	case ENCODE_LE:			// LE
	{
		TCHAR header;
		if (ENCODE_BE == fEncodeType) header = 0xfffe;
		if (ENCODE_LE == fEncodeType) header = 0xfeff;
		FILE* fp = _wfopen(m_fname, _T("wb"));
		fwrite(&header, sizeof(TCHAR), 1, fp);  // 写入文件头
		fwrite(source, sizeof(TCHAR), wcslen(source), fp);
		fclose(fp);
		delete[] source;
	}
	break;
	case ENCODE_BOMU8:		// bom u8
		// break;
	case ENCODE_NOBOMU8:	// nobom u8
	{
		char* trans = UnicodeToUTF8(source);
		FILE* fp = fopen(_bstr_t(m_fname), "wb");
		if (ENCODE_BOMU8 == fEncodeType) {
			TCHAR header[2];
			header[0] = 0xbbef;
			header[1] = 0x00bf;
			fwrite(header, 1, 3, fp);
		}
		fwrite(trans, 1, strlen(trans), fp);
		delete[] source;
		delete[] trans;
		fclose(fp);
	}
	break;
	case ENCODE_ANSII:		// ansii
	{
		FILE* fp = fopen(_bstr_t(m_fname), ("wb"));  //   write by binarary 
		char* trans = UnicodeToANSI(source);
		size_t n = fwrite(trans, 1, strlen(trans), fp);
		fclose(fp);
		delete[] source;
		delete[] trans;
	}
	break;
	default:
		// save as...
		// OnFsavea();
		break;
	}
}

BOOL CMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR));  // 快捷键绑定
	m_pFRDlg = new CFindReplaceDialog;
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pEdit->ModifyStyle(NULL, ES_WANTRETURN);
	auto pFt = pEdit->GetFont();
	pFt->GetLogFont(&m_lf);
	
	CWinApp* pApp = AfxGetApp();  // 获取主程序句柄
	CRect rect;
	/*
	rect.left = GetPrivateProfileIntW(_T("RECT"), _T("left"), 0, profileName);
	rect.right = GetPrivateProfileInt(_T("RECT"), _T("right"), 800, profileName);
	rect.top = GetPrivateProfileInt(_T("RECT"), _T("top"), 0, profileName);
	rect.bottom = GetPrivateProfileInt(_T("RECT"), _T("bottom"), 600, profileName);
	*/
	rect.left = pApp->GetProfileInt(_T("RECT"), _T("left"), 0);
	rect.right = pApp->GetProfileInt(_T("RECT"), _T("right"), 800);
	rect.top = pApp->GetProfileInt(_T("RECT"), _T("top"), 0);
	rect.bottom = pApp->GetProfileInt(_T("RECT"), _T("bottom"), 600);
	m_lf.lfHeight = -1* pApp->GetProfileInt(_T("FONT"), _T("size"), -14);
	m_BkColor = pApp->GetProfileInt(_T("COLOR"),_T("background"),0xffffff);
	m_TxtColor = pApp->GetProfileInt(_T("COLOR"), _T("textcolor"), 0x000000);
	m_EditBrush.DeleteObject();
	m_EditBrush.CreateSolidBrush(m_BkColor);
	if(rect.Width() > 0)
		MoveWindow(rect);
	OnXdeffont();  // 默认字体


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMainDlg::OnFindReplace(WPARAM wParam, LPARAM lParam)
{
	if (m_pFRDlg->IsTerminating())
	{
		// 退出后的执行操作
		// m_pFRDlg->EndDialog(0);
	}
	if (m_pFRDlg->FindNext())
	{
		if (m_pFRDlg->SearchDown())
		{
			SearchDown();
		}
		else
		{
			SearchUp();
		}
	}
	if (m_pFRDlg->ReplaceAll())
	{
		ReplaceAll();
	}
	if (m_pFRDlg->ReplaceCurrent())
	{
		ReplaceCurrent();
	}
	return 0;
}


void CMainDlg::OnFopen()
{
	OnXdeffont();
	// TODO: 在此添加命令处理程序代码
	CFileDialog cfd(true);    // true for open file dlg,  false for save file dlg
	CString fileName;
	// cfd.m_ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0\0");
	cfd.m_ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0\0");   // \0 成对出现，第一个\0是提示，第二个表示之前的是过滤器
	if (IDCANCEL == cfd.DoModal()) {
		return ;
	}
	fileName = cfd.GetPathName();
	SetWindowText(fileName);
	loadFile(fileName);

}


void CMainDlg::OnFsave()
{
	// TODO: 在此添加命令处理程序代码
	
	// unicode to ------> fEncodeType   // write by binarary;
	// m_fname.ReverseFind(_T('\\'));

	// fEncodeType;  // 判断文件类型进行保存
	GetWindowText(m_fname);
	CString content;
	GetDlgItemText(IDC_MEDIT, content);  // 获取内容
	TCHAR* source = new TCHAR[content.GetLength()+3]{ 0 };  // utf16-le
	wcscpy(source, content.GetString());
	switch (fEncodeType)
	{
	case ENCODE_BE:			// BE
		ConvertBig(source);   // edit is encode_le, decode ot BE  //转换为BE；
		// break;  // convertTobig(); 转换后直接保存
	case ENCODE_LE:			// LE
	{
		FILE* fp = _wfopen(m_fname,_T("wb"));
		fwrite(source,sizeof(TCHAR), wcslen(source), fp);
		fclose(fp);
		delete[] source;
		}
		break;
	case ENCODE_BOMU8:		// bom u8
		// break;
	case ENCODE_NOBOMU8:	// nobom u8
	{
		char* trans = UnicodeToUTF8(source);
		FILE* fp = fopen(_bstr_t(m_fname), "wb");
		fwrite(trans, 1, strlen(trans), fp);
		delete[] source;
		delete[] trans;
		fclose(fp);
	}
		break;
	case ENCODE_ANSII:		// ansii
	{
		FILE* fp = fopen(_bstr_t(m_fname), ("wb"));  //   write by binarary 
		char* trans = UnicodeToANSI(source);
		size_t n = fwrite(trans, 1, strlen(trans), fp);
		fclose(fp);
		delete[] source;
		delete[] trans;
	}
		break;
	default :
		// save as...
		OnFsavea();
		break;
	}

}


void CMainDlg::OnEcopy()
{
	// TODO: 在此添加命令处理程序代码
	// MessageBox(_T("点击了复制！"), _T("提示"));
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pEdit->Copy();
}


void CMainDlg::OnEpasty()
{
	// TODO: 在此添加命令处理程序代码
	// MessageBox(_T("点击了粘贴！"), _T("提示"));
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pEdit->Paste();
}


void CMainDlg::OnEundo()
{
	// TODO: 在此添加命令处理程序代码
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pEdit->Undo();

}


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (::TranslateAccelerator(GetSafeHwnd(), m_hAccel, pMsg))   // 绑定快捷键
		return true;

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CMainDlg::OnDropFiles(HDROP hDropInfo)
{
	OnXdeffont();
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString fileName;
	int nFiles = DragQueryFile(hDropInfo,0xffffffff,0,0);
	if (nFiles >= 0)
	{
		DragQueryFile(hDropInfo,0, fileName.GetBufferSetLength(256), 256);
		fileName.ReleaseBuffer();
	}// 获取完整文件名
	SetWindowText(fileName);
	loadFile(fileName);
	
	CDialogEx::OnDropFiles(hDropInfo);
}


void CMainDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	if (pEdit)
		pEdit->MoveWindow(0, 0, cx, cy, FALSE);
	// TODO: 在此处添加消息处理程序代码
}


void CMainDlg::OnDestroy()
{

	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
		// 保存配置文件// 如标题，窗口位置信息，文本最后文本信息，编辑框最后的内容，
	// 确定保存的位置  默认是C:windows下

	CRect rect;
	GetWindowRect(rect);
	CString str;
	CWinApp* pApp = AfxGetApp();  // 获取程序句柄，也可以直接使用 theApp
	pApp->WriteProfileInt(_T("RECT"), _T("left"), rect.left);
	pApp->WriteProfileInt(_T("RECT"), _T("right"), rect.right);
	pApp->WriteProfileInt(_T("RECT"), _T("top"), rect.top);
	pApp->WriteProfileInt(_T("RECT"), _T("bottom"), rect.bottom);
	pApp->WriteProfileInt(_T("FONT"), _T("size"), -1*m_lf.lfHeight);
	pApp->WriteProfileInt(_T("COLOR"), _T("background"), m_BkColor);
	pApp->WriteProfileInt(_T("COLOR"), _T("textcolor"), m_TxtColor);
	// pApp->WriteProfileInt(_T("FONT"), _T("size"), 12);

	/*
	str.Format(_T("%d"), rect.left);
	WritePrivateProfileString(_T("RECT"), _T("left"), str, profileName);
	str.Format(_T("%d"), rect.right);
	WritePrivateProfileString(_T("RECT"), _T("right"), str, profileName);
	str.Format(_T("%d"), rect.top);
	WritePrivateProfileString(_T("RECT"), _T("top"), str, profileName);
	str.Format(_T("%d"), rect.bottom);
	WritePrivateProfileString(_T("RECT"), _T("bottom"), str, profileName);  // 写入一组数据  ，可以在OnInitDialog()进行加载
	*/
	
}


void CMainDlg::OnExit()
{
	// TODO: 
	OnCancel();
	//EndDialog(IDCANCEL);
}


void CMainDlg::OnCut()
{
	// TODO: 在此添加命令处理程序代码
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pEdit->Cut();
}


void CMainDlg::OnUpper()
{
	// TODO: 在此添加命令处理程序代码
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	
	lf.lfHeight += (int)(0.2* lf.lfHeight);
	lf.lfWidth += (int)(0.2 * lf.lfWidth);

	CFont cf;
	cf.CreateFontIndirect(&lf);
	pEdit->SetFont(&cf);
}


void CMainDlg::OnXdeffont()
{
	// TODO: 在此添加命令处理程序代码
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	CFont cf;
	cf.CreateFontIndirectW(&m_lf);
	pEdit->SetFont(&cf);
	lf = m_lf;

}


void CMainDlg::OnLower()
{
	// TODO: 在此添加命令处理程序代码
	auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);

	lf.lfHeight -= (int)(0.2 * lf.lfHeight);
	lf.lfWidth -= (int)(0.2 * lf.lfWidth);

	CFont cf;
	cf.CreateFontIndirect(&lf);
	pEdit->SetFont(&cf);
}


void CMainDlg::OnNewwnd()
{
	// TODO: 在此添加命令处理程序代码
	//CString appName = AfxGetApp()->m_pszExeName;
	//appName = _T(".\\") + appName + _T(".exe");
	TCHAR appName[MAX_PATH];
	GetModuleFileName(NULL, appName, MAX_PATH);   // NULL表示获取当前路进程名，非空返回指定模块路径名，成功返回路径长度
	ShellExecute(NULL, _T("open"), appName, NULL, NULL, SW_SHOWNORMAL);
}


void CMainDlg::OnFsavea()
{
	// TODO: 在此添加命令处理程序代码

	CFileDialog cfd(false);
	TCHAR defaultName[MAX_PATH] = _T("新建文件.txt");
	cfd.m_ofn.lpstrFile = defaultName;   // 设置默认文件名
	cfd.m_ofn.lpstrInitialDir = _T(".\\");  // 初始化目录
	cfd.m_ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0");
	cfd.m_ofn.lpstrTitle = _T("保存文件");
	cfd.AddComboBox(IDC_SAVEAS_COMB);
	cfd.AddControlItem(IDC_SAVEAS_COMB, 0, _T("ANSI"));
	cfd.AddControlItem(IDC_SAVEAS_COMB, 1, _T("utf-8"));
	cfd.AddControlItem(IDC_SAVEAS_COMB, 2, _T("utf-8 带bom头"));
	cfd.AddControlItem(IDC_SAVEAS_COMB, 3, _T("U16-LE"));
	cfd.AddControlItem(IDC_SAVEAS_COMB, 4, _T("U16-BE"));
	cfd.SetSelectedControlItem(IDC_SAVEAS_COMB, fEncodeType<0? 0: fEncodeType);  // 设置默认的编码格式
	if (IDCANCEL == cfd.DoModal())
	{
		return ;
	}
	// GetWindowText(m_fname);
	cfd.GetSelectedControlItem(IDC_SAVEAS_COMB, (DWORD&)fEncodeType);
	m_fname = cfd.GetPathName();
	SetWindowText(m_fname);
	saveAs();

	loadFile(m_fname);
	
}


void CMainDlg::OnNewf()
{
	// TODO: 在此添加命令处理程序代码
	SetWindowText(_T("*新建.txt"));
	SetDlgItemText(IDC_MEDIT, _T(""));  // 新建清空
	fEncodeType = -1;   // 设置为未知编码
	

}


void CMainDlg::OnSearch()
{
	// TODO: 在此添加命令处理程序代码

	if (IsWindow(m_pFRDlg->GetSafeHwnd()))
		delete m_pFRDlg;
	m_pFRDlg = new CFindReplaceDialog;
	CString str = GetSelText();
	m_pFRDlg->Create(TRUE,str);
	m_pFRDlg->ShowWindow(SW_NORMAL);

	//delete findDlg;
	//auto pEdit = (CEdit*)GetDlgItem(IDC_MEDIT);
	//CString s;
	//s.Find(_T("dsd"));
	//pEdit->GetSel();
	//pEdit->SetSel(0,0+6);  // 选中

}


void CMainDlg::OnPrintp()
{
	// TODO: 在此添加命令处理程序代码
	/* 
		dwFlags(DWORD) : 一组标志位，用于控制对话框的外观和行为。这些标志位可以通过按位或运算组合起来。例如：
		PD_RETURNDC: 选择设备上下文而不是打印机驱动程序名称。
		PD_RETURNIC : 返回设备描述表句柄而不是打印机驱动程序名称。
		PD_USEDEVMODECOPIESANDCOLLATE : 使用 devmode 结构中的 dmCopies 和 dmCollate 字段。
		PD_ALLPAGES : 用户可以选择打印所有页面。
		PD_SELECTION : 用户可以选择打印选定范围。
		PD_PAGENUMS : 用户可以选择打印特定页码。
		PD_PRINTTOFILE : 允许将文档发送到文件而不是打印机。
		PD_COLLATE : 允许用户指定打印副本是否应该被整理。
	*/
	
	PRINTDLG pd;
	memset(&pd, 0, sizeof(pd));   // PRINTDLG 结构体初始化
	pd.lStructSize = sizeof(pd);
	pd.Flags = PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIES | PD_PRINTTOFILE; // 设置对话框模式
	pd.hDevMode = NULL;
	pd.hDevNames = NULL;    // 没有这句话会点击打印时会提示内存不足
	pd.hDC = NULL;
	CPrintDialog m_pd(FALSE);
	m_pd.m_pd = pd;
	// CPrintDialog m_pd(FALSE, PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIES | PD_PRINTTOFILE);
	INT_PTR ret= m_pd.DoModal();

	if (ret == IDOK)
	{
		// 用户点击了确定按钮
		// 获取打印机设备上下文
		HDC hdc = m_pd.m_pd.hDC;
		// 这里可以进行打印操作

		// 释放打印机设备上下文
		::ReleaseDC(NULL ,hdc);
		AfxMessageBox(L"ooooooo");
	}
	else if (ret == IDCANCEL)
	{
		// 用户点击了取消按钮
		AfxMessageBox(L"ccccccc");
	}

}


void CMainDlg::OnXdate()
{
	CTime time = CTime::GetCurrentTime();  // 全局的GetCurrentTime需要进行时间转换，CTime::GetCurrentTime内部进行了处理
	CString str;
	str.Format(_T("%02d:%02d %02d/%02d/%02d"), time.GetHour(),
		time.GetMinute(), time.GetYear(), time.GetMonth(), time.GetDay());
	auto pedit = (CEdit*)GetDlgItem(IDC_MEDIT);
	pedit->ReplaceSel(str);
}


void CMainDlg::OnSearchOnNet()
{
	CString str;
	str = GetSelText();
	str = _T("https://www.baidu.com/s?wd=") + str;   // https://www.baidu.com/s?wd=
	ShellExecute(NULL, _T("open"), str, NULL, NULL, SW_SHOW);
}


void CMainDlg::OnSelectall()
{
	// TODO: 在此添加命令处理程序代码
	auto pedit = (CEdit*)GetDlgItem(IDC_MEDIT);
	// int nEnd;
	CString str; 
	pedit->GetWindowText(str);
	pedit->SetSel(0,str.GetLength());   // 设置选择  全选
}


void CMainDlg::OnXautoline()
{
	// TODO: 在此添加命令处理程序代码
	// auto p = GetDlgItem(ID_XAUTOLINE);
	//auto pedit = (CEdit*)GetDlgItem(IDC_MEDIT);
	//CString str;
	//CRect rect;
	//pedit->GetWindowText(str);
	//pedit->GetWindowRect(rect);
	//ScreenToClient(rect);
	//DWORD oldstyle = pedit->GetStyle();
	//DWORD oldstyleEx = pedit->GetExStyle();
	//pedit->DestroyWindow();
	//pedit->CreateEx(oldstyleEx,_T("EDIT"), str, oldstyle | ES_AUTOHSCROLL, rect, this, IDC_MEDIT);
	//// pedit->ModifyStyle(ES_AUTOHSCROLL, ES_AUTOVSCROLL);
	//// pedit->SetWindowText(str);
	//RedrawWindow();
}


void CMainDlg::OnXfont()
{

	CFontDialog fdlg(&m_lf);
	if (IDCANCEL == fdlg.DoModal())
	{
		return;
	}
	
	auto pedit = (CEdit*)GetDlgItem(IDC_MEDIT);
	CFont newft;
	LOGFONT lf;
	fdlg.GetCurrentFont(&lf);
	// memcpy(&m_lf,&lf,sizeof(m_lf));
	m_lf = lf;
	newft.DeleteObject();  
	newft.CreateFontIndirect(&m_lf);
	pedit->SetFont(&newft);
}


void CMainDlg::OnReplace()
{
	// TODO: 在此添加命令处理程序代码
	if (IsWindow(m_pFRDlg->GetSafeHwnd()))
		delete m_pFRDlg;
	m_pFRDlg = new CFindReplaceDialog;
	CString str = GetSelText();
	m_pFRDlg->Create(false,NULL);
	m_pFRDlg->ShowWindow(SW_NORMAL);
}


HBRUSH CMainDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_MEDIT:
	{
		pDC->SetTextColor(m_TxtColor);
		pDC->SetBkColor(m_BkColor);
	}
		return m_EditBrush;
	case IDD_MAINDLG:
		return m_EditBrush;
	}
	
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CMainDlg::OnBkcolor()
{
	// TODO: 在此添加命令处理程序代码
	CColorDialog BkColorDlg(0,0,this);

	if (IDCANCEL == BkColorDlg.DoModal())
	{
		return;
	}
	COLORREF color = BkColorDlg.GetColor();
	if (m_BkColor != color)
	{
		m_BkColor = color;
		m_EditBrush.DeleteObject();
		m_EditBrush.CreateSolidBrush(m_BkColor);	
	}


}


void CMainDlg::OnFontcolor()
{
	// TODO: 在此添加命令处理程序代码
	CColorDialog TxtColorDlg;
	if (IDCANCEL == TxtColorDlg.DoModal())
	{
		return;
	}
	m_TxtColor = TxtColorDlg.GetColor();
}
