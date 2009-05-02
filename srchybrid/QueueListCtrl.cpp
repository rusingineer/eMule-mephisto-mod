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
#include "QueueListCtrl.h"
#include "OtherFunctions.h"
#include "MenuCmds.h"
#include "ClientDetailDialog.h"
#include "Exceptions.h"
#include "KademliaWnd.h"
#include "emuledlg.h"
#include "FriendList.h"
#include "UploadQueue.h"
#include "UpDownClient.h"
#include "TransferWnd.h"
#include "MemDC.h"
#include "SharedFileList.h"
#include "ClientCredits.h"
#include "PartFile.h"
#include "ChatWnd.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "Log.h"
//Xman
#include "ListenSocket.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CQueueListCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CQueueListCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

CQueueListCtrl::CQueueListCtrl()
	: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true, false);

	// Barry - Refresh the queue every 10 secs
	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 10000, QueueUpdateTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'queue list control' timer - %s"),GetErrorMessage(GetLastError()));
}

void CQueueListCtrl::Init()
{
	SetName(_T("QueueListCtrl"));

	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(0,GetResString(IDS_QL_USERNAME),LVCFMT_LEFT,150,0);
	InsertColumn(1,GetResString(IDS_FILE),LVCFMT_LEFT,275,1);
	InsertColumn(2,GetResString(IDS_FILEPRIO),LVCFMT_LEFT,110,2);
	InsertColumn(3,GetResString(IDS_QL_RATING),LVCFMT_LEFT,60,3);
	InsertColumn(4,GetResString(IDS_SCORE),LVCFMT_LEFT,60,4);
	InsertColumn(5,GetResString(IDS_ASKED),LVCFMT_LEFT,60,5);
	InsertColumn(6,GetResString(IDS_LASTSEEN),LVCFMT_LEFT,110,6);
	InsertColumn(7,GetResString(IDS_ENTERQUEUE),LVCFMT_LEFT,110,7);
	InsertColumn(8,GetResString(IDS_BANNED),LVCFMT_LEFT,60,8);
	InsertColumn(9,GetResString(IDS_UPSTATUS),LVCFMT_LEFT,100,9);
	InsertColumn(10,GetResString(IDS_CD_CSOFT), LVCFMT_LEFT, 90, 10);	//Xman version see clientversion in every window
	InsertColumn(11, GetResString(IDS_UPDOWNUPLOADLIST), LVCFMT_LEFT, 90, 11); //Xman show complete up/down in queuelist

	// ==> push small files [sivka] - Stulle
	InsertColumn(12,GetResString(IDS_SMALL),LVCFMT_LEFT,40,12);
	// <== push small files [sivka] - Stulle

	// ==> push rare file - Stulle
	InsertColumn(13,GetResString(IDS_RARE),LVCFMT_LEFT,40,13);
	// <== push rare file - Stulle

	SetAllIcons();
	Localize();
	LoadSettings();

	//Xman client percentage
	CFont* pFont = GetFont();
	LOGFONT lfFont = {0};
	pFont->GetLogFont(&lfFont);
	lfFont.lfHeight = 11;
	m_fontBoldSmaller.CreateFontIndirect(&lfFont);
	//Xman end

	// Barry - Use preferred sort order from preferences
	SetSortArrow();
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
}

CQueueListCtrl::~CQueueListCtrl()
{
	if (m_hTimer)
		VERIFY( ::KillTimer(NULL, m_hTimer) );
}

void CQueueListCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CQueueListCtrl::SetAllIcons()
{
	imagelist.DeleteImageList();
	imagelist.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	imagelist.SetBkColor(CLR_NONE);
	//Xman Show correct Icons	
	/*
	imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientCompatible")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
	imagelist.Add(CTempIconLoader(_T("Friend")));
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientShareaza")));
	imagelist.Add(CTempIconLoader(_T("ClientShareazaPlus")));
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));
	imagelist.Add(CTempIconLoader(_T("ClientAMulePlus")));
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));
	imagelist.Add(CTempIconLoader(_T("ClientLPhantPlus")));
	*/
	imagelist.Add(CTempIconLoader(_T("ClientDefault")));		//0
	imagelist.Add(CTempIconLoader(_T("ClientDefaultPlus")));	//1
	imagelist.Add(CTempIconLoader(_T("ClientEDonkey")));		//2
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));	//3
	imagelist.Add(CTempIconLoader(_T("ClientCompatible")));		//4
	imagelist.Add(CTempIconLoader(_T("ClientCompatiblePlus")));	//5
	imagelist.Add(CTempIconLoader(_T("ClientFriend")));			//6
	imagelist.Add(CTempIconLoader(_T("ClientFriendPlus")));		//7
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkey")));		//8
	imagelist.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));	//9
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));	//10
	imagelist.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));//11
	imagelist.Add(CTempIconLoader(_T("ClientShareaza")));		//12
	imagelist.Add(CTempIconLoader(_T("ClientShareazaPlus")));	//13
	imagelist.Add(CTempIconLoader(_T("ClientAMule")));			//14
	imagelist.Add(CTempIconLoader(_T("ClientAMulePlus")));		//15
	imagelist.Add(CTempIconLoader(_T("ClientLPhant")));			//16
	imagelist.Add(CTempIconLoader(_T("ClientLPhantPlus")));		//17
	imagelist.Add(CTempIconLoader(_T("LEECHER")));				//18 //Xman Anti-Leecher

	//Xman friend visualization
	imagelist.Add(CTempIconLoader(_T("ClientFriendSlotOvl"))); //19
	//Xman end


	//Xman end	
	// ==> Mod Icons - Stulle
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	imagelist.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //20
	*/
	imagelist.Add(CTempIconLoader(_T("SCARANGEL"))); //20
	// <== Mephisto mod [Stulle] - Mephisto
	imagelist.Add(CTempIconLoader(_T("STULLE"))); //21
	imagelist.Add(CTempIconLoader(_T("XTREME"))); //22
	imagelist.Add(CTempIconLoader(_T("MORPH"))); //23
	imagelist.Add(CTempIconLoader(_T("EASTSHARE"))); //24
	imagelist.Add(CTempIconLoader(_T("EMF"))); //25
	imagelist.Add(CTempIconLoader(_T("NEO"))); //26
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	imagelist.Add(CTempIconLoader(_T("MEPHISTO"))); //27
	*/
	imagelist.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //27
	// <== Mephisto mod [Stulle] - Mephisto
	imagelist.Add(CTempIconLoader(_T("XRAY"))); //28
	imagelist.Add(CTempIconLoader(_T("MAGIC"))); //29
	// <== Mod Icons - Stulle
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	imagelist.SetOverlayImage(imagelist.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	// ==> Mod Icons - Stulle
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	// <== Mod Icons - Stulle
}

void CQueueListCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;

	if(pHeaderCtrl->GetItemCount() != 0) {
		CString strRes;

		strRes = GetResString(IDS_QL_USERNAME);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(0, &hdi);

		strRes = GetResString(IDS_FILE);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(1, &hdi);

		strRes = GetResString(IDS_FILEPRIO);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(2, &hdi);

		strRes = GetResString(IDS_QL_RATING);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(3, &hdi);

		strRes = GetResString(IDS_SCORE);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(4, &hdi);

		strRes = GetResString(IDS_ASKED);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(5, &hdi);

		strRes = GetResString(IDS_LASTSEEN);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(6, &hdi);

		strRes = GetResString(IDS_ENTERQUEUE);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(7, &hdi);

		strRes = GetResString(IDS_BANNED);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(8, &hdi);
		
		strRes = GetResString(IDS_UPSTATUS);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(9, &hdi);

		//Xman version see clientversion in every window
		strRes = GetResString(IDS_CD_CSOFT);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(10, &hdi);
		//Xman end

		//Xman show complete up/down in queuelist
		strRes = GetResString(IDS_UPDOWNUPLOADLIST);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(11, &hdi);
		//Xman end

		// ==> push small files [sivka] - Stulle
		strRes = GetResString(IDS_SMALL);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(12, &hdi);
		// <== push small files [sivka] - Stulle

		// ==> push rare file - Stulle
		strRes = GetResString(IDS_RARE);
		hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
		pHeaderCtrl->SetItem(13, &hdi);
		// <== push rare file - Stulle

	}
	// ==> Design Settings [eWombat/Stulle] - Stulle
	theApp.emuledlg->transferwnd->SetBackgroundColor(style_b_queuelist);
	// <== Design Settings [eWombat/Stulle] - Stulle
}

void CQueueListCtrl::AddClient(/*const*/ CUpDownClient* client, bool resetclient)
{
	if (resetclient && client){
		// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
		/*
		client->SetWaitStartTime();
		*/
		// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
		client->SetAskedCount(1);
	}

	if (!theApp.emuledlg->IsRunning())
		return;
	if (thePrefs.IsQueueListDisabled())
		return;

	int iItemCount = GetItemCount();
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM,iItemCount,LPSTR_TEXTCALLBACK,0,0,0,(LPARAM)client);
	Update(iItem);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2OnQueue, iItemCount+1);
}

void CQueueListCtrl::RemoveClient(const CUpDownClient* client)
{
	if (!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1){
		DeleteItem(result);
		theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2OnQueue);
	}
}

void CQueueListCtrl::RefreshClient(const CUpDownClient* client)
{
	// There is some type of timing issue here.. If you click on item in the queue or upload and leave
	// the focus on it when you exit the cient, it breaks on line 854 of emuleDlg.cpp.. 
	// I added this IsRunning() check to this function and the DrawItem method and
	// this seems to keep it from crashing. This is not the fix but a patch until
	// someone points out what is going wrong.. Also, it will still assert in debug mode..
	if (!theApp.emuledlg->IsRunning())
		return;

	//MORPH START - SiRoB, Don't Refresh item if not needed
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd || theApp.emuledlg->transferwnd->queuelistctrl.IsWindowVisible() == false )
		return;
	//MORPH END   - SiRoB, Don't Refresh item if not needed

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		Update(result);
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CQueueListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
		odc->SetBkColor(GetBkColor());
	COLORREF crOldBackColor = odc->GetBkColor(); //Xman PowerRelease //Xman show LowIDs

	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	//Xman narrow font at transferwindow
	/*
	CFont* pOldFont = dc.SelectObject(GetFont());
	*//*
	CFont* pOldFont = dc.SelectObject(thePrefs.UseNarrowFont() ? &m_fontNarrow : GetFont());
	//Xman end
	//MORPH - Moved by SiRoB, Don't draw hidden Rect
	/*
	CRect cur_rec(lpDrawItemStruct->rcItem);
	*//*
	//Xman end
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);
	*/
	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	int iClientStyle = client->GetClientStyle(true,true,true,true);
	StylesStruct style;
	thePrefs.GetStyle(client_styles, iClientStyle, &style);
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
		odc->SetBkColor(crTempColor);

	COLORREF crOldBackColor = odc->GetBkColor(); //Xman PowerRelease //Xman show LowIDs
	crTempColor = m_crWindowText;
	if(style.nFontColor != CLR_DEFAULT)
		crTempColor = style.nFontColor;

	CMemDC dc(odc, &lpDrawItemStruct->rcItem);
	CFont* pOldFont = dc.SelectObject(theApp.GetFontByStyle(style.nFlags,thePrefs.UseNarrowFont()));
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : crTempColor);
	// <== Design Settings [eWombat/Stulle] - Stulle

	int iOldBkMode;
	if (m_crWindowTextBk == CLR_NONE){
		DefWindowProc(WM_ERASEBKGND, (WPARAM)(HDC)dc, 0);
		iOldBkMode = dc.SetBkMode(TRANSPARENT);
	}
	else
		iOldBkMode = OPAQUE;

	CKnownFile* file = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
	CHeaderCtrl *pHeaderCtrl = GetHeaderCtrl();
	int iCount = pHeaderCtrl->GetItemCount();
	cur_rec.right = cur_rec.left - 8;
	cur_rec.left += 4;
	CString Sbuffer;
	for(int iCurrent = 0; iCurrent < iCount; iCurrent++){
		int iColumn = pHeaderCtrl->OrderToIndex(iCurrent);
		if( !IsColumnHidden(iColumn) ){
			cur_rec.right += GetColumnWidth(iColumn);
			switch(iColumn){
				case 0:{
				//Xman Show correct Icons
				/*
					uint8 image;
					if (client->IsFriend())
						image = 4;
					else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
						if (client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 8;
						else
							image = 7;
					}
					else if (client->GetClientSoft() == SO_MLDONKEY){
						if (client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 6;
						else
							image = 5;
					}
					else if (client->GetClientSoft() == SO_SHAREAZA){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 10;
						else
							image = 9;
					}
					else if (client->GetClientSoft() == SO_AMULE){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 12;
						else
							image = 11;
					}
					else if (client->GetClientSoft() == SO_LPHANT){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 14;
						else
							image = 13;
					}
					else if (client->ExtProtocolAvailable()){
						if(client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 3;
						else
							image = 1;
					}
					else{
						if (client->credits->GetScoreRatio(client->GetIP()) > 1)
							image = 2;
						else
							image = 0;
					}
				*/
				uint8 image;
				if (client->IsFriend())
					image = 6;
				else if (client->GetClientSoft() == SO_EDONKEYHYBRID){
					image = 10;
				}
				else if (client->GetClientSoft() == SO_EDONKEY){
					image = 2;
				}
				else if (client->GetClientSoft() == SO_MLDONKEY){
					image = 8;
				}
				else if (client->GetClientSoft() == SO_SHAREAZA){
					image = 12;
				}
				else if (client->GetClientSoft() == SO_AMULE){
					image = 14;
				}
				else if (client->GetClientSoft() == SO_LPHANT){
					image = 16;
				}
				else if (client->ExtProtocolAvailable()){
					// ==> Mod Icons - Stulle
					/*
					image = 4;
					*/
					if(client->GetModClient() == MOD_NONE)
					image = 4;
					else
						image = (uint8)(client->GetModClient() + 19);
					// <== Mod Icons - Stulle
				}
				else{
					image = 0;
				}
				//Xman Anti-Leecher
				if(client->IsLeecher()>0)
					image=18;
				else
				//Xman end
				// ==> Mod Icons - Stulle
				// ==> CreditSystems [EastShare/ MorphXT] - Stulle
				/*
				if (((client->credits)?client->credits->GetScoreRatio(client):0) > 1)
					image++;
				*/
				if (client->GetModClient() == MOD_NONE){
					if(client->credits && client->credits->GetHasScore(client))
						image++;
				}
				// <== CreditSystems [EastShare/ MorphXT] - Stulle
				// <== Mod Icons - Stulle
				//Xman end

				uint32 nOverlayImage = 0;
				if ((client->Credits() && client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED))
					nOverlayImage |= 1;
					//Xman changed: display the obfuscation icon for all clients which enabled it
					/*
					if (client->IsObfuscatedConnectionEstablished())
					*/
					if(client->IsObfuscatedConnectionEstablished() 
						|| (!(client->socket != NULL && client->socket->IsConnected())
						&& (client->SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (client->RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))))
					nOverlayImage |= 2;
					int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
					POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
					imagelist.Draw(dc,image, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));

					// ==> Mod Icons - Stulle
					if(client->Credits() && client->credits->GetHasScore(client) && client->GetModClient() != MOD_NONE)
					{
						if(client->Credits()->GetCurrentIdentState(client->GetIP()) == IS_IDENTIFIED)
							m_overlayimages.Draw(dc,1, point, ILD_TRANSPARENT);
						else
							m_overlayimages.Draw(dc,0, point, ILD_TRANSPARENT);
					}
					// <== Mod Icons - Stulle

					Sbuffer = client->GetUserName();

					//Xman friend visualization
					if (client->IsFriend() && client->GetFriendSlot())
						imagelist.Draw(dc,19, point, ILD_NORMAL);
					//Xman end

					//EastShare Start - added by AndCycle, IP to Country, modified by Commander
					if(theApp.ip2country->ShowCountryFlag() ){
						cur_rec.left+=20;
						POINT point2= {cur_rec.left,cur_rec.top+1};
						theApp.ip2country->GetFlagImageList()->Draw(dc, client->GetCountryFlagIndex(), point2, ILD_NORMAL);
					}
					//EastShare End - added by AndCycle, IP to Country

					cur_rec.left +=20;
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
					cur_rec.left -=20;

					//EastShare Start - added by AndCycle, IP to Country
					if(theApp.ip2country->ShowCountryFlag() ){
						cur_rec.left-=20;
					}
					//EastShare End - added by AndCycle, IP to Country

					break;
				}
				case 1:
					if(file)
					{
						Sbuffer = file->GetFileName();
						// ==> Design Settings [eWombat/Stulle] - Stulle
						/*
						//Xman PowerRelease
						if(file->GetUpPriority()==PR_POWER)
							dc.SetBkColor(RGB(255,225,225));
						//Xman end
						*/
						// <== Design Settings [eWombat/Stulle] - Stulle
					}
					else
						Sbuffer = _T("?");
					break;
				case 2:
					if(file){
						switch (file->GetUpPriority()) {
							case PR_VERYLOW : {
								Sbuffer = GetResString(IDS_PRIOVERYLOW);
								break; }
							case PR_LOW : {
								if( file->IsAutoUpPriority() )
									Sbuffer = GetResString(IDS_PRIOAUTOLOW);
								else
									Sbuffer = GetResString(IDS_PRIOLOW);
								break; }
							case PR_NORMAL : {
								if( file->IsAutoUpPriority() )
									Sbuffer = GetResString(IDS_PRIOAUTONORMAL);
								else
									Sbuffer = GetResString(IDS_PRIONORMAL);
								break; }
							case PR_HIGH : {
								if( file->IsAutoUpPriority() )
									Sbuffer = GetResString(IDS_PRIOAUTOHIGH);
								else
									Sbuffer = GetResString(IDS_PRIOHIGH);
								break; }
							case PR_VERYHIGH : {
								Sbuffer = GetResString(IDS_PRIORELEASE);
								break; }
						   //Xman PowerRelease
							case PR_POWER:
								Sbuffer = GetResString(IDS_POWERRELEASE);
								break;
							//Xman end

							default:
								Sbuffer.Empty();
						}
						// ==> PowerShare [ZZ/MorphXT] - Stulle
						if(client->GetPowerShared(file)) {
							CString tempString = GetResString(IDS_POWERSHARE_PREFIX);
							tempString.Append(_T(","));
							tempString.Append(Sbuffer);
							Sbuffer.Empty();
							Sbuffer = tempString;
						}
						// <== PowerShare [ZZ/MorphXT] - Stulle
						// ==> Fair Play [AndCycle/Stulle] - Stulle
						if (file->statistic.GetFairPlay()) {
							Sbuffer.Append(_T(",FairPlay"));
						}
						// <== Fair Play [AndCycle/Stulle] - Stulle
					}
					else
						Sbuffer = _T("?");
					break;
				case 3:
					Sbuffer.Format(_T("%i"),client->GetScore(false,false,true));
					break;
				case 4:
					// ==> Display reason for zero score [Stulle] - Stulle
					/*
					if (client->HasLowID()){
						if (client->m_bAddNextConnect)
							Sbuffer.Format(_T("%i ****"),client->GetScore(false));
						else
							Sbuffer.Format(_T("%i (%s)"),client->GetScore(false), GetResString(IDS_IDLOW));
					}
					//Xman uploading problem client
					else if(client->isupprob && client->m_bAddNextConnect)
					{
						if(client->socket && client->socket->IsConnected())
							Sbuffer.Format(_T("%i #~~"),client->GetScore(false));
						else
							Sbuffer.Format(_T("%i ~~~"),client->GetScore(false));
					}
					//Xman end
					else
						Sbuffer.Format(_T("%i"),client->GetScore(false));
					*/
					{
						uint32 uScore = client->GetScore(false);
						if (client->HasLowID()){
							if (client->m_bAddNextConnect)
								Sbuffer.Format(_T("%i ****"),uScore);
							else
								Sbuffer.Format(_T("%i (%s)"),uScore, GetResString(IDS_IDLOW));
						}
						//Xman uploading problem client
						else if(client->isupprob && client->m_bAddNextConnect)
						{
							if(client->socket && client->socket->IsConnected())
								Sbuffer.Format(_T("%i #~~"),uScore);
							else
								Sbuffer.Format(_T("%i ~~~"),uScore);
						}
						//Xman end
						else
							Sbuffer.Format(_T("%i"),uScore);

						if(uScore == 0)
							Sbuffer.AppendFormat(_T(" (%s)"),client->GetZeroScoreString());

						// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
						if (client->IsPBFClient())
						{
							CString tempStr;
							if (client->IsSecure())
								tempStr.Format(_T("%s %s"), _T("PBF"), Sbuffer);
							else
								tempStr.Format(_T("%s %s"), _T("PBF II"), Sbuffer);
							Sbuffer = tempStr;
						}
						// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
					}
					// <== Display reason for zero score [Stulle] - Stulle
					break;
				case 5:
					Sbuffer.Format(_T("%i"),client->GetAskedCount());
					break;
				case 6:
					Sbuffer = CastSecondsToHM((::GetTickCount() - client->GetLastUpRequest())/1000);
					break;
				case 7:
					Sbuffer = CastSecondsToHM((::GetTickCount() - client->GetWaitStartTime())/1000);
					break;
				case 8:
					//Xman Code Improvement
					/*
					if(client->IsBanned())
					*/
					if(client->GetUploadState()==US_BANNED)
					//Xman end
						Sbuffer = GetResString(IDS_YES);
					else
						Sbuffer = GetResString(IDS_NO);
					break;
				case 9:
					if( client->GetUpPartCount()){
						cur_rec.bottom--;
						cur_rec.top++;
						client->DrawUpStatusBar(dc,&cur_rec,false,thePrefs.UseFlatBar());
						//Xman client percentage (font idea by morph)
						CString buffer;
						// ==> Show Client Percentage optional [Stulle] - Stulle
						/*
						if (thePrefs.GetUseDwlPercentage())
						*/
						if (thePrefs.GetShowClientPercentage())
						// <== Show Client Percentage optional [Stulle] - Stulle
						{
							if(client->GetHisCompletedPartsPercent_UP() >=0)
							{
								COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
								int iOMode = dc.SetBkMode(TRANSPARENT);
								buffer.Format(_T("%i%%"), client->GetHisCompletedPartsPercent_UP());
								CFont *pOldFont = dc.SelectObject(&m_fontBoldSmaller);
#define	DrawClientPercentText	dc.DrawText(buffer, buffer.GetLength(),&cur_rec, ((DLC_DT_TEXT | DT_RIGHT) & ~DT_LEFT) | DT_CENTER)
								cur_rec.top-=1;cur_rec.bottom-=1;
								DrawClientPercentText;cur_rec.left+=1;cur_rec.right+=1;
								DrawClientPercentText;cur_rec.left+=1;cur_rec.right+=1;
								DrawClientPercentText;cur_rec.top+=1;cur_rec.bottom+=1;
								DrawClientPercentText;cur_rec.top+=1;cur_rec.bottom+=1;
								DrawClientPercentText;cur_rec.left-=1;cur_rec.right-=1;
								DrawClientPercentText;cur_rec.left-=1;cur_rec.right-=1;
								DrawClientPercentText;cur_rec.top-=1;cur_rec.bottom-=1;
								DrawClientPercentText;cur_rec.left++;cur_rec.right++;
								dc.SetTextColor(RGB(255,255,255));
								DrawClientPercentText;
								dc.SelectObject(pOldFont);
								dc.SetBkMode(iOMode);
								dc.SetTextColor(oldclr);
							}
						}
						//Xman end
						cur_rec.bottom++;
						cur_rec.top--;
					}
					break;
				//Xman version see clientversion in every window
				case 10:
					Sbuffer.Format(_T("%s"), client->DbgGetFullClientSoftVer()); //Xman // Maella -Support for tag ET_MOD_VERSION 0x55
					// ==> Design Settings [eWombat/Stulle] - Stulle
					/*
					if(client->HasLowID()) dc.SetBkColor(RGB(255,250,200));//Xman show LowIDs
					*/
					// <== Design Settings [eWombat/Stulle] - Stulle
					break;
				//Xman end

				//Xman show complete up/down in queuelist
				case 11:
					if(client->Credits() )
					{
						Sbuffer.Format(_T("%s/ %s"), CastItoXBytes(client->credits->GetUploadedTotal()), CastItoXBytes(client->credits->GetDownloadedTotal()));
					}
					break;
				//Xman end

 				// ==> push small files [sivka] - Stulle
				case 12:
					{
						if (client->GetSmallFilePush())
							Sbuffer.Format(_T("%s"),GetResString(IDS_YES));
						else
							Sbuffer.Format(_T("%s"),GetResString(IDS_NO));
						break;
					}
				// <== push small files [sivka] - Stulle

				// ==> push rare file - Stulle
				case 13:
					{
						Sbuffer.Format(_T("%.1f"), client->GetRareFilePushRatio()) ;
						break;
					}
				// <== push rare file - Stulle
		   	}
			if( iColumn != 9 && iColumn != 0)
				dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
			dc.SetBkColor(crOldBackColor); //Xman PowerRelease //Xman show LowIDs
			cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	// draw rectangle around selected item(s)
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

		if(bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}
	
	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);
	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CQueueListCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);
	//Xman friendhandling
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && !client->IsFriend()) ? MF_ENABLED : MF_GRAYED), MP_ADDFRIEND, GetResString(IDS_ADDFRIEND), _T("ADDFRIEND"));
	//Xman friendhandling
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_REMOVEFRIEND, GetResString(IDS_REMOVEFRIEND), _T("DELETEFRIEND"));
	ClientMenu.AppendMenu(MF_STRING | (client && client->IsFriend() ? MF_ENABLED : MF_GRAYED), MP_FRIENDSLOT, GetResString(IDS_FRIENDSLOT), _T("FRIENDSLOT"));
	ClientMenu.CheckMenuItem(MP_FRIENDSLOT, (client && client->GetFriendSlot()) ? MF_CHECKED : MF_UNCHECKED);
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	//Xman end

	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient()) ? MF_ENABLED : MF_GRAYED), MP_MESSAGE, GetResString(IDS_SEND_MSG), _T("SENDMESSAGE"));
	ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetViewSharedFilesSupport()) ? MF_ENABLED : MF_GRAYED), MP_SHOWLIST, GetResString(IDS_VIEWFILES), _T("VIEWFILES"));
	if (thePrefs.IsExtControlsEnabled())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->IsBanned()) ? MF_ENABLED : MF_GRAYED), MP_UNBAN, GetResString(IDS_UNBAN));
	if (Kademlia::CKademlia::IsRunning() && !Kademlia::CKademlia::IsConnected())
		ClientMenu.AppendMenu(MF_STRING | ((client && client->IsEd2kClient() && client->GetKadPort()!=0) ? MF_ENABLED : MF_GRAYED), MP_BOOT, GetResString(IDS_BOOTSTRAP));
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED), MP_FIND, GetResString(IDS_FIND), _T("Search"));
	// - show requested files (sivka/Xman)
	ClientMenu.AppendMenu(MF_SEPARATOR); 
	ClientMenu.AppendMenu(MF_STRING | (GetItemCount() > 0 ? MF_ENABLED : MF_GRAYED),MP_LIST_REQUESTED_FILES, GetResString(IDS_LISTREQUESTED), _T("FILEREQUESTED")); 
	//Xman end


	GetPopupMenuPos(*this, point);
	ClientMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( ClientMenu.DestroyMenu() ); // XP Style Menu [Xanatos] - Stulle
}

BOOL CQueueListCtrl::OnCommand(WPARAM wParam,LPARAM /*lParam*/)
{
	wParam = LOWORD(wParam);

	switch (wParam)
	{
	case MP_FIND:
		OnFindStart();
		return TRUE;
	}

	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1){
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		switch (wParam){
			case MP_SHOWLIST:
				client->RequestSharedFileList();
				break;
			case MP_MESSAGE:
				theApp.emuledlg->chatwnd->StartSession(client);
				break;
			case MP_ADDFRIEND:
				if (theApp.friendlist->AddFriend(client))
					Update(iSel);
				break;
			//Xman friendhandling
			case MP_REMOVEFRIEND:
				if (client && client->IsFriend())
				{
					theApp.friendlist->RemoveFriend(client->m_Friend);
					Update(iSel);
				}
				break;
			case MP_FRIENDSLOT: 
				if (client)
				{
					bool IsAlready;				
					IsAlready = client->GetFriendSlot();
					// ==> Multiple friendslots [ZZ] - Mephisto
					/*
					theApp.friendlist->RemoveAllFriendSlots();
					*/
					// <== Multiple friendslots [ZZ] - Mephisto
					if( !IsAlready )
						client->SetFriendSlot(true);
					// ==> Multiple friendslots [ZZ] - Mephisto
					else
						client->SetFriendSlot(false);
					// <== Multiple friendslots [ZZ] - Mephisto
					Update(iSel);
				}
				break;
			//Xman end
			case MP_UNBAN:
				if (client->IsBanned()){
					client->UnBan();
					Update(iSel);
				}
				break;
			case MP_DETAIL:
			case MPG_ALTENTER:
			case IDA_ENTER:
			{
				CClientDetailDialog dialog(client, this);
				dialog.DoModal();
				break;
			}
			case MP_BOOT:
				if (client->GetKadPort())
					Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort(), (client->GetKadVersion() > 1));
				break;
			// - show requested files (sivka/Xman)
			case MP_LIST_REQUESTED_FILES: { 
				if (client != NULL)
				{
					client->ShowRequestedFiles(); 
				}
				break;
										  }
			  //Xman end
		}
	}
	return true;
} 

void CQueueListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem()!= pNMListView->iSubItem) ? true : !GetSortAscending();

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:100), 100);
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:100));

	*pResult = 0;
}

int CQueueListCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;
	int iResult=0;
	switch(lParamSort){
		case 0: 
			if(item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if(item1->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;
		case 100:
			if(item2->GetUserName() && item1->GetUserName())
				iResult=CompareLocaleStringNoCase(item2->GetUserName(), item1->GetUserName());
			else if(item2->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;
		
		case 1: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 101: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file2->GetFileName(), file1->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		
		case 2: {
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			// ==> Fair Play [AndCycle/Stulle] - Stulle
			{
				if (!file1->GetPowerShared() && !file1->statistic.GetFairPlay() && (file2->GetPowerShared() || file2->statistic.GetFairPlay()))
					iResult=-1;			
				else if ((file1->GetPowerShared() || file1->statistic.GetFairPlay()) && !file2->GetPowerShared() && !file2->statistic.GetFairPlay())
					iResult=1;
				else
			// <== Fair Play [AndCycle/Stulle] - Stulle
			// <== PowerShare [ZZ/MorphXT] - Stulle
				iResult=((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority()) - ((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority());
			} // PowerShare [ZZ/MorphXT] - Stulle
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 102:{
			CKnownFile* file1 = theApp.sharedfiles->GetFileByID(item1->GetUploadFileID());
			CKnownFile* file2 = theApp.sharedfiles->GetFileByID(item2->GetUploadFileID());
			if( (file1 != NULL) && (file2 != NULL))
			// ==> PowerShare [ZZ/MorphXT] - Stulle
			// ==> Fair Play [AndCycle/Stulle] - Stulle
			{
				if (!file2->GetPowerShared() && !file2->statistic.GetFairPlay() && (file1->GetPowerShared() || file1->statistic.GetFairPlay()))
					iResult=-1;			
				else if ((file2->GetPowerShared() || file2->statistic.GetFairPlay()) && !file1->GetPowerShared() && !file1->statistic.GetFairPlay())
					iResult=1;
				else
			// <== Fair Play [AndCycle/Stulle] - Stulle
			// <== PowerShare [ZZ/MorphXT] - Stulle
				iResult=((file2->GetUpPriority()==PR_VERYLOW) ? -1 : file2->GetUpPriority()) - ((file1->GetUpPriority()==PR_VERYLOW) ? -1 : file1->GetUpPriority());
			} // PowerShare [ZZ/MorphXT] - Stulle
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}

		case 3: 
			iResult=CompareUnsigned(item1->GetScore(false,false,true), item2->GetScore(false,false,true));
			break;
		case 103: 
			iResult=CompareUnsigned(item2->GetScore(false,false,true), item1->GetScore(false,false,true));
			break;

		case 4: 
			// ==> Superior Client Handling [Stulle] - Stulle
			if(!item1->IsSuperiorClient() && item2->IsSuperiorClient())
					iResult=-1;
			else if(item1->IsSuperiorClient() && !item2->IsSuperiorClient())
					iResult=1;
			else
			// <== Superior Client Handling [Stulle] - Stulle
			iResult=CompareUnsigned(item1->GetScore(false), item2->GetScore(false));
			break;
		case 104: 
			// ==> Superior Client Handling [Stulle] - Stulle
			if(!item2->IsSuperiorClient() && item1->IsSuperiorClient())
					iResult=-1;
			else if(item2->IsSuperiorClient() && !item1->IsSuperiorClient())
					iResult=1;
			else
			// <== Superior Client Handling [Stulle] - Stulle
			iResult=CompareUnsigned(item2->GetScore(false), item1->GetScore(false));
			break;

		case 5: 
			iResult=item1->GetAskedCount() - item2->GetAskedCount();
			break;
		case 105: 
			iResult=item2->GetAskedCount() - item1->GetAskedCount();
			break;
		
		case 6: 
			iResult=item1->GetLastUpRequest() - item2->GetLastUpRequest();
			break;
		case 106: 
			iResult=item2->GetLastUpRequest() - item1->GetLastUpRequest();
			break;
		
		case 7: 
			// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
			/*
			iResult=item1->GetWaitStartTime() - item2->GetWaitStartTime();
			*/
			{
				sint64 time1 = item1->GetWaitStartTime();
				sint64 time2 = item2->GetWaitStartTime();
				if ( time1 == time2 ) {
					iResult = 0;
				} else if ( time1 > time2 ) {
					iResult = 1;
				} else {
					iResult = -1;
				}
				break;
			}
			// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
			break;
		case 107: 
			// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
			/*
			iResult=item2->GetWaitStartTime() - item1->GetWaitStartTime();
			*/
			{
				sint64 time1 = item1->GetWaitStartTime();
				sint64 time2 = item2->GetWaitStartTime();
				if ( time1 == time2 ) {
					iResult = 0;
				} else if ( time1 < time2 ) {
					iResult = 1;
				} else {
					iResult = -1;
				}
				break;
			}
			// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
			break;
		
		case 8: 
			iResult=item1->IsBanned() - item2->IsBanned();
			break;
		case 108: 
			iResult=item2->IsBanned() - item1->IsBanned();
			break;
		
		case 9: 
			iResult=item1->GetUpPartCount() - item2->GetUpPartCount();
			break;
		case 109: 
			iResult=item2->GetUpPartCount() - item1->GetUpPartCount();
			break;
		//Xman version see clientversion in every window
		case 10:
			// Maella -Support for tag ET_MOD_VERSION 0x55-
			if( item1->GetClientSoft() == item2->GetClientSoft() )
				if(item2->GetVersion() == item1->GetVersion() && item1->GetClientSoft() == SO_EMULE){
					iResult= item2->DbgGetFullClientSoftVer().CompareNoCase( item1->DbgGetFullClientSoftVer());
				}
				else {
					iResult= item2->GetVersion() - item1->GetVersion();
				}
			else
				iResult= item1->GetClientSoft() - item2->GetClientSoft();
			break;
		case 110:
			if( item1->GetClientSoft() == item2->GetClientSoft() )
				if(item2->GetVersion() == item1->GetVersion() && item2->GetClientSoft() == SO_EMULE){
					iResult= item1->DbgGetFullClientSoftVer().CompareNoCase( item2->DbgGetFullClientSoftVer());
				}
				else {
					iResult= item1->GetVersion() - item2->GetVersion();
				}
			else
				iResult= item2->GetClientSoft() - item1->GetClientSoft();
			break;
		//Xman end

		//Xman show complete up/down in queuelist
		case 11:
			if(item1->Credits() && item2->Credits())
			{
				iResult=CompareUnsigned64(item1->credits->GetUploadedTotal(), item2->credits->GetUploadedTotal());
			}
			else
				iResult=0;
			break;
		case 111:
			if(item1->Credits() && item2->Credits())
			{
				iResult=CompareUnsigned64(item2->credits->GetUploadedTotal(), item1->credits->GetUploadedTotal());
			}
			else
				iResult=0;
			break;
		//Xman end

		// ==> push small files [sivka] - Stulle
		case 12:
			iResult=item1->GetSmallFilePush() - item2->GetSmallFilePush();
			break;
		case 112:
			iResult=item2->GetSmallFilePush() - item1->GetSmallFilePush();
			break;
		// <== push small files [sivka] - Stulle

		// ==> push rare file - Stulle
		case 13:
			iResult=(int)(item1->GetRareFilePushRatio()*100 - item2->GetRareFilePushRatio()*100);
			break;
		case 113: 
			iResult=(int)(item2->GetRareFilePushRatio()*100 - item1->GetRareFilePushRatio()*100);
			break;
		// <== push rare file - Stulle

		default:
			iResult=0;
			break;
	}
	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->queuelistctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/
	// SLUGFILLER: multiSort remove - handled in parent class

	return iResult;

}

//Xman faster Updating of Queuelist
void CQueueListCtrl::UpdateAll()
{
	if(theApp.emuledlg->IsRunning())
	{
		RedrawItems(0,GetItemCount());
		//CWnd::UpdateWindow(); //not needed because of sorting
		// Sort table
		SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100));
	}
}
//Xman end

// Barry - Refresh the queue every 10 secs
void CALLBACK CQueueListCtrl::QueueUpdateTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/)
{
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	//Xman unreachable
	/*
	try
	*/
	//Xman end
	{
		if (   !theApp.emuledlg->IsRunning() // Don't do anything if the app is shutting down - can cause unhandled exceptions
			|| !thePrefs.GetUpdateQueueList()
			|| theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd
			|| !theApp.emuledlg->transferwnd->queuelistctrl.IsWindowVisible() )
			return;

		//Xman faster Updating of Queuelist
		/*
		const CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
		while( update )
		{
			theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(update);
			update = theApp.uploadqueue->GetNextClient(update);
		}
		*/
		if (theApp.emuledlg->transferwnd->queuelistctrl.GetItemCount()>1)
		{

			theApp.emuledlg->transferwnd->queuelistctrl.UpdateAll();
		}
		//Xman end
	}
	//Xman unreachable
	/*
	CATCH_DFLT_EXCEPTIONS(_T("CQueueListCtrl::QueueUpdateTimer"))
	*/
	//Xman end
}

void CQueueListCtrl::ShowQueueClients()
{
	DeleteAllItems(); 
	CUpDownClient* update = theApp.uploadqueue->GetNextClient(NULL);
	while( update )
	{
		AddClient(update, false);
		update = theApp.uploadqueue->GetNextClient(update);
	}
}

void CQueueListCtrl::ShowSelectedUserDetails()
{
	POINT point;
	::GetCursorPos(&point);
	CPoint p = point; 
    ScreenToClient(&p); 
    int it = HitTest(p); 
    if (it == -1)
		return;

	SetItemState(-1, 0, LVIS_SELECTED);
	SetItemState(it, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	SetSelectionMark(it);   // display selection mark correctly!

	CUpDownClient* client = (CUpDownClient*)GetItemData(GetSelectionMark());
	if (client){
		CClientDetailDialog dialog(client, this);
		dialog.DoModal();
	}
}

void CQueueListCtrl::OnNMDblclk(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	if (iSel != -1) {
		CUpDownClient* client = (CUpDownClient*)GetItemData(iSel);
		if (client){
			CClientDetailDialog dialog(client, this);
			dialog.DoModal();
		}
	}
	*pResult = 0;
}

void CQueueListCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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
			const CUpDownClient* pClient = reinterpret_cast<CUpDownClient*>(pDispInfo->item.lParam);
			if (pClient != NULL){
				switch (pDispInfo->item.iSubItem){
					case 0:
						if (pClient->GetUserName() != NULL && pDispInfo->item.cchTextMax > 0){
							_tcsncpy(pDispInfo->item.pszText, pClient->GetUserName(), pDispInfo->item.cchTextMax);
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
