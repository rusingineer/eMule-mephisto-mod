//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "SharedFilesCtrl.h"
#include "OtherFunctions.h"
#include "FileInfoDialog.h"
#include "MetaDataDlg.h"
#include "ED2kLinkDlg.h"
#include "ArchivePreviewDlg.h"
#include "CommentDialog.h"
#include "HighColorTab.hpp"
#include "ListViewWalkerPropertySheet.h"
#include "UserMsgs.h"
#include "ResizableLib/ResizableSheet.h"
#include "KnownFile.h"
#include "MapKey.h"
#include "SharedFileList.h"
#include "MemDC.h"
#include "PartFile.h"
#include "MenuCmds.h"
#include "IrcWnd.h"
#include "SharedFilesWnd.h"
#include "Opcodes.h"
#include "InputBox.h"
#include "WebServices.h"
#include "TransferWnd.h"
#include "ClientList.h"
#include "UpDownClient.h"
#include "Collection.h"
#include "CollectionCreateDialog.h"
#include "CollectionViewDialog.h"
#include "SharedDirsTreeCtrl.h"
#include "SearchParams.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "ToolTipCtrlX.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "MassRename.h" //Xman Mass Rename (Morph)
#include "Log.h" //Xman Mass Rename (Morph)
#include "UploadQueue.h" // Mephisto Upload - Mephisto

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool NeedArchiveInfoPage(const CSimpleArray<CObject*>* paItems);
void UpdateFileDetailsPages(CListViewWalkerPropertySheet *pSheet,
							CResizablePage *pArchiveInfo, CResizablePage *pMediaInfo);


//////////////////////////////////////////////////////////////////////////////
// CSharedFileDetailsSheet

class CSharedFileDetailsSheet : public CListViewWalkerPropertySheet
{
	DECLARE_DYNAMIC(CSharedFileDetailsSheet)

public:
	CSharedFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage = 0, CListCtrlItemWalk* pListCtrl = NULL);
	virtual ~CSharedFileDetailsSheet();

protected:
	CFileInfoDialog		m_wndMediaInfo;
	CMetaDataDlg		m_wndMetaData;
	CED2kLinkDlg		m_wndFileLink;
	CCommentDialog		m_wndFileComments;
	CArchivePreviewDlg	m_wndArchiveInfo;

	UINT m_uPshInvokePage;
	static LPCTSTR m_pPshStartPage;

	void UpdateTitle();

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDataChanged(WPARAM, LPARAM);
};

LPCTSTR CSharedFileDetailsSheet::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CSharedFileDetailsSheet, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CSharedFileDetailsSheet, CListViewWalkerPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
END_MESSAGE_MAP()

CSharedFileDetailsSheet::CSharedFileDetailsSheet(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage, CListCtrlItemWalk* pListCtrl)
: CListViewWalkerPropertySheet(pListCtrl)
{
	m_uPshInvokePage = uPshInvokePage;
	POSITION pos = aFiles.GetHeadPosition();
	while (pos)
		m_aItems.Add(aFiles.GetNext(pos));
	m_psh.dwFlags &= ~PSH_HASHELP;

	m_wndFileComments.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileComments.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileComments.m_psp.pszIcon = _T("FileComments");
	m_wndFileComments.SetFiles(&m_aItems);
	AddPage(&m_wndFileComments);

	m_wndArchiveInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndArchiveInfo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndArchiveInfo.m_psp.pszIcon = _T("ARCHIVE_PREVIEW");
	m_wndArchiveInfo.SetFiles(&m_aItems);
	m_wndMediaInfo.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMediaInfo.m_psp.dwFlags |= PSP_USEICONID;
	m_wndMediaInfo.m_psp.pszIcon = _T("MEDIAINFO");
	m_wndMediaInfo.SetFiles(&m_aItems);
	if (NeedArchiveInfoPage(&m_aItems))
		AddPage(&m_wndArchiveInfo);
	else
		AddPage(&m_wndMediaInfo);

	m_wndMetaData.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndMetaData.m_psp.dwFlags |= PSP_USEICONID;
	m_wndMetaData.m_psp.pszIcon = _T("METADATA");
	if (m_aItems.GetSize() == 1 && thePrefs.IsExtControlsEnabled()) {
		m_wndMetaData.SetFiles(&m_aItems);
		AddPage(&m_wndMetaData);
	}

	m_wndFileLink.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndFileLink.m_psp.dwFlags |= PSP_USEICONID;
	m_wndFileLink.m_psp.pszIcon = _T("ED2KLINK");
	m_wndFileLink.SetFiles(&m_aItems);
	AddPage(&m_wndFileLink);

	LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}
}

CSharedFileDetailsSheet::~CSharedFileDetailsSheet()
{
}

void CSharedFileDetailsSheet::OnDestroy()
{
	if (m_uPshInvokePage == 0)
		m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPropertySheet::OnDestroy();
}

BOOL CSharedFileDetailsSheet::OnInitDialog()
{
	EnableStackedTabs(FALSE);
	BOOL bResult = CListViewWalkerPropertySheet::OnInitDialog();
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("SharedFileDetailsSheet")); // call this after(!) OnInitDialog
	UpdateTitle();
	return bResult;
}

LRESULT CSharedFileDetailsSheet::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	UpdateFileDetailsPages(this, &m_wndArchiveInfo, &m_wndMediaInfo);
	return 1;
}

void CSharedFileDetailsSheet::UpdateTitle()
{
	if (m_aItems.GetSize() == 1)
		SetWindowText(GetResString(IDS_DETAILS) + _T(": ") + STATIC_DOWNCAST(CKnownFile, m_aItems[0])->GetFileName());
	else
		SetWindowText(GetResString(IDS_DETAILS));
}

BOOL CSharedFileDetailsSheet::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_APPLY_NOW)
	{
		CSharedFilesCtrl* pSharedFilesCtrl = DYNAMIC_DOWNCAST(CSharedFilesCtrl, m_pListCtrl->GetListCtrl());
		if (pSharedFilesCtrl)
		{
			for (int i = 0; i < m_aItems.GetSize(); i++) {
				// so, and why does this not(!) work while the sheet is open ??
				pSharedFilesCtrl->UpdateFile(DYNAMIC_DOWNCAST(CKnownFile, m_aItems[i]));
			}
		}
	}

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////
// CSharedFilesCtrl

IMPLEMENT_DYNAMIC(CSharedFilesCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CSharedFilesCtrl, CMuleListCtrl)
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(LVN_GETINFOTIP, OnLvnGetInfoTip)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CSharedFilesCtrl::CSharedFilesCtrl()
	: CListCtrlItemWalk(this)
{
	memset(&sortstat, 0, sizeof(sortstat));
	nAICHHashing = 0;
	m_pDirectoryFilter = NULL;
	SetGeneralPurposeFind(true, false);
	m_pToolTip = new CToolTipCtrlX;
}

CSharedFilesCtrl::~CSharedFilesCtrl()
{
	delete m_pToolTip;
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	if (m_PowershareMenu) VERIFY( m_PowershareMenu.DestroyMenu() );
	if (m_PowerShareLimitMenu) VERIFY( m_PowerShareLimitMenu.DestroyMenu() );
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	if (m_PsAmountLimitMenu) VERIFY( m_PsAmountLimitMenu.DestroyMenu() );
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	if (m_HideOSMenu) VERIFY( m_HideOSMenu.DestroyMenu() );
	if (m_SelectiveChunkMenu) VERIFY( m_SelectiveChunkMenu.DestroyMenu() );
	if (m_ShareOnlyTheNeedMenu) VERIFY( m_ShareOnlyTheNeedMenu.DestroyMenu() );
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	// ==> mem leak fix [fafner] - Stulle
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );
	// <== mem leak fix [fafner] - Stulle
}

void CSharedFilesCtrl::Init()
{
	SetName(_T("SharedFilesCtrl"));

	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	ASSERT( (GetStyle() & LVS_SINGLESEL) == 0 );

	InsertColumn(0, GetResString(IDS_DL_FILENAME) ,LVCFMT_LEFT,250,0);
	InsertColumn(1,GetResString(IDS_DL_SIZE),LVCFMT_LEFT,100,1);
	InsertColumn(2,GetResString(IDS_TYPE),LVCFMT_LEFT,50,2);
	InsertColumn(3,GetResString(IDS_PRIORITY),LVCFMT_LEFT,70,3);
	InsertColumn(4,GetResString(IDS_FILEID),LVCFMT_LEFT,220,4);
	InsertColumn(5,GetResString(IDS_SF_REQUESTS),LVCFMT_LEFT,100,5);
	InsertColumn(6,GetResString(IDS_SF_ACCEPTS),LVCFMT_LEFT,100,6);
	InsertColumn(7,GetResString(IDS_SF_TRANSFERRED),LVCFMT_LEFT,120,7);
	InsertColumn(8,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,8);
	InsertColumn(9,GetResString(IDS_FOLDER),LVCFMT_LEFT,200,9);
	InsertColumn(10,GetResString(IDS_COMPLSOURCES),LVCFMT_LEFT,100,10);
	InsertColumn(11,GetResString(IDS_SHAREDTITLE),LVCFMT_LEFT,200,11);
	InsertColumn(12,GetResString(IDS_ONQUEUE),LVCFMT_LEFT,50,12); //Xman see OnUploadqueue
	InsertColumn(13,GetResString(IDS_SHAREFACTOR),LVCFMT_LEFT,100,13); //Xman advanced upload-priority
	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	InsertColumn(14,GetResString(IDS_SF_UPLOADED_PARTS),LVCFMT_LEFT,170,13);
	InsertColumn(15,GetResString(IDS_SF_TURN_PART),LVCFMT_LEFT,100,14);
	InsertColumn(16,GetResString(IDS_SF_TURN_SIMPLE),LVCFMT_LEFT,100,15);
	InsertColumn(17,GetResString(IDS_SF_FULLUPLOAD),LVCFMT_LEFT,100,16);
	// <== Spread bars [Slugfiller/MorphXT] - Stulle
	InsertColumn(18,GetResString(IDS_RARE_RATIO),LVCFMT_LEFT,100,17); // push rare file - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	InsertColumn(19,GetResString(IDS_HIDEOS),LVCFMT_LEFT,100,18);
	InsertColumn(20,GetResString(IDS_SHAREONLYTHENEED),LVCFMT_LEFT,100,19);
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	InsertColumn(21,GetResString(IDS_POWERSHARE_COLUMN_LABEL),LVCFMT_LEFT,70,20); // PowerShare [ZZ/MorphXT] - Stulle

	SetAllIcons();
	CreateMenues();
	LoadSettings();

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	/*
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 20)  + (GetSortSecondValue() ? 100 : 0));
	*/
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0 : 30)  + (GetSortSecondValue() ? 100 : 0));
	// <== PowerShare [ZZ/MorphXT] - Stulle

	CToolTipCtrl* tooltip = GetToolTips();
	if (tooltip){
		m_pToolTip->SetFileIconToolTip(true);
		m_pToolTip->SubclassWindow(*tooltip);
		tooltip->ModifyStyle(0, TTS_NOPREFIX);
		tooltip->SetDelayTime(TTDT_AUTOPOP, 20000);
		tooltip->SetDelayTime(TTDT_INITIAL, thePrefs.GetToolTipDelay()*1000);
	}
}

void CSharedFilesCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
	CreateMenues();
}

void CSharedFilesCtrl::SetAllIcons()
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	m_ImageList.Add(CTempIconLoader(_T("EMPTY")));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedServer"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("FileSharedKad"), 16, 16));
	m_ImageList.Add(CTempIconLoader(_T("Rating_NotRated")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fake")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Poor")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Fair")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Good")));
	m_ImageList.Add(CTempIconLoader(_T("Rating_Excellent")));
	m_ImageList.Add(CTempIconLoader(_T("Collection_Search"))); // rating for comments are searched on kad
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("FileCommentsOvl"))), 1);
}

void CSharedFilesCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_DL_FILENAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_DL_SIZE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_TYPE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_PRIORITY);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);

	strRes = GetResString(IDS_FILEID);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_SF_REQUESTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_SF_ACCEPTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_SF_TRANSFERRED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	strRes = GetResString(IDS_SHARED_STATUS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(8, &hdi);

	strRes = GetResString(IDS_FOLDER);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(9, &hdi);

	strRes = GetResString(IDS_COMPLSOURCES);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(10, &hdi);

	strRes = GetResString(IDS_SHAREDTITLE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(11, &hdi);

	//Xman see OnUploadqueue
	strRes = GetResString(IDS_ONQUEUE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(12, &hdi);
	//Xman end

	//Xman advanced upload-priority
	strRes = GetResString(IDS_SHAREFACTOR);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(13, &hdi);
	//Xman end

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	strRes = GetResString(IDS_SF_UPLOADED_PARTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(14, &hdi);

	strRes = GetResString(IDS_SF_TURN_PART);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(15, &hdi);

	strRes = GetResString(IDS_SF_TURN_SIMPLE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(16, &hdi);

	strRes = GetResString(IDS_SF_FULLUPLOAD);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(17, &hdi);
	// <== Spread bars [Slugfiller/MorphXT] - Stulle

	// ==> push rare file - Stulle
	strRes = GetResString(IDS_RARE_RATIO);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(18, &hdi);
	// <== push rare file - Stulle

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
    strRes = GetResString(IDS_HIDEOS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(19, &hdi);

	strRes = GetResString(IDS_SHAREONLYTHENEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(20, &hdi);
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	strRes = GetResString(IDS_POWERSHARE_COLUMN_LABEL);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(21, &hdi);
	// <== PowerShare [ZZ/MorphXT] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	COLORREF crTempColor = thePrefs.GetStyleBackColor(background_styles, style_b_sharedlist);

	if(crTempColor == CLR_DEFAULT)
		crTempColor = thePrefs.GetStyleBackColor(background_styles, style_b_default);

	if(crTempColor != CLR_DEFAULT)
		SetBkColor(crTempColor);
	else
		SetBkColor(COLORREF(RGB(255,255,255)));
	// <== Design Settings [eWombat/Stulle] - Stulle

	CreateMenues();

	int iItems = GetItemCount();
	for (int i = 0; i < iItems; i++)
		Update(i);
}

void CSharedFilesCtrl::AddFile(const CKnownFile* file)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	// check filter conditions if we should show this file right now
	if (m_pDirectoryFilter != NULL){
		CString strFilePath = file->GetPath();
		if (strFilePath.Right(1) == _T("\\")){
			strFilePath = strFilePath.Left(strFilePath.GetLength()-1);
		}
		switch(m_pDirectoryFilter->m_eItemType){
			case SDI_ALL:
				// No filter
				break;
			case SDI_FILESYSTEMPARENT:
				return; // no files
				break;
			case SDI_NO:
				// some shared directory
			case SDI_CATINCOMING:
				// Categories with special incoming dirs
			case SDI_UNSHAREDDIRECTORY:
				// Items from the whole filesystem tree
				if (strFilePath.CompareNoCase(m_pDirectoryFilter->m_strFullPath) != 0)
					return;
				break;
			case SDI_TEMP:
				// only tempfiles
				if (!file->IsPartFile())
					return;
				else if (m_pDirectoryFilter->m_nCatFilter != -1 && (UINT)m_pDirectoryFilter->m_nCatFilter != ((CPartFile*)file)->GetCategory())
					return;
				break;
			case SDI_DIRECTORY:
				// any userselected shared dir but not incoming or temp
				if (file->IsPartFile())
					return;
				if (strFilePath.CompareNoCase(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) == 0)
					return;
				break;
			case SDI_INCOMING:
				// Main incoming directory
				if (strFilePath.CompareNoCase(thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR)) != 0)
					return;
				// Hmm should we show all incoming files dirs or only those from the main incoming dir here?
				// hard choice, will only show the main for now
				break;

			// Avi3k: SharedView Ed2kType
			case SDI_ED2KFILETYPE:
				{
					if (m_pDirectoryFilter->m_nCatFilter == -1 || m_pDirectoryFilter->m_nCatFilter != GetED2KFileTypeID(file->GetFileName()))
						return;
					break;
				}
			// end Avi3k: SharedView Ed2kType
		}
	}
	if (IsFilteredItem(file))
		return;
	if (FindFile(file) != -1)
		return;
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), LPSTR_TEXTCALLBACK, 0, 0, 0, (LPARAM)file);
	if (iItem >= 0)
		Update(iItem);
	ShowFilesCount(); //Xman Code Improvement for ShowFilesCount
}

void CSharedFilesCtrl::RemoveFile(const CKnownFile* file)
{
	int iItem = FindFile(file);
	if (iItem != -1)
	{
		DeleteItem(iItem);
		ShowFilesCount();
	}
}

//Xman advanced upload-priority
/*
void CSharedFilesCtrl::UpdateFile(const CKnownFile* file)
*/
void CSharedFilesCtrl::UpdateFile(const CKnownFile* file, bool force)
//Xman end
{
	if (!file || !theApp.emuledlg->IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->sharedfileswnd && !force) //Xman advanced upload-priority 
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	int iItem = FindFile(file);
	if (iItem != -1)
	{
		Update(iItem);
		//Xman [MoNKi: -Downloaded History-]
		/*
		if (GetItemState(iItem, LVIS_SELECTED))
		*/
		if (GetItemState(iItem, LVIS_SELECTED) && IsWindowVisible())
		//Xman end
			theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	}
}

int CSharedFilesCtrl::FindFile(const CKnownFile* pFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)pFile;
	return FindItem(&find);
}

void CSharedFilesCtrl::ReloadFileList()
{
	DeleteAllItems();
	theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
	
	CCKey bufKey;
	CKnownFile* cur_file;
	for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition(); pos != 0; ){
		theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
		AddFile(cur_file);
	}
	ShowFilesCount();
}

void CSharedFilesCtrl::ShowFilesCount()
{
	//Xman [MoNKi: -Downloaded History-]
	/*
	CString str;
	if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
	str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
	else
	str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
	theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
	*/
	//if(theApp.emuledlg->sharedfileswnd->sharedfilesctrl.IsWindowVisible())
	if(!theApp.emuledlg->sharedfileswnd->historylistctrl.IsWindowVisible()) //Xman Code Improvement for ShowFilesCount
	{	
		CString str;
		if (theApp.sharedfiles->GetHashingCount() + nAICHHashing)
			str.Format(_T(" (%i, %s %i)"), theApp.sharedfiles->GetCount(), GetResString(IDS_HASHING), theApp.sharedfiles->GetHashingCount() + nAICHHashing);
		else
			str.Format(_T(" (%i)"), theApp.sharedfiles->GetCount());
		theApp.emuledlg->sharedfileswnd->GetDlgItem(IDC_TRAFFIC_TEXT)->SetWindowText(GetResString(IDS_SF_FILES) + str);
	}
	//Xman end
}

void CSharedFilesCtrl::GetItemDisplayText(const CKnownFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const
{
	if (pszText == NULL || cchTextMax <= 0) {
		ASSERT(0);
		return;
	}
	pszText[0] = _T('\0');
	CString buffer;
	switch(iSubItem){
		case 0:
			_tcsncpy(pszText, file->GetFileName(), cchTextMax);
			break;
		case 1:
			_tcsncpy(pszText, CastItoXBytes(file->GetFileSize(), false, false), cchTextMax);
			break;
		case 2:
			_tcsncpy(pszText, file->GetFileTypeDisplayStr(), cchTextMax);
			break;
		case 3:{
			_tcsncpy(pszText, file->GetUpPriorityDisplayString(), cchTextMax);
			break;
			   }
		case 4:
			_tcsncpy(pszText, md4str(file->GetFileHash()), cchTextMax);
			break;
		case 5:
			buffer.Format(_T("%u (%u)"), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests());
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 6:
			buffer.Format(_T("%u (%u)"), file->statistic.GetAccepts(), file->statistic.GetAllTimeAccepts());
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 7:
			buffer.Format(_T("%s (%s)"), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 8:
			break;
		case 9:
			_tcsncpy(pszText, file->GetPath(), cchTextMax);
			PathRemoveBackslash(pszText);
			break;
		case 10:
			if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
				buffer.Format(_T("%u"), file->m_nCompleteSourcesCountLo);
			else if (file->m_nCompleteSourcesCountLo == 0)
				buffer.Format(_T("< %u"), file->m_nCompleteSourcesCountHi);
			else
				buffer.Format(_T("%u - %u"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi);
			_tcsncpy(pszText, buffer, cchTextMax);
			break;
		case 11:
			break;
	}
	pszText[cchTextMax - 1] = _T('\0');
}
#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CSharedFilesCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (!theApp.emuledlg->IsRunning())
		return;
	if (!lpDrawItemStruct->itemData)
		return;

	//MORPH START - Added by SiRoB, Don't draw hidden Rect
	RECT clientRect;
	GetClientRect(&clientRect);
	CRect cur_rec(lpDrawItemStruct->rcItem);
	if (cur_rec.top >= clientRect.bottom || cur_rec.bottom <= clientRect.top)
		return;
	//MORPH END   - Added by SiRoB, Don't draw hidden Rect

	CDC* odc = CDC::FromHandle(lpDrawItemStruct->hDC);
	BOOL bCtrlFocused = ((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS));
	// ==> Design Settings [eWombat/Stulle] - Stulle
	/*
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
	//Xman PowerRelease
	/*
		odc->SetBkColor(GetBkColor());
	*//*
	{
		if(((CKnownFile*)lpDrawItemStruct->itemData)->GetUpPriority()==PR_POWER)
			odc->SetBkColor(RGB(255,210,210));
		else
			odc->SetBkColor(m_crWindow);
	}
	//Xman end

	COLORREF crOldBackColor = odc->GetBkColor(); //Xman Code Improvement: FillSolidRect

	/*const*//* CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(GetFont());
	//MORPH - Moved by SiRoB, Don't draw hidden Rect
	/*
	CRect cur_rec(lpDrawItemStruct->rcItem);
	*//*
	//Xman end
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);
	*/
	CKnownFile* file = (CKnownFile*)lpDrawItemStruct->itemData;
	int iStyle = 0;
	if(file->IsPartFile() && thePrefs.GetStyleOnOff(share_styles, style_s_incomplete)!=0)
		iStyle = style_s_incomplete;
	else 
		iStyle = file->GetKnownStyle();
	StylesStruct style;
	thePrefs.GetStyle(share_styles, iStyle, &style);

	COLORREF crTempColor = GetBkColor();
	if (style.nBackColor != CLR_DEFAULT)
		crTempColor = style.nBackColor;

	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		if (bCtrlFocused)
			odc->SetBkColor(m_crHighlight);
		else
			odc->SetBkColor(m_crNoHighlight);
	}
	else
	{
		odc->SetBkColor(crTempColor);
	}

	COLORREF crOldBackColor = odc->GetBkColor(); //Xman Code Improvement: FillSolidRect
	crTempColor = m_crWindowText;
	if(style.nFontColor != CLR_DEFAULT)
		crTempColor = style.nFontColor;

	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(theApp.GetFontByStyle(style.nFlags,thePrefs.UseNarrowFont()));
	//CRect cur_rec(lpDrawItemStruct->rcItem); //MORPH - Moved by SiRoB, Don't draw hidden Rect
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : crTempColor);
	// <== Design Settings [eWombat/Stulle] - Stulle

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	const int iMarginX = 4;
	cur_rec.right = cur_rec.left - iMarginX*2;
	cur_rec.left += iMarginX;
	CString buffer;
	int iIconDrawWidth = theApp.GetSmallSytemIconSize().cx + 3;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			UINT uDTFlags = DLC_DT_TEXT;
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case 0:{
					int iIconPosY = (cur_rec.Height() > theApp.GetSmallSytemIconSize().cy) ? ((cur_rec.Height() - theApp.GetSmallSytemIconSize().cy) / 2) : 0;
					int iImage = theApp.GetFileTypeSystemImageIdx(file->GetFileName());
					if (theApp.GetSystemImageList() != NULL)
						::ImageList_Draw(theApp.GetSystemImageList(), iImage, dc.GetSafeHdc(), cur_rec.left, cur_rec.top + iIconPosY, ILD_NORMAL|ILD_TRANSPARENT);
					if (!file->GetFileComment().IsEmpty() || file->GetFileRating())
						m_ImageList.Draw(dc, 0, CPoint(cur_rec.left, cur_rec.top + iIconPosY), ILD_NORMAL | ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
					cur_rec.left += (iIconDrawWidth - 3);

					if (thePrefs.ShowRatingIndicator() && (file->HasComment() || file->HasRating() || file->IsKadCommentSearchRunning()))
					{
						m_ImageList.Draw(dc, file->UserRating(true)+3, CPoint(cur_rec.left, cur_rec.top + iIconPosY), ILD_NORMAL);
						cur_rec.left += 16;
						iIconDrawWidth += 16;
					}

					cur_rec.left += 3;

					buffer = file->GetFileName();
					break;
				}
				case 1:
					buffer = CastItoXBytes(file->GetFileSize(), false, false);
					uDTFlags |= DT_RIGHT;
					break;
				case 2:
					buffer = file->GetFileTypeDisplayStr();
					break;
				case 3:{
					buffer = file->GetUpPriorityDisplayString();
					// ==> PowerShare [ZZ/MorphXT] - Stulle
					if(file->GetPowerShared()) {
						CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
						tempString.Append(_T(" "));
						tempString.Append(buffer);
						buffer.Empty();
						buffer = tempString;
					}
					// <== PowerShare [ZZ/MorphXT] - Stulle
					break;
				}
				case 4:
					buffer = md4str(file->GetFileHash());
					break;
				case 5:
                    buffer.Format(_T("%u (%u)"), file->statistic.GetRequests(), file->statistic.GetAllTimeRequests());
					break;
				case 6:
					buffer.Format(_T("%u (%u)"), file->statistic.GetAccepts(), file->statistic.GetAllTimeAccepts());
					break;
				case 7:
					buffer.Format(_T("%s (%s)"), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
					break;
				case 8:
					if (file->GetPartCount()){
						cur_rec.bottom--;
						cur_rec.top++;
						file->DrawShareStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
				case 9:
					buffer = file->GetPath();
					PathRemoveBackslash(buffer.GetBuffer());
					buffer.ReleaseBuffer();
					break;
				case 10:
					//Xman show virtual sources (morph) + virtualUploadsources
					/*
                    if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
						buffer.Format(_T("%u"), file->m_nCompleteSourcesCountLo);
                    else if (file->m_nCompleteSourcesCountLo == 0)
						buffer.Format(_T("< %u"), file->m_nCompleteSourcesCountHi);
					else
						buffer.Format(_T("%u - %u"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi);
					*/
					if(file->IsPartFile()==false || thePrefs.UseAdvancedAutoPtio()==false)
					{
						if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
							buffer.Format(_T("%u (%u)"), file->m_nCompleteSourcesCountLo, file->m_nVirtualCompleteSourcesCount);
						else if (file->m_nCompleteSourcesCountLo == 0)
							buffer.Format(_T("< %u (%u)"), file->m_nCompleteSourcesCountHi, file->m_nVirtualCompleteSourcesCount);
						else
							buffer.Format(_T("%u - %u (%u)"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi, file->m_nVirtualCompleteSourcesCount);
					}
					else
					{
						//Xman advanced upload-priority
						if (file->m_nCompleteSourcesCountLo == file->m_nCompleteSourcesCountHi)
							buffer.Format(_T("%u (%u/%u)"), file->m_nCompleteSourcesCountLo, file->m_nVirtualCompleteSourcesCount, file->m_nVirtualUploadSources);
						else if (file->m_nCompleteSourcesCountLo == 0)
							buffer.Format(_T("< %u (%u/%u)"), file->m_nCompleteSourcesCountHi, file->m_nVirtualCompleteSourcesCount, file->m_nVirtualUploadSources);
						else
							buffer.Format(_T("%u - %u (%u/%u)"), file->m_nCompleteSourcesCountLo, file->m_nCompleteSourcesCountHi, file->m_nVirtualCompleteSourcesCount, file->m_nVirtualUploadSources);
						//Xman end
					}
					//Xman end
					break;
				case 11:{
					CPoint pt(cur_rec.left, cur_rec.top);
					m_ImageList.Draw(dc, file->GetPublishedED2K() ? 1 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
					pt.x += 16;
					bool bSharedInKad;
					if ((uint32)time(NULL) < file->GetLastPublishTimeKadSrc())
					{
						if (theApp.IsFirewalled() && theApp.IsConnected())
						{
							if ((theApp.clientlist->GetBuddy() && (file->GetLastPublishBuddy() == theApp.clientlist->GetBuddy()->GetIP()))
								|| (Kademlia::CKademlia::IsRunning() && !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified()))
							{
								bSharedInKad = true;
							}
							else
								bSharedInKad = false;
						}
						else
							bSharedInKad = true;
					}
					else
						bSharedInKad = false;
					m_ImageList.Draw(dc, bSharedInKad ? 2 : 0, pt, ILD_NORMAL | ILD_TRANSPARENT);
					buffer.Empty();
					break;
				}
				//Xman see OnUploadqueue
				case 12:
					{	
						buffer.Format(_T("%u"),file->GetOnUploadqueue());
						break;
					}
				//Xman end

				//Xman advanced upload-priority
				case 13:
				{
					if(thePrefs.UseAdvancedAutoPtio())
						buffer.Format(_T("%.0f%% / %.0f%%"), file->CalculateUploadPriorityPercent(), file->statistic.GetAllTimeTransferred()/(float)file->GetFileSize()*100);
					else
						buffer.Format(_T("%.0f%%"), file->statistic.GetAllTimeTransferred()/(float)file->GetFileSize()*100);
					break;
				}
				//Xman end

				// ==> Spread bars [Slugfiller/MorphXT] - Stulle
				case 14:
					cur_rec.bottom--;
					cur_rec.top++;
					((CKnownFile*)lpDrawItemStruct->itemData)->statistic.DrawSpreadBar(dc,&cur_rec,thePrefs.UseFlatBar());
					cur_rec.bottom++;
					cur_rec.top--;
					break;
				case 15:
					buffer.Format(_T("%.2f"),((CKnownFile*)lpDrawItemStruct->itemData)->statistic.GetSpreadSortValue());
					break;
				case 16:
					if (file->GetFileSize()>(uint64)0)
						buffer.Format(_T("%.2f"),((double)file->statistic.GetAllTimeTransferred())/((double)file->GetFileSize()));
					else
						buffer.Format(_T("%.2f"),0.0f);
					break;
				case 17:
					buffer.Format(_T("%.2f"),((CKnownFile*)lpDrawItemStruct->itemData)->statistic.GetFullSpreadCount());
					break;
				// <== Spread bars [Slugfiller/MorphXT] - Stulle
				// ==> push rare file - Stulle
				case 18:
					buffer.Format(_T("%.1f"),file->GetFileRatio());
					break;
				// <== push rare file - Stulle
				// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
				case 19:
				{
					UINT hideOSInWork = file->HideOSInWork();
					buffer = _T("[") + GetResString((hideOSInWork>0)?IDS_POWERSHARE_ON_LABEL:IDS_POWERSHARE_OFF_LABEL) + _T("] ");
					if(file->GetHideOS()<0)
						buffer.Append(_T(" ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
					hideOSInWork = (file->GetHideOS()>=0)?file->GetHideOS():thePrefs.GetHideOvershares();
					if (hideOSInWork>0)
						buffer.AppendFormat(_T("%i"), hideOSInWork);
					else
						buffer.AppendFormat(_T("%s"), GetResString(IDS_DISABLED));
					if (file->GetSelectiveChunk()>=0){
						if (file->GetSelectiveChunk())
							buffer.Append(_T(" + S"));
					}else
						if (thePrefs.IsSelectiveShareEnabled())
							buffer.Append(_T(" + ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". S"));
					break;
				}
				case 20:
					if(file->GetShareOnlyTheNeed()>=0) {
						if (file->GetShareOnlyTheNeed())
							buffer.Format(_T("%i") ,file->GetShareOnlyTheNeed());
						else
							buffer = GetResString(IDS_DISABLED);
					} else {
						buffer = ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". ") + GetResString((thePrefs.GetShareOnlyTheNeed()>0)?IDS_ENABLED:IDS_DISABLED);
					}
					break;
				// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
				// ==> PowerShare [ZZ/MorphXT] - Stulle
				case 21:{
					int powersharemode;
					bool powershared = file->GetPowerShared();
					buffer = _T("[") + GetResString((powershared)?IDS_POWERSHARE_ON_LABEL:IDS_POWERSHARE_OFF_LABEL) + _T("] ");
					if (file->GetPowerSharedMode()>=0)
						powersharemode = file->GetPowerSharedMode();
					else {
						powersharemode = thePrefs.GetPowerShareMode();
						buffer.Append(_T(" ") + ((CString)GetResString(IDS_DEFAULT)).Left(1) + _T(". "));
					}
					if(powersharemode == 2)
						buffer.Append(GetResString(IDS_POWERSHARE_AUTO_LABEL));
					else if (powersharemode == 1)
						buffer.Append(GetResString(IDS_POWERSHARE_ACTIVATED_LABEL));
					else if (powersharemode == 3) {
						buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
						if (file->GetPowerShareLimit()<0)
							buffer.AppendFormat(_T(" %s. %i"), ((CString)GetResString(IDS_DEFAULT)).Left(1), thePrefs.GetPowerShareLimit());
						else
							buffer.AppendFormat(_T(" %i"), file->GetPowerShareLimit());
						// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
						if (file->GetPsAmountLimit()<0)
							buffer.AppendFormat(_T(" %s. %i%%"), ((CString)GetResString(IDS_DEFAULT)).Left(1), thePrefs.GetPsAmountLimit());
						else
							buffer.AppendFormat(_T(" %i%%"), file->GetPsAmountLimit());
						// <== Limit PS by amount of data uploaded [Stulle] - Stulle
					}
					else
						buffer.Append(GetResString(IDS_POWERSHARE_DISABLED_LABEL));
					buffer.Append(_T(" ("));
					if (file->GetPowerShareAuto())
						buffer.Append(GetResString(IDS_POWERSHARE_ADVISED_LABEL));
					else if (file->GetPowerShareLimited() && (powersharemode == 3))
						buffer.Append(GetResString(IDS_POWERSHARE_LIMITED));
					else if (file->GetPowerShareAuthorized())
						buffer.Append(GetResString(IDS_POWERSHARE_AUTHORIZED_LABEL));
					else
						buffer.Append(GetResString(IDS_POWERSHARE_DENIED_LABEL));
					buffer.Append(_T(")"));
					break;
				}
				// <== PowerShare [ZZ/MorphXT] - Stulle
			}

			// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			/*
			if (iColumn != 8)
			*/
			if (iColumn != 8 && iColumn != 14)
			// <== Spread bars [Slugfiller/MorphXT] - Stulle
				dc.DrawText(buffer, buffer.GetLength(), &cur_rec, uDTFlags);
			if (iColumn == 0)
				cur_rec.left -= iIconDrawWidth;
			dc.SetBkColor(crOldBackColor); //Xman Code Improvement: FillSolidRect
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}
	//Xman Code Improvement for ShowFilesCount
	/*
	ShowFilesCount();
	*/
	//Xman end
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		RECT outline_rec = lpDrawItemStruct->rcItem;

		outline_rec.top--;
		outline_rec.bottom++;
		dc.FrameRect(&outline_rec, &CBrush(GetBkColor()));
		outline_rec.top++;
		outline_rec.bottom--;
		outline_rec.left++;
		outline_rec.right--;

		if (lpDrawItemStruct->itemID != 0 && GetItemState(lpDrawItemStruct->itemID - 1, LVIS_SELECTED))
			outline_rec.top--;

		if (lpDrawItemStruct->itemID + 1 != (UINT)GetItemCount() && GetItemState(lpDrawItemStruct->itemID + 1, LVIS_SELECTED))
			outline_rec.bottom++;

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	else if (((lpDrawItemStruct->itemState & ODS_FOCUS) == ODS_FOCUS) && (GetFocus() == this))
	{
		RECT focus_rec;
		focus_rec.top    = lpDrawItemStruct->rcItem.top;
		focus_rec.bottom = lpDrawItemStruct->rcItem.bottom;
		focus_rec.left   = lpDrawItemStruct->rcItem.left + 1;
		focus_rec.right  = lpDrawItemStruct->rcItem.right - 1;
		dc.FrameRect(&focus_rec, &CBrush(m_crNoFocusLine));
	}
	
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CSharedFilesCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// get merged settings
	bool bFirstItem = true;
	int iSelectedItems = GetSelectedCount();
	int iCompleteFileSelected = -1;
	UINT uPrioMenuItem = 0;

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	int iHideOS = -1;
	UINT uHideOSMenuItem = 0;
	UINT uSelectiveChunkMenuItem = 0;
	UINT uShareOnlyTheNeedMenuItem = 0;
	CString buffer;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	int iPowerShareLimit = -1;
	UINT uPowershareMenuItem = 0;
	UINT uPowerShareLimitMenuItem = 0;
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	int iPsAmountLimit = -1;
	UINT uPsAmountLimitMenuItem = 0;
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle

	const CKnownFile* pSingleSelFile = NULL;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos)
	{
		const CKnownFile* pFile = (CKnownFile*)GetItemData(GetNextSelectedItem(pos));
		if (bFirstItem)
			pSingleSelFile = pFile;
		else
			pSingleSelFile = NULL;

		int iCurCompleteFile = pFile->IsPartFile() ? 0 : 1;
		if (bFirstItem)
			iCompleteFileSelected = iCurCompleteFile;
		else if (iCompleteFileSelected != iCurCompleteFile)
			iCompleteFileSelected = -1;

		UINT uCurPrioMenuItem = 0;
		if (pFile->IsAutoUpPriority())
			uCurPrioMenuItem = MP_PRIOAUTO;
		else if (pFile->GetUpPriority() == PR_VERYLOW)
			uCurPrioMenuItem = MP_PRIOVERYLOW;
		else if (pFile->GetUpPriority() == PR_LOW)
			uCurPrioMenuItem = MP_PRIOLOW;
		else if (pFile->GetUpPriority() == PR_NORMAL)
			uCurPrioMenuItem = MP_PRIONORMAL;
		else if (pFile->GetUpPriority() == PR_HIGH)
			uCurPrioMenuItem = MP_PRIOHIGH;
		else if (pFile->GetUpPriority() == PR_VERYHIGH)
			uCurPrioMenuItem = MP_PRIOVERYHIGH;
		//Xman PowerRelease
		else if (pFile->GetUpPriority()==PR_POWER)
			uCurPrioMenuItem = MP_PRIOPOWER;
		//Xman end
		else
			ASSERT(0);

		if (bFirstItem)
			uPrioMenuItem = uCurPrioMenuItem;
		else if (uPrioMenuItem != uCurPrioMenuItem)
			uPrioMenuItem = 0;

		// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		UINT uCurHideOSMenuItem = 0;
		int iCurHideOS = pFile->GetHideOS();
		if (iCurHideOS == -1)
			uCurHideOSMenuItem = MP_HIDEOS_DEFAULT;
		else
			uCurHideOSMenuItem = MP_HIDEOS_SET;
		if (bFirstItem)
		{
			uHideOSMenuItem = uCurHideOSMenuItem;
			iHideOS = iCurHideOS;
		}
		else if (uHideOSMenuItem != uCurHideOSMenuItem || iHideOS != iCurHideOS)
		{
			uHideOSMenuItem = 0;
			iHideOS = -1;
		}
		
		UINT uCurSelectiveChunkMenuItem = 0;
		if (pFile->GetSelectiveChunk() == -1)
			uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK;
		else
			uCurSelectiveChunkMenuItem = MP_SELECTIVE_CHUNK+1 + pFile->GetSelectiveChunk();
		if (bFirstItem)
			uSelectiveChunkMenuItem = uCurSelectiveChunkMenuItem;
		else if (uSelectiveChunkMenuItem != uCurSelectiveChunkMenuItem)
			uSelectiveChunkMenuItem = 0;

		UINT uCurShareOnlyTheNeedMenuItem = 0;
		if (pFile->GetShareOnlyTheNeed() == -1)
			uCurShareOnlyTheNeedMenuItem = MP_SHAREONLYTHENEED;
		else
			uCurShareOnlyTheNeedMenuItem = MP_SHAREONLYTHENEED+1 + pFile->GetShareOnlyTheNeed();
		if (bFirstItem)
			uShareOnlyTheNeedMenuItem = uCurShareOnlyTheNeedMenuItem ;
		else if (uShareOnlyTheNeedMenuItem != uCurShareOnlyTheNeedMenuItem)
			uShareOnlyTheNeedMenuItem = 0;
		// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		// ==> PowerShare [ZZ/MorphXT] - Stulle
		UINT uCurPowershareMenuItem = 0;
		if (pFile->GetPowerSharedMode()==-1)
			uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT;
		else
			uCurPowershareMenuItem = MP_POWERSHARE_DEFAULT+1 + pFile->GetPowerSharedMode();
		
		if (bFirstItem)
			uPowershareMenuItem = uCurPowershareMenuItem;
		else if (uPowershareMenuItem != uCurPowershareMenuItem)
			uPowershareMenuItem = 0;

		UINT uCurPowerShareLimitMenuItem = 0;
		int iCurPowerShareLimit = pFile->GetPowerShareLimit();
		if (iCurPowerShareLimit==-1)
			uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT;
		else
			uCurPowerShareLimitMenuItem = MP_POWERSHARE_LIMIT_SET;
		
		if (bFirstItem)
		{
			uPowerShareLimitMenuItem = uCurPowerShareLimitMenuItem;
			iPowerShareLimit = iCurPowerShareLimit;
		}
		else if (uPowerShareLimitMenuItem != uCurPowerShareLimitMenuItem || iPowerShareLimit != iCurPowerShareLimit)
		{
			uPowerShareLimitMenuItem = 0;
			iPowerShareLimit = -1;
		}
		// <== PowerShare [ZZ/MorphXT] - Stulle

		// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
		UINT uCurPsAmountLimitMenuItem = 0;
		int iCurPsAmountLimit = pFile->GetPsAmountLimit();
		if (iCurPsAmountLimit==-1)
			uCurPsAmountLimitMenuItem = MP_PS_AMOUNT_LIMIT;
		else
			uCurPsAmountLimitMenuItem = MP_PS_AMOUNT_LIMIT_SET;

		if (bFirstItem)
		{
			uPsAmountLimitMenuItem = uCurPsAmountLimitMenuItem;
			iPsAmountLimit = iCurPsAmountLimit;
		}
		else if (uPsAmountLimitMenuItem != uCurPsAmountLimitMenuItem || iPsAmountLimit != iCurPsAmountLimit)
		{
			uPsAmountLimitMenuItem = 0;
			iPsAmountLimit = -1;
		}
		// <== Limit PS by amount of data uploaded [Stulle] - Stulle

		bFirstItem = false;
	}

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PrioMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	//Xman PowerRelease 
	/*
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOAUTO, uPrioMenuItem, 0);
	*/
	m_PrioMenu.CheckMenuRadioItem(MP_PRIOVERYLOW, MP_PRIOPOWER, uPrioMenuItem, 0);
	//Xman end

	bool bSingleCompleteFileSelected = (iSelectedItems == 1 && iCompleteFileSelected == 1);
	m_SharedFilesMenu.EnableMenuItem(MP_OPEN, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	UINT uInsertedMenuItem = 0;
	static const TCHAR _szSkinPkgSuffix1[] = _T(".") EMULSKIN_BASEEXT _T(".zip");
	static const TCHAR _szSkinPkgSuffix2[] = _T(".") EMULSKIN_BASEEXT _T(".rar");
	if (bSingleCompleteFileSelected 
		&& pSingleSelFile 
		&& (   pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix1)-1).CompareNoCase(_szSkinPkgSuffix1) == 0
		|| pSingleSelFile->GetFilePath().Right(ARRSIZE(_szSkinPkgSuffix2)-1).CompareNoCase(_szSkinPkgSuffix2) == 0))
	{
		MENUITEMINFO mii = {0};
		mii.cbSize = sizeof mii;
		mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.fState = MFS_ENABLED;
		mii.wID = MP_INSTALL_SKIN;
		CString strBuff(GetResString(IDS_INSTALL_SKIN));
		mii.dwTypeData = const_cast<LPTSTR>((LPCTSTR)strBuff);
		if (::InsertMenuItem(m_SharedFilesMenu, MP_OPENFOLDER, FALSE, &mii))
			uInsertedMenuItem = mii.wID;
	}
	//Xman PowerRelease
	m_SharedFilesMenu.EnableMenuItem(MP_PRIOPOWER, iCompleteFileSelected >0 ? MF_ENABLED : MF_GRAYED);
	//Xman end
	m_SharedFilesMenu.EnableMenuItem(MP_OPENFOLDER, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_RENAME, bSingleCompleteFileSelected ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_REMOVE, iCompleteFileSelected > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.SetDefaultItem(bSingleCompleteFileSelected ? MP_OPEN : -1);
	m_SharedFilesMenu.EnableMenuItem(MP_CMT, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_DETAIL, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(thePrefs.GetShowCopyEd2kLinkCmd() ? MP_GETED2KLINK : MP_SHOWED2KLINK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_HideOSMenu.m_hMenu, (iSelectedItems > 0 && iCompleteFileSelected > 0) ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_SelectiveChunkMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (thePrefs.GetHideOvershares()==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetHideOvershares());
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_HideOSMenu.ModifyMenu(MP_HIDEOS_DEFAULT, MF_STRING,MP_HIDEOS_DEFAULT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_HideOSMenu.RemoveMenu(MP_HIDEOS_DEFAULT,MF_BYCOMMAND);
	m_HideOSMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_HIDEOS_DEFAULT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	if (iHideOS==-1)
		buffer = GetResString(IDS_EDIT);
	else if (iHideOS==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i"), iHideOS);
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_HideOSMenu.ModifyMenu(MP_HIDEOS_SET, MF_STRING,MP_HIDEOS_SET, buffer);
	*/
	m_HideOSMenu.RemoveMenu(MP_HIDEOS_SET,MF_BYCOMMAND);
	m_HideOSMenu.InsertMenu(2,MF_STRING|MF_BYPOSITION,MP_HIDEOS_SET,buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_HideOSMenu.CheckMenuRadioItem(MP_HIDEOS_DEFAULT, MP_HIDEOS_SET, uHideOSMenuItem, 0);
	buffer.Format(_T(" (%s)"),thePrefs.IsSelectiveShareEnabled()?GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_SelectiveChunkMenu.ModifyMenu(MP_SELECTIVE_CHUNK, MF_STRING, MP_SELECTIVE_CHUNK, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_SelectiveChunkMenu.RemoveMenu(MP_SELECTIVE_CHUNK,MF_BYCOMMAND);
	m_SelectiveChunkMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_SELECTIVE_CHUNK,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_SelectiveChunkMenu.CheckMenuRadioItem(MP_SELECTIVE_CHUNK, MP_SELECTIVE_CHUNK_1, uSelectiveChunkMenuItem, 0);

	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_ShareOnlyTheNeedMenu.m_hMenu, (iSelectedItems > 0 && iCompleteFileSelected > 0) ? MF_ENABLED : MF_GRAYED);
	buffer.Format(_T(" (%s)"),thePrefs.GetShareOnlyTheNeed()?GetResString(IDS_ENABLED):GetResString(IDS_DISABLED));
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_ShareOnlyTheNeedMenu.ModifyMenu(MP_SHAREONLYTHENEED, MF_STRING, MP_SHAREONLYTHENEED, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_ShareOnlyTheNeedMenu.RemoveMenu(MP_SHAREONLYTHENEED,MF_BYCOMMAND);
	m_ShareOnlyTheNeedMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_SHAREONLYTHENEED,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_ShareOnlyTheNeedMenu.CheckMenuRadioItem(MP_SHAREONLYTHENEED, MP_SHAREONLYTHENEED_1, uShareOnlyTheNeedMenuItem, 0);
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.EnableMenuItem(MP_SPREADBAR_RESET, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED); // Spread bars [Slugfiller/MorphXT] - Stulle
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_SharedFilesMenu.EnableMenuItem((UINT_PTR)m_PowershareMenu.m_hMenu, (iSelectedItems > 0 && iCompleteFileSelected > 0) ? MF_ENABLED : MF_GRAYED);
	switch (thePrefs.GetPowerShareMode()){
		case 0:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_DISABLED));
			break;
		case 1:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_ACTIVATED));
			break;
		case 2:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_AUTO));
			break;
		case 3:
			buffer.Format(_T(" (%s)"),GetResString(IDS_POWERSHARE_LIMITED));
			break;
		default:
			buffer = _T(" (?)");
			break;
	}
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PowershareMenu.ModifyMenu(MP_POWERSHARE_DEFAULT, MF_STRING,MP_POWERSHARE_DEFAULT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_PowershareMenu.RemoveMenu(MP_POWERSHARE_DEFAULT,MF_BYCOMMAND);
	m_PowershareMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_POWERSHARE_DEFAULT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_PowershareMenu.CheckMenuRadioItem(MP_POWERSHARE_DEFAULT, MP_POWERSHARE_LIMITED, uPowershareMenuItem, 0);
	m_PowershareMenu.EnableMenuItem((UINT_PTR)m_PowerShareLimitMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (iPowerShareLimit==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%u)"),thePrefs.GetPowerShareLimit());
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PowerShareLimitMenu.ModifyMenu(MP_POWERSHARE_LIMIT, MF_STRING,MP_POWERSHARE_LIMIT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_PowerShareLimitMenu.RemoveMenu(MP_POWERSHARE_LIMIT,MF_BYCOMMAND);
	m_PowerShareLimitMenu.InsertMenu(2,MF_STRING|MF_BYPOSITION,MP_POWERSHARE_LIMIT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	if (iPowerShareLimit==-1)
		buffer = GetResString(IDS_EDIT);
	else if (iPowerShareLimit==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i"),iPowerShareLimit);
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PowerShareLimitMenu.ModifyMenu(MP_POWERSHARE_LIMIT_SET, MF_STRING,MP_POWERSHARE_LIMIT_SET, buffer);
	*/
	m_PowerShareLimitMenu.RemoveMenu(MP_POWERSHARE_LIMIT_SET,MF_BYCOMMAND);
	m_PowerShareLimitMenu.InsertMenu(2,MF_STRING|MF_BYPOSITION,MP_POWERSHARE_LIMIT_SET,buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_PowerShareLimitMenu.CheckMenuRadioItem(MP_POWERSHARE_LIMIT, MP_POWERSHARE_LIMIT_SET, uPowerShareLimitMenuItem, 0);
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	m_PowershareMenu.EnableMenuItem((UINT_PTR)m_PsAmountLimitMenu.m_hMenu, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	if (iPsAmountLimit==0)
		buffer.Format(_T(" (%s)"),GetResString(IDS_DISABLED));
	else
		buffer.Format(_T(" (%i%%)"),thePrefs.GetPsAmountLimit());
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PsAmountLimitMenu.ModifyMenu(MP_PS_AMOUNT_LIMIT, MF_STRING,MP_PS_AMOUNT_LIMIT, GetResString(IDS_DEFAULT) + buffer);
	*/
	m_PsAmountLimitMenu.RemoveMenu(MP_PS_AMOUNT_LIMIT,MF_BYCOMMAND);
	m_PsAmountLimitMenu.InsertMenu(1,MF_STRING|MF_BYPOSITION,MP_PS_AMOUNT_LIMIT,GetResString(IDS_DEFAULT) + buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	if (iPsAmountLimit==-1)
		buffer = GetResString(IDS_EDIT);
	else if (iPsAmountLimit==0)
		buffer = GetResString(IDS_DISABLED);
	else
		buffer.Format(_T("%i%%"),iPsAmountLimit);
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_PsAmountLimitMenu.ModifyMenu(MP_PS_AMOUNT_LIMIT_SET, MF_STRING,MP_PS_AMOUNT_LIMIT_SET, buffer);
	*/
	m_PsAmountLimitMenu.RemoveMenu(MP_PS_AMOUNT_LIMIT_SET,MF_BYCOMMAND);
	m_PsAmountLimitMenu.InsertMenu(2,MF_STRING|MF_BYPOSITION,MP_PS_AMOUNT_LIMIT_SET,buffer);
	// <== XP Style Menu [Xanatos] - Stulle
	m_PowerShareLimitMenu.CheckMenuRadioItem(MP_PS_AMOUNT_LIMIT, MP_PS_AMOUNT_LIMIT_SET, uPsAmountLimitMenuItem, 0);
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> Copy feedback feature [MorphXT] - Stulle
	/*
	// Xman: IcEcRacKer Copy UL-feedback
	m_SharedFilesMenu.EnableMenuItem(MP_ULFEEDBACK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	//Xman end
	*/
	m_SharedFilesMenu.EnableMenuItem(MP_COPYFEEDBACK, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	m_SharedFilesMenu.EnableMenuItem(MP_COPYFEEDBACK_US, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	// <== Copy feedback feature [MorphXT] - Stulle

	m_SharedFilesMenu.EnableMenuItem(MP_FIND, GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED);

	//Xman Mass Rename (Morph)
	m_SharedFilesMenu.EnableMenuItem(MP_MASSRENAME, iSelectedItems > 0 ? MF_ENABLED : MF_GRAYED);
	//Xman end

	m_CollectionsMenu.EnableMenuItem(MP_MODIFYCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_VIEWCOLLECTION, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL ) ? MF_ENABLED : MF_GRAYED);
	m_CollectionsMenu.EnableMenuItem(MP_SEARCHAUTHOR, ( pSingleSelFile != NULL && pSingleSelFile->m_pCollection != NULL && !pSingleSelFile->m_pCollection->GetAuthorKeyHashString().IsEmpty()) ? MF_ENABLED : MF_GRAYED);
#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
	//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		if (iSelectedItems > 0 && theApp.IsConnected() && theApp.IsFirewalled() && theApp.clientlist->GetBuddy())
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_ENABLED);
		else
			m_SharedFilesMenu.EnableMenuItem(MP_GETKADSOURCELINK, MF_GRAYED);
	}
#endif
	m_SharedFilesMenu.EnableMenuItem(Irc_SetSendLink, iSelectedItems == 1 && theApp.emuledlg->ircwnd->IsConnected() ? MF_ENABLED : MF_GRAYED);

	CTitleMenu WebMenu;
	WebMenu.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	WebMenu.AddMenuTitle(NULL, true);
	*/
	WebMenu.AddMenuTitle(GetResString(IDS_WEBSERVICES), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	int iWebMenuEntries = theWebServices.GetFileMenuEntries(&WebMenu);
	UINT flag2 = (iWebMenuEntries == 0 || iSelectedItems != 1) ? MF_GRAYED : MF_STRING;
	m_SharedFilesMenu.AppendMenu(flag2 | MF_POPUP, (UINT_PTR)WebMenu.m_hMenu, GetResString(IDS_WEBSERVICES), _T("WEB"));

	GetPopupMenuPos(*this, point);
	m_SharedFilesMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON,point.x,point.y,this);

	m_SharedFilesMenu.RemoveMenu(m_SharedFilesMenu.GetMenuItemCount()-1,MF_BYPOSITION);
	VERIFY( WebMenu.DestroyMenu() );
	if (uInsertedMenuItem)
		VERIFY( m_SharedFilesMenu.RemoveMenu(uInsertedMenuItem, MF_BYCOMMAND) );
}

BOOL CSharedFilesCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	CTypedPtrList<CPtrList, CKnownFile*> selectedList;
	POSITION pos = GetFirstSelectedItemPosition();
	while (pos != NULL){
		int index = GetNextSelectedItem(pos);
		if (index >= 0)
			selectedList.AddTail((CKnownFile*)GetItemData(index));
	}

	if (   wParam == MP_CREATECOLLECTION
		|| wParam == MP_FIND
		|| selectedList.GetCount() > 0)
	{
		CKnownFile* file = NULL;
		if (selectedList.GetCount() == 1)
			file = selectedList.GetHead();

		switch (wParam){
			case Irc_SetSendLink:
				if (file)
					theApp.emuledlg->ircwnd->SetSendFileString(CreateED2kLink(file));
				break;
			case MP_GETED2KLINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += CreateED2kLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
#if defined(_DEBUG)
			//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
			case MP_GETKADSOURCELINK:{
				CString str;
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (!str.IsEmpty())
						str += _T("\r\n");
					str += theApp.CreateKadSourceLink(file);
				}
				theApp.CopyTextToClipboard(str);
				break;
			}
#endif
			// file operations
			case MP_OPEN:
			case IDA_ENTER:
				if (file && !file->IsPartFile())
					OpenFile(file);
				break; 
			case MP_INSTALL_SKIN:
				if (file && !file->IsPartFile())
					InstallSkin(file->GetFilePath());
				break;
			case MP_OPENFOLDER:
				if (file && !file->IsPartFile())
					ShellExecute(NULL, _T("open"), file->GetPath(), NULL, NULL, SW_SHOW);
				break; 
			case MP_RENAME:
			case MPG_F2:
				if (file && !file->IsPartFile()){
					InputBox inputbox;
					CString title = GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), file->GetFileName());
					inputbox.SetEditFilenameMode();
					inputbox.DoModal();
					CString newname = inputbox.GetInput();
					if (!inputbox.WasCancelled() && newname.GetLength()>0)
					{
						// at least prevent users from specifying something like "..\dir\file"
						static const TCHAR _szInvFileNameChars[] = _T("\\/:*?\"<>|");
						if (newname.FindOneOf(_szInvFileNameChars) != -1){
							AfxMessageBox(GetErrorMessage(ERROR_BAD_PATHNAME));
							break;
						}

						CString newpath;
						PathCombine(newpath.GetBuffer(MAX_PATH), file->GetPath(), newname);
						newpath.ReleaseBuffer();
						if (_trename(file->GetFilePath(), newpath) != 0){
							CString strError;
							strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, _tcserror(errno));
							AfxMessageBox(strError);
							break;
						}
						
						if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
						{
							file->SetFileName(newname);
							STATIC_DOWNCAST(CPartFile, file)->SetFullName(newpath); 
						}
						else
						{
							theApp.sharedfiles->RemoveKeywords(file);
							file->SetFileName(newname);
							theApp.sharedfiles->AddKeywords(file);
						}
						file->SetFilePath(newpath);
						UpdateFile(file);
					}
				}
				else
					MessageBeep(MB_OK);
				break;
			case MP_REMOVE:
			case MPG_DELETE:
			{
				if (IDNO == AfxMessageBox(GetResString(IDS_CONFIRM_FILEDELETE),MB_ICONWARNING | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_YESNO))
					return TRUE;

				SetRedraw(FALSE);
				bool bRemovedItems = false;
				while (!selectedList.IsEmpty())
				{
					CKnownFile* myfile = selectedList.RemoveHead();
					if (!myfile || myfile->IsPartFile())
						continue;
					
					BOOL delsucc = FALSE;
					if (!PathFileExists(myfile->GetFilePath()))
						delsucc = TRUE;
					else{
						// Delete
						if (!thePrefs.GetRemoveToBin()){
							delsucc = DeleteFile(myfile->GetFilePath());
						}
						else{
							// delete to recycle bin :(
							TCHAR todel[MAX_PATH+1];
							memset(todel, 0, sizeof todel);
							_tcsncpy(todel, myfile->GetFilePath(), ARRSIZE(todel)-2);

							SHFILEOPSTRUCT fp = {0};
							fp.wFunc = FO_DELETE;
							fp.hwnd = theApp.emuledlg->m_hWnd;
							fp.pFrom = todel;
							fp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;// | FOF_NOERRORUI
							delsucc = (SHFileOperation(&fp) == 0);
						}
					}
					if (delsucc){
						theApp.sharedfiles->RemoveFile(myfile);
						bRemovedItems = true;
						if (myfile->IsKindOf(RUNTIME_CLASS(CPartFile)))
							theApp.emuledlg->transferwnd->downloadlistctrl.ClearCompleted(static_cast<CPartFile*>(myfile));
					}
					else{
						CString strError;
						strError.Format( GetResString(IDS_ERR_DELFILE) + _T("\r\n\r\n%s"), myfile->GetFilePath(), GetErrorMessage(GetLastError()));
						AfxMessageBox(strError);
					}
				}
				SetRedraw(TRUE);
				if (bRemovedItems) {
					AutoSelectItem();
					// Depending on <no-idea> this does not always cause a
					// LVN_ITEMACTIVATE message sent. So, explicitly redraw
					// the item.
					theApp.emuledlg->sharedfileswnd->ShowSelectedFilesSummary();
				}
				break; 
			}
			case MP_CMT:
				ShowFileDialog(selectedList, IDD_COMMENT);
                break; 
			case MPG_ALTENTER:
			case MP_DETAIL:
				ShowFileDialog(selectedList);
				break;
			case MP_FIND:
				OnFindStart();
				break;
			case MP_CREATECOLLECTION:
			{
				CCollection* pCollection = new CCollection();
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					pCollection->AddFileToCollection(selectedList.GetNext(pos),true);
				}
				CCollectionCreateDialog dialog;
				dialog.SetCollection(pCollection,true);
				dialog.DoModal();
				//We delete this collection object because when the newly created
				//collection file is added to the sharedfile list, it is read and verified
				//and which creates the colleciton object that is attached to that file..
				delete pCollection;
				break;
			}
			case MP_SEARCHAUTHOR:
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					SSearchParams* pParams = new SSearchParams;
					pParams->strExpression = file->m_pCollection->GetCollectionAuthorKeyString();
					pParams->eType = SearchTypeKademlia;
					pParams->strFileType = ED2KFTSTR_EMULECOLLECTION;
					pParams->strSpecialTitle = file->m_pCollection->m_sCollectionAuthorName;
					if (pParams->strSpecialTitle.GetLength() > 50){
						pParams->strSpecialTitle = pParams->strSpecialTitle.Left(50) + _T("...");
					}
					theApp.emuledlg->searchwnd->m_pwndResults->StartSearch(pParams);
				}
				break;
			case MP_VIEWCOLLECTION:
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					CCollectionViewDialog dialog;
					dialog.SetCollection(file->m_pCollection);
					dialog.DoModal();
				}
				break;
			case MP_MODIFYCOLLECTION:
				if (selectedList.GetCount() == 1 && file->m_pCollection)
				{
					CCollectionCreateDialog dialog;
					CCollection* pCollection = new CCollection(file->m_pCollection);
					dialog.SetCollection(pCollection,false);
					dialog.DoModal();
					delete pCollection;				
				}
				break;
			case MP_SHOWED2KLINK:
				ShowFileDialog(selectedList, IDD_ED2KLINK);
				break;
			case MP_PRIOVERYLOW:
			case MP_PRIOLOW:
			case MP_PRIONORMAL:
			case MP_PRIOHIGH:
			case MP_PRIOVERYHIGH:
			case MP_PRIOPOWER:  //Xman PowerRelease
			case MP_PRIOAUTO:
				{
					SetRedraw(FALSE); //Xman Code Improvement
					POSITION pos = selectedList.GetHeadPosition();
					while (pos != NULL)
					{
						CKnownFile* file = selectedList.GetNext(pos);
						switch (wParam) {
							case MP_PRIOVERYLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYLOW);
								UpdateFile(file);
								break;
							case MP_PRIOLOW:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_LOW);
								UpdateFile(file);
								break;
							case MP_PRIONORMAL:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_NORMAL);
								UpdateFile(file);
								break;
							case MP_PRIOHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_HIGH);
								UpdateFile(file);
								break;
							case MP_PRIOVERYHIGH:
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_VERYHIGH);
								UpdateFile(file);
								break;	
							//Xman PowerRelease
							case MP_PRIOPOWER:
								if(file->IsPartFile()) //only to be sure
									break;
								file->SetAutoUpPriority(false);
								file->SetUpPriority(PR_POWER);
								UpdateFile(file);
								break;
							//Xman end
							case MP_PRIOAUTO:
								file->SetAutoUpPriority(true);
								//Xman advanced upload-priority
								if (thePrefs.UseAdvancedAutoPtio())
#ifdef _BETA
									file->CalculateAndSetUploadPriority2(); 
#else
									file->CalculateAndSetUploadPriority(); 
#endif
								else
								//Xman end
									file->UpdateAutoUpPriority();
								UpdateFile(file); 
								break;
						}
					}
					SetRedraw(TRUE); //Xman Code Improvement

					break;
				}
			// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
			case MP_HIDEOS_DEFAULT:
			case MP_HIDEOS_SET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				int newHideOS = -1;
				if (wParam==MP_HIDEOS_SET)
				{
					InputBox inputbox;
					CString title=GetResString(IDS_HIDEOS);
					CString currHideOS;
					if (file)
						currHideOS.Format(_T("%i"), (file->GetHideOS()>=0)?file->GetHideOS():thePrefs.GetHideOvershares());
					else
						currHideOS = _T("0");
					inputbox.SetLabels(GetResString(IDS_HIDEOS), GetResString(IDS_HIDEOVERSHARES), currHideOS);
					inputbox.SetNumber(true);
					int result = inputbox.DoModal();
					if (result == IDCANCEL || (newHideOS = inputbox.GetInputInt()) < 0)
						break;
				}
				SetRedraw(FALSE);
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if  (newHideOS == file->GetHideOS()) break;
					file->SetHideOS(newHideOS);
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			case MP_POWERSHARE_ON:
			case MP_POWERSHARE_OFF:
			case MP_POWERSHARE_DEFAULT:
			case MP_POWERSHARE_AUTO:
			case MP_POWERSHARE_LIMITED:
			{
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					switch (wParam) {
						case MP_POWERSHARE_DEFAULT:
							file->SetPowerShared(-1);
							break;
						case MP_POWERSHARE_ON:
							file->SetPowerShared(1);
							break;
						case MP_POWERSHARE_OFF:
							file->SetPowerShared(0);
							break;
						case MP_POWERSHARE_AUTO:
							file->SetPowerShared(2);
							break;
						case MP_POWERSHARE_LIMITED:
							file->SetPowerShared(3);
							break;
					}
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				theApp.uploadqueue->ReSortUploadSlots(true); // Mephisto Upload - Mephisto
				break;
			}
			case MP_POWERSHARE_LIMIT:
			case MP_POWERSHARE_LIMIT_SET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				int newPowerShareLimit = -1;
				if (wParam==MP_POWERSHARE_LIMIT_SET)
				{
					InputBox inputbox;
					CString title=GetResString(IDS_POWERSHARE);
					CString currPowerShareLimit;
					if (file)
						currPowerShareLimit.Format(_T("%i"), (file->GetPowerShareLimit()>=0)?file->GetPowerShareLimit():thePrefs.GetPowerShareLimit());
					else
						currPowerShareLimit = _T("0");
					inputbox.SetLabels(GetResString(IDS_POWERSHARE), GetResString(IDS_POWERSHARE_LIMIT), currPowerShareLimit);
					inputbox.SetNumber(true);
					int result = inputbox.DoModal();
					if (result == IDCANCEL || (newPowerShareLimit = inputbox.GetInputInt()) < 0)
						break;
				}
				SetRedraw(FALSE);
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if  (newPowerShareLimit == file->GetPowerShareLimit()) break;
					file->SetPowerShareLimit(newPowerShareLimit);
					if (file->IsPartFile())
						((CPartFile*)file)->UpdatePartsInfo();
					else
						file->UpdatePartsInfo();
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			// <== PowerShare [ZZ/MorphXT] - Stulle
			// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
			case MP_PS_AMOUNT_LIMIT:
			case MP_PS_AMOUNT_LIMIT_SET:
			{
				POSITION pos = selectedList.GetHeadPosition();
				int newPsAmountLimit = -1;
				if (wParam==MP_PS_AMOUNT_LIMIT_SET)
				{
					InputBox inputbox;
					CString title=GetResString(IDS_POWERSHARE);
					CString currPsAmountLimit;
					if (file)
						currPsAmountLimit.Format(_T("%i"), ((file->GetPsAmountLimit()>=0.0f)?file->GetPsAmountLimit():thePrefs.GetPsAmountLimit()));
					else
						currPsAmountLimit = _T("0");
					inputbox.SetLabels(GetResString(IDS_POWERSHARE), GetResString(IDS_PS_AMOUNT_LIMIT_LABEL), currPsAmountLimit);
					inputbox.SetNumber(true);
					int result = inputbox.DoModal();
					if (result == IDCANCEL || (newPsAmountLimit = inputbox.GetInputInt()) < 0)
						break;
					if (newPsAmountLimit > MAX_PS_AMOUNT_LIMIT)
					{
						AfxMessageBox(GetResString(IDS_PS_AMOUNT_LIMIT_WRONG),MB_OK | MB_ICONINFORMATION,0);
						break;
					}
				}
				SetRedraw(FALSE);
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if  (newPsAmountLimit == file->GetPsAmountLimit()) break;
					file->SetPsAmountLimit(newPsAmountLimit);
					if (file->IsPartFile())
						((CPartFile*)file)->UpdatePartsInfo();
					else
						file->UpdatePartsInfo();
					UpdateFile(file);
				}
				SetRedraw(TRUE);
				break;
			}
			// <== Limit PS by amount of data uploaded [Stulle] - Stulle
			// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			case MP_SPREADBAR_RESET:
			{
				SetRedraw(FALSE);
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					file->statistic.ResetSpreadBar();
				}
				SetRedraw(TRUE);
				break;
			}
			// <== Spread bars [Slugfiller/MorphXT] - Stulle
			// ==> Copy feedback feature [MorphXT] - Stulle
			/*
			// Xman: idea: IcEcRacKer Copy UL-feedback
			case MP_ULFEEDBACK: 
				{ 
					CString feed; 

					bool morefiles = selectedList.GetCount() > 1;
					uint64 sumTransferred=0;
					uint64 sumAllTimeTransferred=0;

					if(!selectedList.IsEmpty())
					{
						feed.AppendFormat(_T("%s: %s \r\n"), GetResString(IDS_SF_STATISTICS),thePrefs.GetUserNick()); 
						feed.AppendFormat(_T("Mod: %s%s[%s] \r\n"),_T("eMule"), theApp.m_strCurVersionLong, MOD_VERSION);  
					}

					while (!selectedList.IsEmpty())
					{
						CKnownFile* file = selectedList.RemoveHead();
						sumTransferred += file->statistic.GetTransferred();
						sumAllTimeTransferred += file->statistic.GetAllTimeTransferred();

						feed.AppendFormat(_T("%s: %s \r\n"),GetResString(IDS_DL_FILENAME),file->GetFileName()); 
						feed.AppendFormat(_T("%s: %s \r\n"),GetResString(IDS_TYPE),file->GetFileType()); 
						feed.AppendFormat(_T("%s: %s\r\n"),GetResString(IDS_DL_SIZE), CastItoXBytes(file->GetFileSize(), false, false)); 
						CPartFile* pfile = (CPartFile*)file; 
						if(pfile && pfile->IsPartFile()) 
							feed.AppendFormat(_T("%s %.1f%%\r\n"), GetResString(IDS_FD_COMPSIZE), pfile->GetPercentCompleted()); 
						else 
							feed.AppendFormat(_T("%s 100%%\r\n"), GetResString(IDS_FD_COMPSIZE)); 
						feed.AppendFormat(_T("%s: %s (%s) \r\n"),GetResString(IDS_SF_TRANSFERRED), CastItoXBytes(file->statistic.GetTransferred(), false, false), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));   
						feed.AppendFormat(_T("%s: %u (%u)\r\n"),GetResString(IDS_COMPLSOURCES),file->m_nCompleteSourcesCountLo, file->m_nVirtualCompleteSourcesCount); 
						feed.AppendFormat(_T("%s: %u \r\n"),GetResString(IDS_ONQUEUE),(file->GetOnUploadqueue()));  //Xman see OnUploadqueue
						feed.AppendFormat(_T("%s: %u (%u) \r\n\r\n"),GetResString(IDS_SF_ACCEPTS),file->statistic.GetAccepts(),(file->statistic.GetAllTimeAccepts())); 
					}
					if(morefiles)
						feed.AppendFormat(_T("sum: %s: %s (%s) \r\n\r\n"),GetResString(IDS_SF_TRANSFERRED), CastItoXBytes(sumTransferred, false, false), CastItoXBytes(sumAllTimeTransferred, false, false));   

					theApp.CopyTextToClipboard(feed); 
					break; 
				} 
			//Xman end
			*/
 			case MP_COPYFEEDBACK:
			case MP_COPYFEEDBACK_US:
			{
				CString feed;
				uint64 uTransferredSum = 0;
				uint64 uTransferredAllSum = 0;
				int iCount = 0;
				POSITION pos = selectedList.GetHeadPosition();

				if(wParam == MP_COPYFEEDBACK_US)
				{
					if (thePrefs.GetColorFeedback())
						feed.AppendFormat(_T("[color=green][b]Feedback from %s on [%s][/b][/color]\r\n"),thePrefs.GetUserNick(),theApp.m_strModLongVersion);
					else
						feed.AppendFormat(_T("Feedback from %s on [%s]\r\n"),thePrefs.GetUserNick(),theApp.m_strModLongVersion);
				}
				else
				{
					if (thePrefs.GetColorFeedback())
					{
						feed.Append(_T("[color=green][b]"));
						feed.AppendFormat(GetResString(IDS_FEEDBACK_FROM),thePrefs.GetUserNick(), theApp.m_strModLongVersion);
						feed.Append(_T("[/b][/color]\r\n"));
					}
					else
					{
						feed.AppendFormat(GetResString(IDS_FEEDBACK_FROM),thePrefs.GetUserNick(), theApp.m_strModLongVersion);
						feed.Append(_T("\r\n"));
					}
				}

				while (pos != NULL)
				{
					CKnownFile* file = selectedList.GetNext(pos);
					feed.Append(file->GetFeedback(wParam == MP_COPYFEEDBACK_US));
					feed.Append(_T(" \r\n"));

					uTransferredSum += file->statistic.GetTransferred();
					uTransferredAllSum += file->statistic.GetAllTimeTransferred();
					iCount++;
				}

				if(iCount>1)
				{
					if(wParam == MP_COPYFEEDBACK_US)
					{
						if (thePrefs.GetColorFeedback())
							feed.AppendFormat(_T("[color=orange]Transferred (all files):[/color] [color=red]%s (%s)[/color]\r\n"),CastItoXBytes(uTransferredSum,false,false,3,true),CastItoXBytes(uTransferredAllSum,false,false,3,true));
						else
							feed.AppendFormat(_T("Transferred (all files): %s (%s)\r\n"),CastItoXBytes(uTransferredSum,false,false,3,true),CastItoXBytes(uTransferredAllSum,false,false,3,true));
					}
					else
					{
						if (thePrefs.GetColorFeedback())
							feed.AppendFormat(_T("[color=orange]%s:[/color] [color=red]%s (%s)[/color]\r\n"),GetResString(IDS_FEEDBACK_ALL_TRANSFERRED),CastItoXBytes(uTransferredSum,false,false,3),CastItoXBytes(uTransferredAllSum,false,false,3));
						else
							feed.AppendFormat(_T("%s: %s (%s)\r\n"),GetResString(IDS_FEEDBACK_ALL_TRANSFERRED),CastItoXBytes(uTransferredSum,false,false,3),CastItoXBytes(uTransferredAllSum,false,false,3));
					}
				}
				//Todo: copy all the comments too
				theApp.CopyTextToClipboard(feed);
				break;
			}
			// <== Copy feedback feature [MorphXT] - Stulle

			//Xman Mass Rename (Morph)
			case MP_MASSRENAME: 
			{
				CMassRenameDialog MRDialog;
				// Add the files to the dialog
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL) {
					CKnownFile*  file = selectedList.GetAt (pos);
					MRDialog.m_FileList.AddTail (file);
					selectedList.GetNext (pos);
				}
				int result = MRDialog.DoModal ();
				if (result == IDOK) {
					// The user has successfully entered new filenames. Now we have
					// to rename all the files...
					POSITION pos = selectedList.GetHeadPosition();
					int i=0;
					while (pos != NULL) {
						CString newname = MRDialog.m_NewFilenames.at (i);
						CString newpath = MRDialog.m_NewFilePaths.at (i);
						CKnownFile* file = selectedList.GetAt (pos);
						// .part files could be renamed by simply changing the filename
						// in the CKnownFile object.
						if ((!file->IsPartFile()) && (_trename(file->GetFilePath(), newpath) != 0)){
							// Use the "Format"-Syntax of AddLogLine here instead of
							// CString.Format+AddLogLine, because if "%"-characters are
							// in the string they would be misinterpreted as control sequences!
							AddLogLine(false,_T("Failed to rename '%s' to '%s', Error: %hs"), file->GetFilePath(), newpath, _tcserror(errno));
						} else {
							CString strres;
							if (!file->IsPartFile()) {
								// Use the "Format"-Syntax of AddLogLine here instead of
								// CString.Format+AddLogLine, because if "%"-characters are
								// in the string they would be misinterpreted as control sequences!
								AddLogLine(false,_T("Successfully renamed '%s' to '%s'"), file->GetFilePath(), newpath);
								file->SetFileName(newname);
								if (file->IsKindOf(RUNTIME_CLASS(CPartFile)))
									((CPartFile*) file)->SetFullName(newpath);
							} else {
								// Use the "Format"-Syntax of AddLogLine here instead of
								// CString.Format+AddLogLine, because if "%"-characters are
								// in the string they would be misinterpreted as control sequences!
								AddLogLine(false,_T("Successfully renamed .part file '%s' to '%s'"), file->GetFileName(), newname);
								file->SetFileName(newname, true); 
								((CPartFile*) file)->UpdateDisplayedInfo();
								((CPartFile*) file)->SavePartFile(); 
							}
							file->SetFilePath(newpath);
							UpdateFile(file);
						}

						// Next item
						selectedList.GetNext (pos);
						i++;
					}
				}
				break;
			}
			//Xman end
			default:
				// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
				/*
				if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
					theWebServices.RunURL(file, wParam);
				}
				*/
				POSITION pos = selectedList.GetHeadPosition();
				while (pos != NULL)
				{
					file = selectedList.GetNext(pos);
					if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+256){
						theWebServices.RunURL(file, wParam);
					}
					else if (wParam>=MP_SELECTIVE_CHUNK && wParam<=MP_SELECTIVE_CHUNK_1){
						file->SetSelectiveChunk(wParam==MP_SELECTIVE_CHUNK?-1:wParam-MP_SELECTIVE_CHUNK_0);
						UpdateFile(file);
					}else if (wParam>=MP_SHAREONLYTHENEED && wParam<=MP_SHAREONLYTHENEED_1){
						file->SetShareOnlyTheNeed(wParam==MP_SHAREONLYTHENEED?-1:wParam-MP_SHAREONLYTHENEED_0);
						UpdateFile(file);
					}
				}
				// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
				break;
		}
	}
	return TRUE;
}

void CSharedFilesCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column

	bool sortAscending = (GetSortItem() != pNMListView->iSubItem) ? true : !GetSortAscending();

	// Ornis 4-way-sorting
	int adder=0;
	if (pNMListView->iSubItem>=5 && pNMListView->iSubItem<=7)
	{
		ASSERT( pNMListView->iSubItem - 5 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[pNMListView->iSubItem - 5] = !sortstat[pNMListView->iSubItem - 5];
		adder = sortstat[pNMListView->iSubItem-5] ? 0 : 100;
	}
	else if (pNMListView->iSubItem==11)
	{
		ASSERT( 3 < ARRSIZE(sortstat) );
		if (!sortAscending)
			sortstat[3] = !sortstat[3];
		adder = sortstat[3] ? 0 : 100;
	}

	// Sort table
	if (adder==0)	
		SetSortArrow(pNMListView->iSubItem, sortAscending); 
	else
		SetSortArrow(pNMListView->iSubItem, sortAscending ? arrowDoubleUp : arrowDoubleDown);

	UpdateSortHistory(pNMListView->iSubItem + adder + (sortAscending ? 0:20),20);
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	/*
	SortItems(SortProc, pNMListView->iSubItem + adder + (sortAscending ? 0:20));
	*/
	SortItems(SortProc, pNMListView->iSubItem + adder + (sortAscending ? 0:30));
	// <== PowerShare [ZZ/MorphXT] - Stulle

	*pResult = 0;
}

int CSharedFilesCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CKnownFile* item1 = (CKnownFile*)lParam1;
	const CKnownFile* item2 = (CKnownFile*)lParam2;
	
	// ==> PowerShare [ZZ/MorphXT] - Stulle

	// all shifted by 10!!!

	// <== PowerShare [ZZ/MorphXT] - Stulle
	
	int iResult=0;
	switch(lParamSort){
		case 0: //filename asc
			iResult=CompareLocaleStringNoCase(item1->GetFileName(),item2->GetFileName());
			break;
		case 30: //filename desc
			iResult=CompareLocaleStringNoCase(item2->GetFileName(),item1->GetFileName());
			break;

		case 1: //filesize asc
			iResult=item1->GetFileSize()==item2->GetFileSize()?0:(item1->GetFileSize()>item2->GetFileSize()?1:-1);
			break;

		case 31: //filesize desc
			iResult=item1->GetFileSize()==item2->GetFileSize()?0:(item2->GetFileSize()>item1->GetFileSize()?1:-1);
			break;

		case 2: //filetype asc
			iResult=item1->GetFileTypeDisplayStr().Compare(item2->GetFileTypeDisplayStr());
			break;
		case 32: //filetype desc
			iResult=item2->GetFileTypeDisplayStr().Compare(item1->GetFileTypeDisplayStr());
			break;

		case 3: //prio asc
		{
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			if (!item1->GetPowerShared() && item2->GetPowerShared())
				iResult=-1;			
			else if (item1->GetPowerShared() && !item2->GetPowerShared())
				iResult=1;
			else
			{
			// <== PowerShare [ZZ/MorphXT] - Stulle
			uint8 p1=item1->GetUpPriority() +1;
			if(p1==5)
				p1=0;
			uint8 p2=item2->GetUpPriority() +1;
			if(p2==5)
				p2=0;
			iResult=p1-p2;
			} // PowerShare [ZZ/MorphXT] - Stulle
			break;
		}
		case 33: //prio desc
		{
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			if (!item2->GetPowerShared() && item1->GetPowerShared())
				iResult=-1;			
			else if (item2->GetPowerShared() && !item1->GetPowerShared())
				iResult=1;
			else
			{
			// <== PowerShare [ZZ/MorphXT] - Stulle
				uint8 p1=item1->GetUpPriority() +1;
				if(p1==5)
					p1=0;
				uint8 p2=item2->GetUpPriority() +1;
				if(p2==5)
					p2=0;
				iResult=p2-p1;
			} // PowerShare [ZZ/MorphXT] - Stulle
			break;
		}

		case 4: //fileID asc
			iResult=memcmp(item1->GetFileHash(), item2->GetFileHash(), 16);
			break;
		case 34: //fileID desc
			iResult=memcmp(item2->GetFileHash(), item1->GetFileHash(), 16);
			break;

		case 5: //requests asc
			iResult=item1->statistic.GetRequests() - item2->statistic.GetRequests();
			break;
		case 35: //requests desc
			iResult=item2->statistic.GetRequests() - item1->statistic.GetRequests();
			break;
		
		case 6: //acc requests asc
			iResult=item1->statistic.GetAccepts() - item2->statistic.GetAccepts();
			break;
		case 36: //acc requests desc
			iResult=item2->statistic.GetAccepts() - item1->statistic.GetAccepts();
			break;
		
		case 7: //all transferred asc
			iResult=item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item1->statistic.GetTransferred()>item2->statistic.GetTransferred()?1:-1);
			break;
		case 37: //all transferred desc
			iResult=item1->statistic.GetTransferred()==item2->statistic.GetTransferred()?0:(item2->statistic.GetTransferred()>item1->statistic.GetTransferred()?1:-1);
			break;

		case 9: //folder asc
			iResult=CompareLocaleStringNoCase(item1->GetPath(),item2->GetPath());
			break;
		case 39: //folder desc
			iResult=CompareLocaleStringNoCase(item2->GetPath(),item1->GetPath());
			break;

		case 10: //complete sources asc
			iResult=CompareUnsigned(item1->m_nCompleteSourcesCount, item2->m_nCompleteSourcesCount);
			break;
		case 40: //complete sources desc
			iResult=CompareUnsigned(item2->m_nCompleteSourcesCount, item1->m_nCompleteSourcesCount);
			break;

		case 11: //ed2k shared asc
			iResult=item1->GetPublishedED2K() - item2->GetPublishedED2K();
			break;
		case 41: //ed2k shared desc
			iResult=item2->GetPublishedED2K() - item1->GetPublishedED2K();
			break;

		//Xman see OnUploadqueue
		case 12:
			iResult= item1->GetOnUploadqueue() -item2->GetOnUploadqueue();
			break;
		case 42:
			iResult= item2->GetOnUploadqueue() -item1->GetOnUploadqueue();
			break;
		//Xman end

		//Xman advanced upload-priority
		//sort by the second value-> faster
		case 13:
			{
				float it1value= item1->statistic.GetAllTimeTransferred()/(float)item1->GetFileSize()*1000; //sort one number after ,
				float it2value= item2->statistic.GetAllTimeTransferred()/(float)item2->GetFileSize()*1000;
				iResult=(int)(it1value-it2value);
				break;
			}
		case 43:
			{
				float it1value= item1->statistic.GetAllTimeTransferred()/(float)item1->GetFileSize()*1000; //sort one number after ,
				float it2value= item2->statistic.GetAllTimeTransferred()/(float)item2->GetFileSize()*1000;
				iResult=(int)(it2value-it1value);
				break;
			}
		//Xman end

		// ==> Spread bars [Slugfiller/MorphXT] - Stulle
		case 14: //spread asc
		case 15:
			iResult=CompareFloat(((CKnownFile*)lParam1)->statistic.GetSpreadSortValue(),((CKnownFile*)lParam2)->statistic.GetSpreadSortValue());
			break;
		case 44: //spread desc
		case 45:
			iResult=CompareFloat(((CKnownFile*)lParam2)->statistic.GetSpreadSortValue(),((CKnownFile*)lParam1)->statistic.GetSpreadSortValue());
			break;

		case 16: // VQB:  Simple UL asc
		case 46: //VQB:  Simple UL desc
			{
				float x1 = ((float)item1->statistic.GetAllTimeTransferred())/((float)item1->GetFileSize());
				float x2 = ((float)item2->statistic.GetAllTimeTransferred())/((float)item2->GetFileSize());
				if (lParamSort == 15) iResult=CompareFloat(x1,x2); else iResult=CompareFloat(x2,x1);
			break;
			}
		case 17: // SF:  Full Upload Count asc
			iResult=CompareFloat(((CKnownFile*)lParam1)->statistic.GetFullSpreadCount(),((CKnownFile*)lParam2)->statistic.GetFullSpreadCount());
			break;
		case 47: // SF:  Full Upload Count desc
			iResult=CompareFloat(((CKnownFile*)lParam2)->statistic.GetFullSpreadCount(),((CKnownFile*)lParam1)->statistic.GetFullSpreadCount());
			break;
		// <== Spread bars [Slugfiller/MorphXT] - Stulle
		// ==> push rare file - Stulle
		case 18:
			iResult=CompareFloat(item1->GetFileRatio(),item2->GetFileRatio());
		case 48:
			iResult=CompareFloat(item2->GetFileRatio(),item1->GetFileRatio());
		// <== push rare file - Stulle
		// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		case 19:
			if (item1->GetHideOS() == item2->GetHideOS())
				iResult=item1->GetSelectiveChunk() - item2->GetSelectiveChunk();
			else
				iResult=item1->GetHideOS() - item2->GetHideOS();
			break;
		case 49:
			if (item2->GetHideOS() == item1->GetHideOS())
				iResult=item2->GetSelectiveChunk() - item1->GetSelectiveChunk();
			else
				iResult=item2->GetHideOS() - item1->GetHideOS();
			break;
		case 20:
			iResult=item1->GetShareOnlyTheNeed() - item2->GetShareOnlyTheNeed();
			break;
		case 50:
			iResult=item2->GetShareOnlyTheNeed() - item1->GetShareOnlyTheNeed();
			break;
		// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		// ==> PowerShare [ZZ/MorphXT] - Stulle
		case 21:
			if (!item1->GetPowerShared() && item2->GetPowerShared())
				iResult=-1;
			else if (item1->GetPowerShared() && !item2->GetPowerShared())
				iResult=1;
			else
				if (item1->GetPowerSharedMode() != item2->GetPowerSharedMode())
					iResult=item1->GetPowerSharedMode() - item2->GetPowerSharedMode();
				else
					if (!item1->GetPowerShareAuthorized() && item2->GetPowerShareAuthorized())
						iResult=-1;
					else if (item1->GetPowerShareAuthorized() && !item2->GetPowerShareAuthorized())
						iResult=1;
					else
						if (!item1->GetPowerShareAuto() && item2->GetPowerShareAuto())
							iResult=-1;
						else if (item1->GetPowerShareAuto() && !item2->GetPowerShareAuto())
							iResult=1;
						else
							if (!item1->GetPowerShareLimited() && item2->GetPowerShareLimited())
								iResult=-1;
							else if (item1->GetPowerShareLimited() && !item2->GetPowerShareLimited())
								iResult=1;
							else
								iResult=0;
			break;
		case 51:
			if (!item2->GetPowerShared() && item1->GetPowerShared())
				iResult=-1;
			else if (item2->GetPowerShared() && !item1->GetPowerShared())
				iResult=1;
			else
				if (item1->GetPowerSharedMode() != item2->GetPowerSharedMode())
					iResult=item2->GetPowerSharedMode() - item1->GetPowerSharedMode();
				else
					if (!item2->GetPowerShareAuthorized() && item1->GetPowerShareAuthorized())
						iResult=-1;
					else if (item2->GetPowerShareAuthorized() && !item1->GetPowerShareAuthorized())
						iResult=1;
					else
						if (!item2->GetPowerShareAuto() && item1->GetPowerShareAuto())
							iResult=-1;
						else if (item2->GetPowerShareAuto() && !item1->GetPowerShareAuto())
							iResult=1;
						else
							if (!item2->GetPowerShareLimited() && item1->GetPowerShareLimited())
								iResult=-1;
							else if (item2->GetPowerShareLimited() && !item1->GetPowerShareLimited())
								iResult=1;
							else
								iResult=0;
			break;
		// <== PowerShare [ZZ/MorphXT] - Stulle
		case 105: //all requests asc
			iResult=CompareUnsigned(item1->statistic.GetAllTimeRequests(), item2->statistic.GetAllTimeRequests());
			break;
		case 135: //all requests desc
			iResult=CompareUnsigned(item2->statistic.GetAllTimeRequests(), item1->statistic.GetAllTimeRequests());
			break;

		case 106: //all acc requests asc
			iResult=CompareUnsigned(item1->statistic.GetAllTimeAccepts(), item2->statistic.GetAllTimeAccepts());
			break;
		case 136: //all acc requests desc
			iResult=CompareUnsigned(item2->statistic.GetAllTimeAccepts(), item1->statistic.GetAllTimeAccepts());
			break;

		case 107: //all transferred asc
			iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item1->statistic.GetAllTimeTransferred()>item2->statistic.GetAllTimeTransferred()?1:-1);
			break;
		case 137: //all transferred desc
			iResult=item1->statistic.GetAllTimeTransferred()==item2->statistic.GetAllTimeTransferred()?0:(item2->statistic.GetAllTimeTransferred()>item1->statistic.GetAllTimeTransferred()?1:-1);
			break;

		case 111:{ //kad shared asc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			iResult=i1 - i2;
			break;
		}
		case 141:{ //kad shared desc
			uint32 tNow = time(NULL);
			int i1 = (tNow < item1->GetLastPublishTimeKadSrc()) ? 1 : 0;
			int i2 = (tNow < item2->GetLastPublishTimeKadSrc()) ? 1 : 0;
			iResult=i2 - i1;
			break;
		}
		default: 
			iResult=0;
			break;
	}
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->sharedfileswnd->sharedfilesctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/

	return iResult;

}

void CSharedFilesCtrl::OpenFile(const CKnownFile* file)
{
	if(file->m_pCollection)
	{
		CCollectionViewDialog dialog;
		dialog.SetCollection(file->m_pCollection);
		dialog.DoModal();
	}
	else
		ShellOpenFile(file->GetFilePath(), NULL);
}

void CSharedFilesCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1)
	{
		CKnownFile* file = (CKnownFile*)GetItemData(iSel);
		if (file)
		{
			if (GetKeyState(VK_MENU) & 0x8000)
			{
				CTypedPtrList<CPtrList, CKnownFile*> aFiles;
				aFiles.AddHead(file);
				ShowFileDialog(aFiles);
			}
			else if (!file->IsPartFile())
				OpenFile(file);
		}
	}
	*pResult = 0;
}

void CSharedFilesCtrl::CreateMenues()
{
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	if (m_PowershareMenu) VERIFY( m_PowershareMenu.DestroyMenu() );
	if (m_PowerShareLimitMenu) VERIFY( m_PowerShareLimitMenu.DestroyMenu() );
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	if (m_PsAmountLimitMenu) VERIFY( m_PsAmountLimitMenu.DestroyMenu() );
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	if (m_HideOSMenu) VERIFY( m_HideOSMenu.DestroyMenu() );
	if (m_SelectiveChunkMenu) VERIFY( m_SelectiveChunkMenu.DestroyMenu() );
	if (m_ShareOnlyTheNeedMenu) VERIFY( m_ShareOnlyTheNeedMenu.DestroyMenu() );
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	if (m_PrioMenu) VERIFY( m_PrioMenu.DestroyMenu() );
	if (m_CollectionsMenu) VERIFY( m_CollectionsMenu.DestroyMenu() );
	if (m_SharedFilesMenu) VERIFY( m_SharedFilesMenu.DestroyMenu() );

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_PowershareMenu.CreateMenu();
	m_PowershareMenu.AddMenuTitle(GetResString(IDS_POWERSHARE), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_DEFAULT,GetResString(IDS_DEFAULT));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_OFF,GetResString(IDS_POWERSHARE_DISABLED));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_ON,GetResString(IDS_POWERSHARE_ACTIVATED));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_AUTO,GetResString(IDS_POWERSHARE_AUTO));
	m_PowershareMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMITED,GetResString(IDS_POWERSHARE_LIMITED)); 
	m_PowerShareLimitMenu.CreateMenu();
	m_PowerShareLimitMenu.AddMenuTitle(GetResString(IDS_POWERSHARE_LIMITED), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PowerShareLimitMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMIT,	GetResString(IDS_DEFAULT));
	m_PowerShareLimitMenu.AppendMenu(MF_STRING,MP_POWERSHARE_LIMIT_SET,	GetResString(IDS_DISABLED));
	// <== PowerShare [ZZ/MorphXT] - Stulle

	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
	m_PsAmountLimitMenu.CreateMenu();
	m_PsAmountLimitMenu.AddMenuTitle(GetResString(IDS_PS_LIMITED_AMNT), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PsAmountLimitMenu.AppendMenu(MF_STRING,MP_PS_AMOUNT_LIMIT,	GetResString(IDS_DEFAULT));
	m_PsAmountLimitMenu.AppendMenu(MF_STRING,MP_PS_AMOUNT_LIMIT_SET,	GetResString(IDS_DISABLED));
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_HideOSMenu.CreateMenu();
	m_HideOSMenu.AddMenuTitle(GetResString(IDS_HIDEOS), true, false); // XP Style Menu [Xanatos] - Stulle
	m_HideOSMenu.AppendMenu(MF_STRING,MP_HIDEOS_DEFAULT, GetResString(IDS_DEFAULT));
	m_HideOSMenu.AppendMenu(MF_STRING,MP_HIDEOS_SET, GetResString(IDS_DISABLED));
	m_SelectiveChunkMenu.CreateMenu();
	m_SelectiveChunkMenu.AddMenuTitle(GetResString(IDS_SELECTIVESHARE), true, false); // XP Style Menu [Xanatos] - Stulle
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK,	GetResString(IDS_DEFAULT));
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK_0,	GetResString(IDS_DISABLED));
	m_SelectiveChunkMenu.AppendMenu(MF_STRING,MP_SELECTIVE_CHUNK_1,	GetResString(IDS_ENABLED));

	m_ShareOnlyTheNeedMenu.CreateMenu();
	m_ShareOnlyTheNeedMenu.AddMenuTitle(GetResString(IDS_SHAREONLYTHENEED), true, false); // XP Style Menu [Xanatos] - Stulle
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED,	GetResString(IDS_DEFAULT));
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED_0,	GetResString(IDS_DISABLED));
	m_ShareOnlyTheNeedMenu.AppendMenu(MF_STRING,MP_SHAREONLYTHENEED_1,	GetResString(IDS_ENABLED));
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle


	m_PrioMenu.CreateMenu();
	m_PrioMenu.AddMenuTitle(GetResString(IDS_PRIORITY), true, false); // XP Style Menu [Xanatos] - Stulle
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYLOW,GetResString(IDS_PRIOVERYLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOLOW,GetResString(IDS_PRIOLOW));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIONORMAL,GetResString(IDS_PRIONORMAL));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOHIGH, GetResString(IDS_PRIOHIGH));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOVERYHIGH, GetResString(IDS_PRIORELEASE));
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOPOWER, GetResString(IDS_POWERRELEASE)); //Xman PowerRelease
	m_PrioMenu.AppendMenu(MF_STRING,MP_PRIOAUTO, GetResString(IDS_PRIOAUTO));//UAP

	m_CollectionsMenu.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	m_CollectionsMenu.AddMenuTitle(NULL, true);
	*/
	m_CollectionsMenu.AddMenuTitle(GetResString(IDS_SEARCH_EMULECOLLECTION), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_CREATECOLLECTION, GetResString(IDS_CREATECOLLECTION), _T("COLLECTION_ADD"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_MODIFYCOLLECTION, GetResString(IDS_MODIFYCOLLECTION), _T("COLLECTION_EDIT"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_VIEWCOLLECTION, GetResString(IDS_VIEWCOLLECTION), _T("COLLECTION_VIEW"));
	m_CollectionsMenu.AppendMenu(MF_STRING,MP_SEARCHAUTHOR, GetResString(IDS_SEARCHAUTHORCOLLECTION), _T("COLLECTION_SEARCH"));

	m_SharedFilesMenu.CreatePopupMenu();
	m_SharedFilesMenu.AddMenuTitle(GetResString(IDS_SHAREDFILES), true);

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPEN, GetResString(IDS_OPENFILE), _T("OPENFILE"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_OPENFOLDER, GetResString(IDS_OPENFOLDER), _T("OPENFOLDER"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_RENAME, GetResString(IDS_RENAME) + _T("..."), _T("FILERENAME"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_REMOVE, GetResString(IDS_DELETE), _T("DELETE"));
	if (thePrefs.IsExtControlsEnabled())
		m_SharedFilesMenu.AppendMenu(MF_STRING,Irc_SetSendLink,GetResString(IDS_IRC_ADDLINKTOIRC), _T("IRCCLIPBOARD"));

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PrioMenu.m_hMenu, GetResString(IDS_PRIORITY) + _T(" (") + GetResString(IDS_PW_CON_UPLBL) + _T(")"), _T("FILEPRIORITY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_CollectionsMenu.m_hMenu, GetResString(IDS_META_COLLECTION), _T("COLLECTION"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 	

	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("FILEINFO"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_CMT, GetResString(IDS_CMT_ADD), _T("FILECOMMENTS")); 
	if (thePrefs.GetShowCopyEd2kLinkCmd())
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETED2KLINK, GetResString(IDS_DL_LINK1), _T("ED2KLINK") );
	else
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SHOWED2KLINK, GetResString(IDS_DL_SHOWED2KLINK), _T("ED2KLINK") );
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_FIND, GetResString(IDS_FIND), _T("Search"));
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 

	m_SharedFilesMenu.AppendHeading(GetResString(IDS_RELEASER));
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PowershareMenu.m_hMenu, GetResString(IDS_POWERSHARE), _T("FILEPOWERSHARE"));
	m_PowershareMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_PowershareMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PowerShareLimitMenu.m_hMenu, GetResString(IDS_POWERSHARE_LIMIT));
	// <== PowerShare [ZZ/MorphXT] - Stulle
	// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
    m_PowershareMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_PsAmountLimitMenu.m_hMenu, GetResString(IDS_PS_AMOUNT_LIMIT));
	// <== Limit PS by amount of data uploaded [Stulle] - Stulle
	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_HideOSMenu.m_hMenu, GetResString(IDS_HIDEOS), _T("FILEHIDEOS"));
	m_HideOSMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	m_HideOSMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_SelectiveChunkMenu.m_hMenu, GetResString(IDS_SELECTIVESHARE));

	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)m_ShareOnlyTheNeedMenu.m_hMenu, GetResString(IDS_SHAREONLYTHENEED), _T("FILESHAREONLYTHENEED"));
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_SPREADBAR_RESET, GetResString(IDS_SPREAD_RESET), _T("RESETSPREADBAR")); // Spread bars [Slugfiller/MorphXT] - Stulle

	// ==> Copy feedback feature [MorphXT] - Stulle
	/*
	// Xman: IcEcRacKer Copy UL-feedback
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_ULFEEDBACK, _T("Copy UL-Feedback"), _T("FILECOMMENTS")); 
	m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	//Xman end
	*/
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK, GetResString(IDS_COPYFEEDBACK), _T("COPY"));
	m_SharedFilesMenu.AppendMenu(MF_STRING,MP_COPYFEEDBACK_US, GetResString(IDS_COPYFEEDBACK_US), _T("COPY"));
	m_SharedFilesMenu.AppendMenu(MF_SEPARATOR);
	// <== Copy feedback feature [MorphXT] - Stulle

	//Xman Mass Rename (Morph)
	if (thePrefs.IsExtControlsEnabled())
	{
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_MASSRENAME,GetResString(IDS_MR), _T("FILEMASSRENAME"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR); 
	}
	//Xman end

#if defined(_DEBUG)
	if (thePrefs.IsExtControlsEnabled()){
		//JOHNTODO: Not for release as we need kad lowID users in the network to see how well this work work. Also, we do not support these links yet.
		m_SharedFilesMenu.AppendMenu(MF_STRING,MP_GETKADSOURCELINK, _T("Copy eD2K Links To Clipboard (Kad)"));
		m_SharedFilesMenu.AppendMenu(MF_STRING|MF_SEPARATOR);
	}
#endif
}

void CSharedFilesCtrl::ShowComments(CKnownFile* file)
{
	if (file)
	{
		CTypedPtrList<CPtrList, CKnownFile*> aFiles;
		aFiles.AddHead(file);
		ShowFileDialog(aFiles, IDD_COMMENT);
	}
}

void CSharedFilesCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	if (theApp.emuledlg->IsRunning()){
		// Although we have an owner drawn listview control we store the text for the primary item in the listview, to be
		// capable of quick searching those items via the keyboard. Because our listview items may change their contents,
		// we do this via a text callback function. The listview control will send us the LVN_DISPINFO notification if
		// it needs to know the contents of the primary item.
		//
		// But, the listview control sends this notification all the time, even if we do not search for an item. At least
		// this notification is only sent for the visible items and not for all items in the list. Though, because this
		// function is invoked *very* often, no *NOT* put any time consuming code here in.

		if (pDispInfo->item.mask & LVIF_TEXT){
			const CKnownFile* pFile = reinterpret_cast<CKnownFile*>(pDispInfo->item.lParam);
			if (pFile != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pFile->GetFileName(), pDispInfo->item.cchTextMax);
							pDispInfo->item.pszText[pDispInfo->item.cchTextMax-1] = _T('\0');
						}
						break;
					default:
						// shouldn't happen
						pDispInfo->item.pszText[0] = _T('\0');
						break;
				}
			}
		}
	}
	*pResult = 0;
}

void CSharedFilesCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'C' && (GetKeyState(VK_CONTROL) & 0x8000))
	{
		// Ctrl+C: Copy listview items to clipboard
		SendMessage(WM_COMMAND, MP_GETED2KLINK);
		return;
	}
	else if (nChar == VK_F5)
		ReloadFileList();

	CMuleListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSharedFilesCtrl::ShowFileDialog(CTypedPtrList<CPtrList, CKnownFile*>& aFiles, UINT uPshInvokePage)
{
	if (aFiles.GetSize() > 0)
	{
		CSharedFileDetailsSheet dialog(aFiles, uPshInvokePage, this);
		dialog.DoModal();
	}
}

void CSharedFilesCtrl::SetDirectoryFilter(CDirectoryItem* pNewFilter, bool bRefresh){
	if (m_pDirectoryFilter == pNewFilter)
		return;
	m_pDirectoryFilter = pNewFilter;
	if (bRefresh)
		ReloadFileList();
}

void CSharedFilesCtrl::OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMLVGETINFOTIP>(pNMHDR);
	if (pGetInfoTip->iSubItem == 0)
	{
		LVHITTESTINFO hti = {0};
		::GetCursorPos(&hti.pt);
		ScreenToClient(&hti.pt);
		if (SubItemHitTest(&hti) == -1 || hti.iItem != pGetInfoTip->iItem || hti.iSubItem != 0){
			// don' show the default label tip for the main item, if the mouse is not over the main item
			if ((pGetInfoTip->dwFlags & LVGIT_UNFOLDED) == 0 && pGetInfoTip->cchTextMax > 0 && pGetInfoTip->pszText[0] != _T('\0'))
				pGetInfoTip->pszText[0] = _T('\0');
			return;
		}

		const CKnownFile* pFile = (CKnownFile*)GetItemData(pGetInfoTip->iItem);
		if (pFile && pGetInfoTip->pszText && pGetInfoTip->cchTextMax > 0)
		{
			CString strInfo = pFile->GetInfoSummary();
			strInfo += TOOLTIP_AUTOFORMAT_SUFFIX_CH;
			_tcsncpy(pGetInfoTip->pszText, strInfo, pGetInfoTip->cchTextMax);
			pGetInfoTip->pszText[pGetInfoTip->cchTextMax-1] = _T('\0');
		}
	}
	*pResult = 0;
}

bool CSharedFilesCtrl::IsFilteredItem(const CKnownFile* pKnownFile) const
{
	const CStringArray& rastrFilter = theApp.emuledlg->sharedfileswnd->m_astrFilter;
	if (rastrFilter.GetSize() == 0)
		return false;

	// filtering is done by text only for all colums to keep it consistent and simple for the user even if that
	// doesn't allows complex filters
	TCHAR szFilterTarget[256];
	GetItemDisplayText(pKnownFile, theApp.emuledlg->sharedfileswnd->GetFilterColumn(),
					   szFilterTarget, _countof(szFilterTarget));

	bool bItemFiltered = false;
	for (int i = 0; i < rastrFilter.GetSize(); i++)
	{
		const CString& rstrExpr = rastrFilter.GetAt(i);
		bool bAnd = true;
		LPCTSTR pszText = (LPCTSTR)rstrExpr;
		if (pszText[0] == _T('-')) {
			pszText += 1;
			bAnd = false;
		}

		bool bFound = (stristr(szFilterTarget, pszText) != NULL);
		if ((bAnd && !bFound) || (!bAnd && bFound)) {
			bItemFiltered = true;
			break;
		}
	}
	return bItemFiltered;
}
