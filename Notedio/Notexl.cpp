
// Notexl.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "Notexl.h"
#include "CMainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNotexlApp

BEGIN_MESSAGE_MAP(CNotexlApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CNotexlApp 构造

CNotexlApp::CNotexlApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	//wchar_t* spath = new wchar_t[16];    
	//StrCpyW(spath, _T(".\\ntxl.INI"));   // 同_tcsdup(_T("...."));
	this->m_pszProfileName = _tcsdup(_T(".\\ntxl.INI"));   // 在堆内申请空间，防止静态成员最后释放，导致野指针释放问题。
										// 默认初始化m_pszProfileName = _tcsdup(str); 也是堆内申请空间的

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CNotexlApp 对象

CNotexlApp theApp;


// CNotexlApp 初始化

BOOL CNotexlApp::InitInstance()
{
	//  SetRegistryKey(_T("App_name"));   // writeprofileint等方法的写入位置设置为 注册表 内
	// 取消注释则是保存为ini文件，或者自己写文件加载文件

	CMainDlg dlg;
	dlg.DoModal();
	
	return FALSE;
}

