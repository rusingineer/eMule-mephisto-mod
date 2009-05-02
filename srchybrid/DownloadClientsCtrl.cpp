//--- xrmb:downloadclientslist ---

//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

//SLAHAM: ADDED/MODIFIED DownloadClientsCtrl =>

#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "DownloadClientsCtrl.h"
#include "ClientDetailDialog.h"
#include "MemDC.h"
#include "MenuCmds.h"
#include "FriendList.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "UpDownClient.h"
//Xman
/*
#include "UploadQueue.h"
*/
//Xman end
#include "ClientCredits.h"
#include "PartFile.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "SharedFileList.h"
#include "ListenSocket.h" //Xman changed: display the obfuscation icon for all clients which enabled it

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


IMPLEMENT_DYNAMIC(CDownloadClientsCtrl, CMuleListCtrl)

BEGIN_MESSAGE_MAP(CDownloadClientsCtrl, CMuleListCtrl)
	ON_WM_CONTEXTMENU()
	ON_WM_SYSCOLORCHANGE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclkDownloadClientlist)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
END_MESSAGE_MAP()

CDownloadClientsCtrl::CDownloadClientsCtrl()
: CListCtrlItemWalk(this)
{
	SetGeneralPurposeFind(true, false);
}

void CDownloadClientsCtrl::Init()
{
	SetName(_T("DownloadClientsCtrl"));

	CImageList ilDummyImageList; //dummy list for getting the proper height of listview entries
	ilDummyImageList.Create(1, theApp.GetSmallSytemIconSize().cy,theApp.m_iDfltImageListColorFlags|ILC_MASK, 1, 1); 
	SetImageList(&ilDummyImageList, LVSIL_SMALL);
	ASSERT( (GetStyle() & LVS_SHAREIMAGELISTS) == 0 );
	ilDummyImageList.Detach();

	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	InsertColumn(0,	GetResString(IDS_QL_USERNAME), LVCFMT_LEFT, 165);
	InsertColumn(1,	GetResString(IDS_CD_CSOFT), LVCFMT_LEFT, 90); 
	InsertColumn(2,	GetResString(IDS_FILE), LVCFMT_LEFT, 235);
	InsertColumn(3,	GetResString(IDS_DL_SPEED), LVCFMT_LEFT, 65);
	InsertColumn(4, GetResString(IDS_AVAILABLEPARTS), LVCFMT_LEFT, 150);
	InsertColumn(5,	GetResString(IDS_CL_TRANSFDOWN), LVCFMT_LEFT, 115);
	InsertColumn(6,	GetResString(IDS_CL_TRANSFUP), LVCFMT_LEFT, 115);
	InsertColumn(7,	GetResString(IDS_META_SRCTYPE), LVCFMT_LEFT, 60);

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
	SortItems(SortProc, GetSortItem() + (GetSortAscending() ? 0:100)); //Xman Code Improvement
}

CDownloadClientsCtrl::~CDownloadClientsCtrl()
{
}

void CDownloadClientsCtrl::SetAllIcons() 
{
	m_ImageList.DeleteImageList();
	m_ImageList.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_ImageList.SetBkColor(CLR_NONE);
	//Xman Show correct Icons
	/*
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));
	m_ImageList.Add(CTempIconLoader(_T("Friend")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));
	m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));
	m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));
	*/
	m_ImageList.Add(CTempIconLoader(_T("ClientDefault")));		//0
	m_ImageList.Add(CTempIconLoader(_T("ClientDefaultPlus")));	//1
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkey")));		//2
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyPlus")));	//3
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatible")));		//4
	m_ImageList.Add(CTempIconLoader(_T("ClientCompatiblePlus")));	//5
	m_ImageList.Add(CTempIconLoader(_T("ClientFriend")));			//6
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendPlus")));		//7
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkey")));		//8
	m_ImageList.Add(CTempIconLoader(_T("ClientMLDonkeyPlus")));	//9
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybrid")));	//10
	m_ImageList.Add(CTempIconLoader(_T("ClientEDonkeyHybridPlus")));//11
	m_ImageList.Add(CTempIconLoader(_T("ClientShareaza")));		//12
	m_ImageList.Add(CTempIconLoader(_T("ClientShareazaPlus")));	//13
	m_ImageList.Add(CTempIconLoader(_T("ClientAMule")));			//14
	m_ImageList.Add(CTempIconLoader(_T("ClientAMulePlus")));		//15
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhant")));			//16
	m_ImageList.Add(CTempIconLoader(_T("ClientLPhantPlus")));		//17
	m_ImageList.Add(CTempIconLoader(_T("LEECHER")));				//18 //Xman Anti-Leecher
	//Xman friend visualization
	m_ImageList.Add(CTempIconLoader(_T("ClientFriendSlotOvl"))); //19
	//Xman end
	
	//Xman end


	// ==> Mod Icons - Stulle
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	m_ImageList.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //20
	*/
	m_ImageList.Add(CTempIconLoader(_T("SCARANGEL"))); //20
	// <== Mephisto mod [Stulle] - Mephisto
	m_ImageList.Add(CTempIconLoader(_T("STULLE"))); //21
	m_ImageList.Add(CTempIconLoader(_T("XTREME"))); //22
	m_ImageList.Add(CTempIconLoader(_T("MORPH"))); //23
	m_ImageList.Add(CTempIconLoader(_T("EASTSHARE"))); //24
	m_ImageList.Add(CTempIconLoader(_T("EMF"))); //25
	m_ImageList.Add(CTempIconLoader(_T("NEO"))); //26
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	m_ImageList.Add(CTempIconLoader(_T("MEPHISTO"))); //27
	*/
	m_ImageList.Add(CTempIconLoader(_T("AAAEMULEAPP"))); //27
	// <== Mephisto mod [Stulle] - Mephisto
	m_ImageList.Add(CTempIconLoader(_T("XRAY"))); //28
	m_ImageList.Add(CTempIconLoader(_T("MAGIC"))); //29
	// <== Mod Icons - Stulle
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("ClientSecureOvl"))), 1);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlayObfu"))), 2);
	m_ImageList.SetOverlayImage(m_ImageList.Add(CTempIconLoader(_T("OverlaySecureObfu"))), 3);
	// ==> Mod Icons - Stulle
	m_overlayimages.DeleteImageList ();
	m_overlayimages.Create(16,16,theApp.m_iDfltImageListColorFlags|ILC_MASK,0,1);
	m_overlayimages.SetBkColor(CLR_NONE);
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditOvl")));
	m_overlayimages.Add(CTempIconLoader(_T("ClientCreditSecureOvl")));
	// <== Mod Icons - Stulle
}

void CDownloadClientsCtrl::Localize()
{
	CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
	HDITEM hdi;
	hdi.mask = HDI_TEXT;
	CString strRes;

	strRes = GetResString(IDS_QL_USERNAME);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(0, &hdi);

	strRes = GetResString(IDS_CD_CSOFT);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(1, &hdi);

	strRes = GetResString(IDS_FILE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(2, &hdi);

	strRes = GetResString(IDS_DL_SPEED);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(3, &hdi);

	strRes = GetResString(IDS_AVAILABLEPARTS);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(4, &hdi);

	strRes = GetResString(IDS_CL_TRANSFDOWN);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(5, &hdi);

	strRes = GetResString(IDS_CL_TRANSFUP);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(6, &hdi);

	strRes = GetResString(IDS_META_SRCTYPE);
	hdi.pszText = const_cast<LPTSTR>((LPCTSTR)strRes);
	pHeaderCtrl->SetItem(7, &hdi);

	// ==> Design Settings [eWombat/Stulle] - Stulle
	theApp.emuledlg->transferwnd->SetBackgroundColor(style_b_dlclientlist);
	// <== Design Settings [eWombat/Stulle] - Stulle
}

void CDownloadClientsCtrl::AddClient(CUpDownClient* client)
{
	if(!theApp.emuledlg->IsRunning())
		return;
       
	//Xman Code Improvement
	/*
	InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), client->GetUserName(), 0, 0, 1, (LPARAM)client);
	RefreshClient(client);
	*/
	int iItem = InsertItem(LVIF_TEXT|LVIF_PARAM, GetItemCount(), client->GetUserName(), 0, 0, 1, (LPARAM)client);
	Update(iItem);
	//Xman end
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount()); 
}

void CDownloadClientsCtrl::RemoveClient(CUpDownClient* client)
{
	if(!theApp.emuledlg->IsRunning())
		return;

	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if (result != -1)
		DeleteItem(result);
	theApp.emuledlg->transferwnd->UpdateListCount(CTransferWnd::wnd2Downloading, GetItemCount()); 
}

void CDownloadClientsCtrl::RefreshClient(CUpDownClient* client)
{
	if( !theApp.emuledlg->IsRunning() )
		return;
	
	//MORPH START - SiRoB, Don't Refresh item if not needed 
	if( theApp.emuledlg->activewnd != theApp.emuledlg->transferwnd  || theApp.emuledlg->transferwnd->downloadclientsctrl.IsWindowVisible() == false ) 
		return; 
	//MORPH END   - SiRoB, Don't Refresh item if not needed 
	
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)client;
	int result = FindItem(&find);
	if(result != -1)
		Update(result);
	return;
}

#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)

void CDownloadClientsCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
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
	COLORREF crOldBackColor = odc->GetBkColor();  //Xman show LowIDs
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
	//MORPH - Moved by SiRoB, Don't draw hidden Rect
	COLORREF crOldTextColor = dc.SetTextColor((lpDrawItemStruct->itemState & ODS_SELECTED) ? m_crHighlightText : m_crWindowText);
	*/
	const CUpDownClient* client = (CUpDownClient*)lpDrawItemStruct->itemData;
	int iClientStyle = client->GetClientStyle(false,true,false,true);
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

	COLORREF crOldBackColor = odc->GetBkColor();  //Xman show LowIDs
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
					if (client->credits != NULL){
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
					}
					else
						image = 0;
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
					/*
					if (((client->credits)?client->credits->GetMyScoreRatio(client->GetIP()):0) > 1)
						image++;
					*/
					if (client->GetModClient() == MOD_NONE){
						if (((client->credits)?client->credits->GetMyScoreRatio(client->GetIP()):0) > 1)
							image++;
					}
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
					//Xman end
						nOverlayImage |= 2;
					int iIconPosY = (cur_rec.Height() > 16) ? ((cur_rec.Height() - 16) / 2) : 1;
					POINT point = {cur_rec.left, cur_rec.top + iIconPosY};
					m_ImageList.Draw(dc,image, point, ILD_NORMAL | INDEXTOOVERLAYMASK(nOverlayImage));
					
					// ==> Mod Icons - Stulle
					if(client->Credits() && client->Credits()->GetMyScoreRatio(client->GetIP()) > 1)
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
						m_ImageList.Draw(dc,19, point, ILD_NORMAL);
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
					//Xman // Maella -Support for tag ET_MOD_VERSION 0x55
					/*
					Sbuffer.Format(_T("%s"), client->GetClientSoftVer());
					*/
					Sbuffer.Format(_T("%s"), client->DbgGetFullClientSoftVer());
					//Xman end
					// ==> Design Settings [eWombat/Stulle] - Stulle
					/*
					if(client->HasLowID()) dc.SetBkColor(RGB(255,250,200)); //Xman show LowIDs
					*/
					// <== Design Settings [eWombat/Stulle] - Stulle
					break;
				case 2:
					Sbuffer.Format(_T("%s"), client->GetRequestFile()->GetFileName());
					break;
				case 3:
					Sbuffer=CastItoXBytes( (float)client->GetDownloadDatarate() , false, true);
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec, DLC_DT_TEXT | DT_RIGHT);
					break;
				case 4:
				{ //Xman
					cur_rec.bottom--;
					cur_rec.top++;
					client->DrawStatusBar(dc, &cur_rec, false, thePrefs.UseFlatBar());
					//Xman client percentage (font idea by morph)
					CString buffer;
					// ==> Show Client Percentage optional [Stulle] - Stulle
					/*
					if (thePrefs.GetUseDwlPercentage())
					*/
					if (thePrefs.GetShowClientPercentage())
					// <== Show Client Percentage optional [Stulle] - Stulle
					{
						if(client->GetHisCompletedPartsPercent_Down() >=0)
						{
							COLORREF oldclr = dc.SetTextColor(RGB(0,0,0));
							int iOMode = dc.SetBkMode(TRANSPARENT);
							buffer.Format(_T("%i%%"), client->GetHisCompletedPartsPercent_Down());
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
					break;
				} //Xman
				case 5:
					if(client->Credits() && client->GetSessionDown() < client->credits->GetDownloadedTotal())
						Sbuffer.Format(_T("%s (%s)"), CastItoXBytes(client->GetSessionDown()), CastItoXBytes(client->credits->GetDownloadedTotal()));
					else
						Sbuffer.Format(_T("%s"), CastItoXBytes(client->GetSessionDown()));
					break;
				case 6:
					if(client->Credits() && client->GetSessionUp() < client->credits->GetUploadedTotal())
						Sbuffer.Format(_T("%s (%s)"), CastItoXBytes(client->GetSessionUp()), CastItoXBytes(client->credits->GetUploadedTotal()));
					else
						Sbuffer.Format(_T("%s"), CastItoXBytes(client->GetSessionUp()));
					break;
				case 7:
					switch(client->GetSourceFrom()){
					case SF_SERVER:
						Sbuffer = _T("eD2K Server");
						break;
					case SF_KADEMLIA:
						Sbuffer = GetResString(IDS_KADEMLIA);
						break;
					case SF_SOURCE_EXCHANGE:
						Sbuffer = GetResString(IDS_SE);
						break;
					case SF_PASSIVE:
						Sbuffer = GetResString(IDS_PASSIVE);
						break;
					case SF_LINK:
						Sbuffer = GetResString(IDS_SW_LINK);
						break;
					//Xman SLS
					case SF_SLS:
						Sbuffer = _T("SLS");
						break;
					//Xman end

					default:
						Sbuffer = GetResString(IDS_UNKNOWN);
						break;
					}
					break;
				}
				if( iColumn != 4 && iColumn != 0 && iColumn != 3 && iColumn != 11)
					dc.DrawText(Sbuffer,Sbuffer.GetLength(),&cur_rec,DLC_DT_TEXT);
				dc.SetBkColor(crOldBackColor); //Xman show LowIDs
				cur_rec.left += GetColumnWidth(iColumn);
		}
	}

	//draw rectangle around selected item(s)
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

		if (bCtrlFocused)
			dc.FrameRect(&outline_rec, &CBrush(m_crFocusLine));
		else
			dc.FrameRect(&outline_rec, &CBrush(m_crNoFocusLine));
	}

	if (m_crWindowTextBk == CLR_NONE)
		dc.SetBkMode(iOldBkMode);

	dc.SelectObject(pOldFont);
	dc.SetTextColor(crOldTextColor);
}

void CDownloadClientsCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult){

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// Barry - Store sort order in preferences
	// Determine ascending based on whether already sorted on this column
	bool sortAscending = (GetSortItem()!= pNMListView->iSubItem) ? (pNMListView->iSubItem == 0) : !GetSortAscending();	

	// Sort table
	UpdateSortHistory(pNMListView->iSubItem + (sortAscending ? 0:100), 100);
	SetSortArrow(pNMListView->iSubItem, sortAscending);
	SortItems(SortProc, pNMListView->iSubItem + (sortAscending ? 0:100));

	*pResult = 0;
}

BOOL CDownloadClientsCtrl::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
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

			//Xman Xtreme Downloadmanager
			case MP_STOP_CLIENT: 
				StopSingleClient(client);
				break;		
			//Xman end
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

int CDownloadClientsCtrl::SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort){
	const CUpDownClient* item1 = (CUpDownClient*)lParam1;
	const CUpDownClient* item2 = (CUpDownClient*)lParam2;
	int iResult=0;
	switch(lParamSort){
		case 0: 
		case 100:
			if(item1->GetUserName() && item2->GetUserName())
				iResult=CompareLocaleStringNoCase(item1->GetUserName(), item2->GetUserName());
			else if(item1->GetUserName())
				iResult=1;
			else
				iResult=-1;
			break;

		//Xman
		// Maella -Support for tag ET_MOD_VERSION 0x55-
		case 1:
		case 101:
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
		//Xman end
		case 2: 
		case 102:
		{
			CKnownFile* file1 = item1->GetRequestFile();
			CKnownFile* file2 = item2->GetRequestFile();
			if( (file1 != NULL) && (file2 != NULL))
				iResult=CompareLocaleStringNoCase(file1->GetFileName(), file2->GetFileName());
			else if( file1 == NULL )
				iResult=1;
			else
				iResult=-1;
			break;
		}
		case 3:
		case 103:
			iResult=CompareUnsigned(item1->GetDownloadDatarate(), item2->GetDownloadDatarate());
			break;
		case 4:
		case 104: 
			iResult=CompareUnsigned(item1->GetPartCount(), item2->GetPartCount());
			break;
		case 5:
		case 105:
			iResult=CompareUnsigned(item1->GetSessionDown(), item2->GetSessionDown());
			break;
		case 6:
		case 106:
			iResult=CompareUnsigned(item1->GetSessionUp(), item2->GetSessionUp());
			break;
		case 7: 
		case 107: 
			iResult=CompareUnsigned(item1->GetSourceFrom(), item2->GetSourceFrom());
			break;
		default:
			iResult=0;
			break;
	}

	if (lParamSort>=100)
		iResult*=-1;

	// SLUGFILLER: multiSort remove - handled in parent class
	/*
	int dwNextSort;
	//call secondary sortorder, if this one results in equal
	//(Note: yes I know this call is evil OO wise, but better than changing a lot more code, while we have only one instance anyway - might be fixed later)
	if (iResult == 0 && (dwNextSort = theApp.emuledlg->transferwnd->downloadclientsctrl.GetNextSortOrder(lParamSort)) != (-1)){
		iResult= SortProc(lParam1, lParam2, dwNextSort);
	}
	*/

	return iResult;
}

void CDownloadClientsCtrl::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	int iSel = GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED);
	const CUpDownClient* client = (iSel != -1) ? (CUpDownClient*)GetItemData(iSel) : NULL;

	CTitleMenu ClientMenu;
	ClientMenu.CreatePopupMenu();
	ClientMenu.AddMenuTitle(GetResString(IDS_CLIENTS), true);
	ClientMenu.AppendMenu(MF_STRING | (client ? MF_ENABLED : MF_GRAYED), MP_DETAIL, GetResString(IDS_SHOWDETAILS), _T("CLIENTDETAILS"));
	ClientMenu.SetDefaultItem(MP_DETAIL);

	//Xman Xtreme Downloadmanager
	if (client && client->GetDownloadState() == DS_DOWNLOADING)
		ClientMenu.AppendMenu(MF_STRING,MP_STOP_CLIENT,GetResString(IDS_STOP_CLIENT), _T("EXIT"));
	//xman end

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

void CDownloadClientsCtrl::ShowSelectedUserDetails(){
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

void CDownloadClientsCtrl::OnNMDblclkDownloadClientlist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDownloadClientsCtrl::OnSysColorChange()
{
	CMuleListCtrl::OnSysColorChange();
	SetAllIcons();
}

void CDownloadClientsCtrl::OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult)
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

//Xman Xtreme Downloadmanager
void CDownloadClientsCtrl::StopSingleClient(CUpDownClient* single)  
{
	if (single != NULL && single->GetDownloadState() == DS_DOWNLOADING) {
		if(single->socket!=NULL)
		{
			single->SendCancelTransfer();
		}
		single->SetDownloadState(DS_ONQUEUE,_T("download aborted by user"), CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
	}
}
//Xman end
