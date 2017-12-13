
// ResourceAgentDlg.cpp : ���� ����
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


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CResourceAgentDlg ��ȭ ����



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


// CResourceAgentDlg �޽��� ó����

BOOL CResourceAgentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
    SetDlgItemText(IDC_EDIT_URL, L"http://localhost:8088/is");


    m_Local_IP_List.InsertColumn(0, L"IpList", LVCFMT_LEFT, 100);

    SysInfo sysInfo = SysInfo::GetSystemInfo();

    //AfxMessageBox(sysInfo.work_group.c_str());
    SetDlgItemText(IDC_EDIT_WORKGROUP, _bstr_t(sysInfo.work_group.c_str()));

    for (auto &ip : sysInfo.local_ip_list)
        m_Local_IP_List.InsertItem(0, _bstr_t(ip.c_str()));
    //std::for_each(sysInfo.local_ip_list.begin(), sysInfo.local_ip_list.end(),
    //    [](const std::string &ip) { AfxMessageBox(to_wstring(ip).c_str()); });



	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CResourceAgentDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CResourceAgentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CResourceAgentDlg::OnBnClickedPostResource()
{
    // TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
