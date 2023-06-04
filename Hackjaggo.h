#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		


class CHackjaggoApp : public CWinApp
{
public:
	CHackjaggoApp();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP();
};

extern CHackjaggoApp theApp;
