
// ResourceAgent.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CResourceAgentApp:
// �� Ŭ������ ������ ���ؼ��� ResourceAgent.cpp�� �����Ͻʽÿ�.
//

class CResourceAgentApp : public CWinApp
{
public:
	CResourceAgentApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CResourceAgentApp theApp;