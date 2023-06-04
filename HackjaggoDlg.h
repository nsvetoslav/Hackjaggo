#pragma once

#include <vector>
#include <map>

class CHackjaggoDlg : public CDialogEx
{
	struct list_control_item {
		CString strPID;
		CString strName;

		list_control_item(const CString _strPID, const CString _strName) {
			strPID = _strPID;
			strName = _strName;
		}
	};

	enum ProcessesListControlColumns : int
	{
		ProcessesListControlColumnPID,
		ProcessesListControlColumnName,
	};

	enum LoadedModulesListControlColumns : int {
		LoadedModulesListControlColumnName,
		LoadedModulesListControlColumnFullPath
	};

	enum ModuleFunctionsListControlColumns : int {
		ModuleFunctionsListControlColumnName,
	};

	DECLARE_MESSAGE_MAP();

public:
	CHackjaggoDlg(CWnd* pParent = nullptr);	
	virtual ~CHackjaggoDlg() = default;

	// dialog resource
	enum { IDD = IDD_HACKJAGGO_DIALOG };

protected:
	// handles the ddx controls
	void DoDataExchange(CDataExchange* pDX) override;	
	
	// runs on dialog initialization
	BOOL OnInitDialog() override;

private:
	void InitializeProcessesListControl();

	void InitializeLoadedModulesListControls();

	void InitializeModulesFunctionsListControl();

	void ListExportedFunctions(const TCHAR* modulePath);

	void SetListControlItems();

	void FilterProcesses(const CString& filterText);

	void EnumerateExportedFunctions(HMODULE hModule);

private:
	// handles on-paint message
	afx_msg void OnPaint();

	// handes on-drag dialog
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	afx_msg void OnContexMenuLoadedModules();

	afx_msg void OnContextMenuModuleFunctions();

	afx_msg void OnTextChanged();
	
private:
	// icon
	HICON m_hIcon;

	// the processes list control
	CListCtrl m_oProcessesListControl;

	// processes list control context menu
	CMenu m_oProcessesListControlContextMenu;

	// loaded modules list control context menu
	CMenu m_oLoadedModulesListControlContextMenu;

	// filter control
	CEdit m_oTextFilter;

	// loaded modules
	CListCtrl m_oLoadedModulesListControl;

	// module functions
	CListCtrl m_oModuleFunctionsListControl;

	// exe name
	CStatic m_oExeName;

	// module name 
	CStatic m_oModuleName;

	std::vector<list_control_item> m_oProccessesListControlPidToNameVector;
};
