
// ResourceAgentDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ResourceAgent.h"
#include "ResourceAgentDlg.h"
#include "afxdialogex.h"

#include "sysinfo.h"
#include "regenum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CResourceAgentDlg 대화 상자



CResourceAgentDlg::CResourceAgentDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CResourceAgentDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CResourceAgentDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_LOCAL_IPADDR, m_Local_IP_List);
}

BEGIN_MESSAGE_MAP(CResourceAgentDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_POST_RESOURCE, &CResourceAgentDlg::OnBnClickedPostResource)
END_MESSAGE_MAP()


// CResourceAgentDlg 메시지 처리기

BOOL CResourceAgentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
    SetDlgItemText(IDC_EDIT_URL, L"http://localhost:8088/is");


    m_Local_IP_List.InsertColumn(0, L"IpList", LVCFMT_LEFT, 100);

    SysInfo sysInfo = SysInfo::GetSystemInfo();

    //AfxMessageBox(sysInfo.work_group.c_str());
    SetDlgItemText(IDC_EDIT_WORKGROUP, _bstr_t(sysInfo.work_group.c_str()));

    for (auto &ip : sysInfo.local_ip_list)
        m_Local_IP_List.InsertItem(0, _bstr_t(ip.c_str()));
    //std::for_each(sysInfo.local_ip_list.begin(), sysInfo.local_ip_list.end(),
    //    [](const std::string &ip) { AfxMessageBox(to_wstring(ip).c_str()); });



	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CResourceAgentDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CResourceAgentDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CResourceAgentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CResourceAgentDlg::OnBnClickedPostResource()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    PostInstalledSoftwareList(L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest1", L"Computer1", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest1", L"Computer2", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest1", L"Computer3", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest1", L"Computer4", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest1", L"Computer5", L"localhost:8088");

    PostInstalledSoftwareList(L"WorkGroupTest2", L"Computer6", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest2", L"Computer7", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest2", L"Computer8", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest2", L"Computer9", L"localhost:8088");
    PostInstalledSoftwareList(L"WorkGroupTest2", L"Computer10", L"localhost:8088");

}
