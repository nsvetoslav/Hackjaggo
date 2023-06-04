#include "pch.h"
#include "framework.h"
#include "Hackjaggo.h"
#include "HackjaggoDlg.h"
#include "afxdialogex.h"
#include <windows.h>
#include <psapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MENU_SHOW_LOADED_MODULES 111111
#define MENU_SHOW_MODULE_FUNCTIONS 111112

CHackjaggoDlg::CHackjaggoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HACKJAGGO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHackjaggoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_PROCESSES, m_oProcessesListControl);	
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_oTextFilter);	
	DDX_Control(pDX, IDC_LIST_MODULES, m_oLoadedModulesListControl);
	DDX_Control(pDX, IDC_LIST_MODULE_FUNCTIONS, m_oModuleFunctionsListControl);

	DDX_Control(pDX, IDC_EXE_NAME, m_oExeName);
	DDX_Control(pDX, IDC_MODULE_NAME, m_oModuleName);

	
}

BEGIN_MESSAGE_MAP(CHackjaggoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(MENU_SHOW_LOADED_MODULES, OnContexMenuLoadedModules)
	ON_COMMAND(MENU_SHOW_MODULE_FUNCTIONS, OnContextMenuModuleFunctions)	
	ON_EN_CHANGE(IDC_EDIT_SEARCH, OnTextChanged)
END_MESSAGE_MAP()

BOOL CHackjaggoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		

	InitializeProcessesListControl();

	InitializeLoadedModulesListControls();

	InitializeModulesFunctionsListControl();

	m_oProcessesListControlContextMenu.CreatePopupMenu();
	m_oProcessesListControlContextMenu.AppendMenu(MF_STRING, MENU_SHOW_LOADED_MODULES, _T("List loaded Modules"));

	m_oLoadedModulesListControlContextMenu.CreatePopupMenu();
	m_oLoadedModulesListControlContextMenu.AppendMenu(MF_STRING, MENU_SHOW_MODULE_FUNCTIONS, _T("List exported funcitons"));
	

	FilterProcesses(_T(""));

	return TRUE;  
}

void CHackjaggoDlg::InitializeProcessesListControl()
{
	// Set the extended style to display full row selection and gridlines
	DWORD dwStyle = m_oProcessesListControl.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	m_oProcessesListControl.SetExtendedStyle(dwStyle);

	m_oProcessesListControl.InsertColumn(ProcessesListControlColumnPID, _T("PID"), LVCFMT_LEFT, 50);
	m_oProcessesListControl.InsertColumn(ProcessesListControlColumnName, _T("Name"), LVCFMT_LEFT, 500);

	SetListControlItems();
}

void CHackjaggoDlg::InitializeLoadedModulesListControls()
{
	// Set the extended style to display full row selection and gridlines
	DWORD dwStyle = m_oProcessesListControl.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	m_oLoadedModulesListControl.SetExtendedStyle(dwStyle);

	m_oLoadedModulesListControl.InsertColumn(LoadedModulesListControlColumnName, _T("Name"), LVCFMT_LEFT, 350);
	m_oLoadedModulesListControl.InsertColumn(LoadedModulesListControlColumnFullPath, _T("Path"), LVCFMT_LEFT, 350);	
}

void CHackjaggoDlg::InitializeModulesFunctionsListControl()
{
	// Set the extended style to display full row selection and gridlines
	DWORD dwStyle = m_oProcessesListControl.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	m_oModuleFunctionsListControl.SetExtendedStyle(dwStyle);

	m_oModuleFunctionsListControl.InsertColumn(ModuleFunctionsListControlColumnName, _T("Name"), LVCFMT_LEFT, 350);
}

void CHackjaggoDlg::EnumerateExportedFunctions(HMODULE hModule)
{
	m_oModuleFunctionsListControl.DeleteAllItems();
	// Get the base address of the module
	DWORD_PTR moduleBase = (DWORD_PTR)hModule;

	// Enumerate exported functions
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)moduleBase;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(moduleBase + pDosHeader->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)(moduleBase + pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	DWORD* pFunctions = (DWORD*)(moduleBase + pExportDir->AddressOfFunctions);
	DWORD* pNames = (DWORD*)(moduleBase + pExportDir->AddressOfNames);
	WORD* pOrdinals = (WORD*)(moduleBase + pExportDir->AddressOfNameOrdinals);

	for (DWORD i = 0; i < pExportDir->NumberOfNames; ++i)
	{
		DWORD functionRVA = pFunctions[pOrdinals[i]];
		DWORD functionNameRVA = pNames[i];
		const char* functionName = (const char*)(moduleBase + functionNameRVA);

		m_oModuleFunctionsListControl.InsertItem(0, CString(functionName));
	}
}


void CHackjaggoDlg::ListExportedFunctions(const TCHAR* modulePath)
{
	HMODULE hModule = LoadLibrary(modulePath);
	if (hModule != NULL)
	{
		// Enumerate exported functions
		EnumerateExportedFunctions(hModule);

		// Free the module handle
		FreeLibrary(hModule);
	}
	else
	{
		// Failed to load the module
		DWORD error = GetLastError();
		// Handle the error or display an appropriate message
	}
}

void CHackjaggoDlg::SetListControlItems()
{
	m_oProcessesListControl.DeleteAllItems();

	const DWORD bufferSize = 32767;
	DWORD processIds[bufferSize], bytesReturned;

	// Enumerate all processes
	if (EnumProcesses(processIds, sizeof(processIds), &bytesReturned))
	{
		// Calculate the number of processes
		DWORD processCount = bytesReturned / sizeof(DWORD);

		for (DWORD i = 0; i < processCount; ++i)
		{
			HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);

			if (processHandle != nullptr)
			{
				WCHAR processName[MAX_PATH];
				if (GetModuleBaseName(processHandle, nullptr, processName, sizeof(processName) / sizeof(WCHAR)) > 0)
				{
 					CString strPID;
					strPID.Format(_T("%d"), GetProcessId(processHandle));
					
					const int itemIndex = m_oProcessesListControl.InsertItem(i, strPID);			
					m_oProcessesListControl.SetItemText(itemIndex, ProcessesListControlColumnName, processName);					
					
					// Add the PID and process name to the vector
					m_oProccessesListControlPidToNameVector.push_back(list_control_item(strPID, processName));
				}

				CloseHandle(processHandle);
			}
		}
	}

}

void CHackjaggoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
	
		CRect rect;
		GetClientRect(&rect);
		
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CHackjaggoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CHackjaggoDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd == &m_oProcessesListControl)
	{
		// Display the context menu at the specified position
		m_oProcessesListControlContextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
	}
	else if (pWnd == &m_oLoadedModulesListControl) 
	{
		// Display the context menu at the specified position
		m_oLoadedModulesListControlContextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
	}
	else if (pWnd == &m_oModuleFunctionsListControl)
	{
	}
	else
	{
		CDialogEx::OnContextMenu(pWnd, point);
	}
}

void CHackjaggoDlg::OnContexMenuLoadedModules()
{
	m_oModuleFunctionsListControl.DeleteAllItems();
	m_oLoadedModulesListControl.DeleteAllItems();
	m_oExeName.SetWindowTextW(_T(""));
	m_oModuleName.SetWindowTextW(_T(""));

	CString strModules;
	
	int selectedIndex = m_oProcessesListControl.GetNextItem(-1, LVNI_SELECTED);
	if (selectedIndex == -1) {
		MessageBox(_T("Invalid process selected."));
		return;
	}

	CString processName = _T("Process: ");
	processName.Append(m_oProcessesListControl.GetItemText(selectedIndex, ProcessesListControlColumnName));	
	m_oExeName.SetWindowTextW(processName);
	
	const CString processPID = m_oProcessesListControl.GetItemText(selectedIndex, ProcessesListControlColumnPID);
	
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, _ttoi(processPID));

	if (processHandle != nullptr)
	{
		HMODULE moduleHandles[1024];
		DWORD bytesNeeded;

		if (EnumProcessModules(processHandle, moduleHandles, sizeof(moduleHandles), &bytesNeeded))
		{
			int moduleCount = bytesNeeded / sizeof(HMODULE);

			TCHAR moduleName[MAX_PATH];
			for (int i = 0; i < moduleCount; ++i)
			{
				long lItemIndex = 0;

				ZeroMemory(moduleName, sizeof(moduleName));
				if (GetModuleBaseName(processHandle, moduleHandles[i], moduleName, sizeof(moduleName) / sizeof(TCHAR)))
				{
					lItemIndex = m_oLoadedModulesListControl.InsertItem(0, moduleName);
				}

				TCHAR modulePath[MAX_PATH];
				if (GetModuleFileNameEx(processHandle, moduleHandles[i], modulePath, sizeof(modulePath) / sizeof(TCHAR)) > 0)
				{
					m_oLoadedModulesListControl.SetItemText(lItemIndex, LoadedModulesListControlColumnFullPath, modulePath);
				}
			}
		}
		else
		{
			MessageBox(_T("Failed to enumerate modules."));
		}

		CloseHandle(processHandle);
	}
	else
	{
		MessageBox(_T("Failed to open process."));
	}
}

void CHackjaggoDlg::OnContextMenuModuleFunctions()
{
	int selectedIndex = m_oLoadedModulesListControl.GetNextItem(-1, LVNI_SELECTED);
	if (selectedIndex == -1) {
		MessageBox(_T("Invalid process selected."));
		return;
	}

	const CString strModuleName = m_oLoadedModulesListControl.GetItemText(selectedIndex, LoadedModulesListControlColumnName);
	const CString strFullPath = m_oLoadedModulesListControl.GetItemText(selectedIndex, LoadedModulesListControlColumnFullPath);

	m_oModuleName.SetWindowTextW(_T("Module: ") + strModuleName);
	ListExportedFunctions(strFullPath);
}

void CHackjaggoDlg::OnTextChanged()
{
	// Get the entered text from the edit control
	CString strFilter;
	m_oTextFilter.GetWindowText(strFilter);

	// Filter the list control based on the entered text
	FilterProcesses(strFilter);
}

void CHackjaggoDlg::FilterProcesses(const CString& filterText)
{
	// Clear the list control
	m_oProcessesListControl.DeleteAllItems();

	// Iterate through the processes and filter based on the entered text
	const int processCount = m_oProccessesListControlPidToNameVector.size();

	long lItemIndex = 0;

	CString filterTextLower = filterText;
	filterTextLower.MakeLower();

	for (auto& item : m_oProccessesListControlPidToNameVector) {

		CString itemLowered = item.strName;
		itemLowered.MakeLower();

		// Check if the process name contains the filter text
		if (itemLowered.Find(filterTextLower) != -1)
		{
			// Insert the matching process into the list control
			int index = m_oProcessesListControl.InsertItem(++lItemIndex, item.strPID);
			m_oProcessesListControl.SetItemText(index, ProcessesListControlColumnName, item.strName);
		}
	}
}