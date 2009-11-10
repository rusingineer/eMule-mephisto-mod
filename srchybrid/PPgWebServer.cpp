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
#include "PPgWebServer.h"
#include "otherfunctions.h"
#include "WebServer.h"
#include "MMServer.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "ServerWnd.h"
#include "HelpIDs.h"
// ==> UPnP support [MoNKi] - leuk_he
/*
#include ".\ppgwebserver.h"
#include "UPnPImplWrapper.h"
#include "UPnPImpl.h"
*/
// <== UPnP support [MoNKi] - leuk_he
// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
#include "PreferencesDlg.h"
#include "MD5Sum.h"
// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define HIDDEN_PASSWORD _T("*****")


IMPLEMENT_DYNAMIC(CPPgWebServer, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgWebServer, CPropertyPage)
	ON_EN_CHANGE(IDC_WSPASS, OnDataChange)
	ON_EN_CHANGE(IDC_WSPASSLOW, OnDataChange)
	ON_EN_CHANGE(IDC_WSPORT, OnDataChange)
	ON_EN_CHANGE(IDC_MMPASSWORDFIELD, OnDataChange)
	ON_EN_CHANGE(IDC_TMPLPATH, OnDataChange)
	ON_EN_CHANGE(IDC_MMPORT_FIELD, OnDataChange)
	ON_EN_CHANGE(IDC_WSTIMEOUT, OnDataChange)
	ON_BN_CLICKED(IDC_WSENABLED, OnEnChangeWSEnabled)
	ON_BN_CLICKED(IDC_WSENABLEDLOW, OnEnChangeWSEnabled)
	ON_BN_CLICKED(IDC_MMENABLED, OnEnChangeMMEnabled)
	ON_BN_CLICKED(IDC_WSRELOADTMPL, OnReloadTemplates)
	ON_BN_CLICKED(IDC_TMPLBROWSE, OnBnClickedTmplbrowse)
	ON_BN_CLICKED(IDC_WS_GZIP, OnDataChange)
	ON_BN_CLICKED(IDC_WS_ALLOWHILEVFUNC, OnDataChange)
	ON_BN_CLICKED(IDC_WSUPNP, OnDataChange)
	ON_WM_HELPINFO()
	ON_WM_DESTROY()
	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_WEBSERVER, OnTcnSelchangeTab)
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle
	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
	ON_CBN_SELCHANGE(IDC_ACCOUNTSELECT, UpdateSelection)
	ON_BN_CLICKED(IDC_ADVADMINENABLED,   OnEnableChange)
	ON_BN_CLICKED(IDC_ADVADMIN_KAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_TRANSFER, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SEARCH, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SERVER, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SHARED, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_STATS, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_PREFS, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_ADVADMIN_USERLEVEL, OnSettingsChange)	
	ON_BN_CLICKED(IDC_ADVADMIN_DELETE, OnBnClickedDel)
	ON_BN_CLICKED(IDC_ADVADMIN_NEW, OnBnClickedNew)	
	ON_EN_CHANGE(IDC_ADVADMIN_PASS, OnSettingsChange)
	ON_EN_CHANGE(IDC_ADVADMIN_CATS, OnSettingsChange)
//	ON_EN_CHANGE(IDC_ADVADMIN_USER, OnSettingsChange)
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
END_MESSAGE_MAP()

CPPgWebServer::CPPgWebServer()
	: CPropertyPage(CPPgWebServer::IDD)
{
	bCreated = false;
	m_icoBrowse = NULL;
	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	m_imageList.DeleteImageList();
	m_imageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 14+1, 0);
	m_imageList.Add(CTempIconLoader(_T("WEB")));
	// <== maah
}

CPPgWebServer::~CPPgWebServer()
{
}

void CPPgWebServer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	DDX_Control(pDX, IDC_TAB_WEBSERVER , m_tabCtr);
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle
	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
	DDX_Control(pDX, IDC_ACCOUNTSELECT, m_cbAccountSelector); 
	DDX_Control(pDX, IDC_ADVADMIN_USERLEVEL, m_cbUserlevel); 
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
}

BOOL CPPgWebServer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	AddBuddyButton(GetDlgItem(IDC_TMPLPATH)->m_hWnd, ::GetDlgItem(m_hWnd, IDC_TMPLBROWSE));
	InitAttachedBrowseButton(::GetDlgItem(m_hWnd, IDC_TMPLBROWSE), m_icoBrowse);

	((CEdit*)GetDlgItem(IDC_WSPASS))->SetLimitText(12);
	((CEdit*)GetDlgItem(IDC_WSPORT))->SetLimitText(6);

	// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
	InitTab(true);
	m_currentTab = NONE;
	m_tabCtr.SetCurSel(NONE);
	SetTab(WEBSERVER);
	// <== Tabbed WebInterface settings panel [Stulle] - Stulle
	// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
	FillComboBox();
	FillUserlevelBox();
	m_cbAccountSelector.SetCurSel(0); // new account
	// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle

	LoadSettings();
	Localize();

	OnEnChangeWSEnabled();

	// note: there are better classes to create a pure hyperlink, however since it is only needed here
	//		 we rather use an already existing class
	CRect rect;
	GetDlgItem(IDC_GUIDELINK)->GetWindowRect(rect);
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
	m_wndMobileLink.CreateEx(NULL,0,_T("MsgWnd"),WS_BORDER | WS_VISIBLE | WS_CHILD | HTC_WORDWRAP | HTC_UNDERLINE_HOVER,rect.left,rect.top,rect.Width(),rect.Height(),m_hWnd,0);
	m_wndMobileLink.SetBkColor(::GetSysColor(COLOR_3DFACE)); // still not the right color, will fix this later (need to merge the .rc file before it changes ;) )
	m_wndMobileLink.SetFont(GetFont());
	if (!bCreated){
		bCreated = true;
		m_wndMobileLink.AppendText(_T("Link: "));
		m_wndMobileLink.AppendHyperLink(GetResString(IDS_MMGUIDELINK),0,CString(_T("http://mobil.emule-project.net")),0,0);
	}
	return TRUE;
}

void CPPgWebServer::LoadSettings(void)
{
	CString strBuffer;

	GetDlgItem(IDC_WSPASS)->SetWindowText(HIDDEN_PASSWORD);
	GetDlgItem(IDC_WSPASSLOW)->SetWindowText(HIDDEN_PASSWORD);
	GetDlgItem(IDC_MMPASSWORDFIELD)->SetWindowText(HIDDEN_PASSWORD);

	strBuffer.Format(_T("%d"), thePrefs.GetWSPort());
	GetDlgItem(IDC_WSPORT)->SetWindowText(strBuffer);

	strBuffer.Format(_T("%d"), thePrefs.GetMMPort());
	GetDlgItem(IDC_MMPORT_FIELD)->SetWindowText(strBuffer);

	GetDlgItem(IDC_TMPLPATH)->SetWindowText(thePrefs.GetTemplate());

	strBuffer.Format(_T("%d"), thePrefs.GetWebTimeoutMins());
	SetDlgItemText(IDC_WSTIMEOUT,strBuffer);

	if(thePrefs.GetWSIsEnabled())
		CheckDlgButton(IDC_WSENABLED,1);
	else
		CheckDlgButton(IDC_WSENABLED,0);

	if(thePrefs.GetWSIsLowUserEnabled())
		CheckDlgButton(IDC_WSENABLEDLOW,1);
	else
		CheckDlgButton(IDC_WSENABLEDLOW,0);

	if(thePrefs.IsMMServerEnabled())
		CheckDlgButton(IDC_MMENABLED,1);
	else
		CheckDlgButton(IDC_MMENABLED,0);

	CheckDlgButton(IDC_WS_GZIP,(thePrefs.GetWebUseGzip())? 1 : 0);
	CheckDlgButton(IDC_WS_ALLOWHILEVFUNC,(thePrefs.GetWebAdminAllowedHiLevFunc())? 1 : 0);

	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && thePrefs.GetWSIsEnabled());
	CheckDlgButton(IDC_WSUPNP, (thePrefs.IsUPnPEnabled() && thePrefs.m_bWebUseUPnP) ? TRUE : FALSE);
	
	CheckDlgButton(IDC_ADVADMINENABLED, thePrefs.UseIonixWebsrv()); // Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle

	OnEnChangeMMEnabled();

	SetModified(FALSE);	// FoRcHa
}

BOOL CPPgWebServer::OnApply()
{	
	if(m_bModified)
	{
		CString sBuf;

		// get and check templatefile existance...
		GetDlgItem(IDC_TMPLPATH)->GetWindowText(sBuf);
		if ( IsDlgButtonChecked(IDC_WSENABLED) && !PathFileExists(sBuf)) {
			CString buffer;
			buffer.Format(GetResString(IDS_WEB_ERR_CANTLOAD),sBuf);
			AfxMessageBox(buffer,MB_OK);
			return FALSE;
		}
		thePrefs.SetTemplate(sBuf);
		theApp.webserver->ReloadTemplates();


		uint16 oldPort=thePrefs.GetWSPort();

		GetDlgItem(IDC_WSPASS)->GetWindowText(sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetWSPass(sBuf);
		
		GetDlgItem(IDC_WSPASSLOW)->GetWindowText(sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetWSLowPass(sBuf);

		GetDlgItem(IDC_WSPORT)->GetWindowText(sBuf);
		if (_tstoi(sBuf)!=oldPort) {
			thePrefs.SetWSPort((uint16)_tstoi(sBuf));
			theApp.webserver->RestartServer();
		}

		GetDlgItemText(IDC_WSTIMEOUT,sBuf);
		thePrefs.m_iWebTimeoutMins=_tstoi(sBuf);

		thePrefs.SetWSIsEnabled(IsDlgButtonChecked(IDC_WSENABLED)!=0);
		thePrefs.SetWSIsLowUserEnabled(IsDlgButtonChecked(IDC_WSENABLEDLOW)!=0);
		thePrefs.SetWebUseGzip(IsDlgButtonChecked(IDC_WS_GZIP)!=0);
		theApp.webserver->StartServer();
		thePrefs.m_bAllowAdminHiLevFunc= (IsDlgButtonChecked(IDC_WS_ALLOWHILEVFUNC)!=0);

		// mobilemule
		GetDlgItem(IDC_MMPORT_FIELD)->GetWindowText(sBuf);
		if (_tstoi(sBuf)!= thePrefs.GetMMPort() ) {
			thePrefs.SetMMPort((uint16)_tstoi(sBuf));
			theApp.mmserver->StopServer();
			theApp.mmserver->Init();
		}
		thePrefs.SetMMIsEnabled(IsDlgButtonChecked(IDC_MMENABLED)!=0);
		if (IsDlgButtonChecked(IDC_MMENABLED))
			theApp.mmserver->Init();
		else
			theApp.mmserver->StopServer();
		GetDlgItem(IDC_MMPASSWORDFIELD)->GetWindowText(sBuf);
		if(sBuf != HIDDEN_PASSWORD)
			thePrefs.SetMMPass(sBuf);
		
		// ==> UPnP support [MoNKi] - leuk_he
		/*
		if (IsDlgButtonChecked(IDC_WSUPNP))
		{
			ASSERT( thePrefs.IsUPnPEnabled() );
			if (!thePrefs.m_bWebUseUPnP && thePrefs.GetWSIsEnabled() && theApp.m_pUPnPFinder != NULL) // add the port to existing mapping without having eMule restarting (if all conditions are met)
				theApp.m_pUPnPFinder->GetImplementation()->LateEnableWebServerPort(thePrefs.GetWSPort());
			thePrefs.m_bWebUseUPnP = true;
		}
		else
			thePrefs.m_bWebUseUPnP = false;
		*/
		if ((UINT)thePrefs.GetUPnPNatWeb() != IsDlgButtonChecked(IDC_WSUPNP))
		{
			theApp.m_UPnP_IGDControlPoint->SetUPnPNat(thePrefs.IsUPnPEnabled()); // and start/stop nat. 
			thePrefs.SetUPnPNatWeb(IsDlgButtonChecked(IDC_WSUPNP)!=0);
		}
		// <== UPnP support [MoNKi] - leuk_he

		theApp.webserver->SaveWebServConf(); // Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle

		theApp.emuledlg->serverwnd->UpdateMyInfo();
		SetModified(FALSE);
		SetTmplButtonState();
	}

	return CPropertyPage::OnApply();
}

void CPPgWebServer::Localize(void)
{
	if(m_hWnd){
		SetWindowText(GetResString(IDS_PW_WS));
		GetDlgItem(IDC_WSPASS_LBL)->SetWindowText(GetResString(IDS_WS_PASS));
		GetDlgItem(IDC_WSPORT_LBL)->SetWindowText(GetResString(IDS_PORT));
		GetDlgItem(IDC_WSENABLED)->SetWindowText(GetResString(IDS_ENABLED));
		GetDlgItem(IDC_WSRELOADTMPL)->SetWindowText(GetResString(IDS_SF_RELOAD));
		GetDlgItem(IDC_WSENABLED)->SetWindowText(GetResString(IDS_ENABLED));
		SetDlgItemText(IDC_WS_GZIP,GetResString(IDS_WEB_GZIP_COMPRESSION));
		SetDlgItemText(IDC_WSUPNP, GetResString(IDS_WEBUPNPINCLUDE));

		GetDlgItem(IDC_WSPASS_LBL2)->SetWindowText(GetResString(IDS_WS_PASS));
		GetDlgItem(IDC_WSENABLEDLOW)->SetWindowText(GetResString(IDS_ENABLED));
		GetDlgItem(IDC_STATIC_GENERAL)->SetWindowText(GetResString(IDS_PW_GENERAL));

		GetDlgItem(IDC_STATIC_ADMIN)->SetWindowText(GetResString(IDS_ADMIN));
		GetDlgItem(IDC_STATIC_LOWUSER)->SetWindowText(GetResString(IDS_WEB_LOWUSER));
		GetDlgItem(IDC_WSENABLEDLOW)->SetWindowText(GetResString(IDS_ENABLED));

		GetDlgItem(IDC_TEMPLATE)->SetWindowText(GetResString(IDS_WS_RELOAD_TMPL));
		SetDlgItemText(IDC_WSTIMEOUTLABEL,GetResString(IDS_WEB_SESSIONTIMEOUT)+_T(":"));
		SetDlgItemText(IDC_MINS,GetResString(IDS_LONGMINS) );

		GetDlgItem(IDC_MMENABLED)->SetWindowText(GetResString(IDS_ENABLEMM));
		GetDlgItem(IDC_STATIC_MOBILEMULE)->SetWindowText(GetResString(IDS_MOBILEMULE));
		GetDlgItem(IDC_MMPASSWORD)->SetWindowText(GetResString(IDS_WS_PASS));
		GetDlgItem(IDC_MMPORT_LBL)->SetWindowText(GetResString(IDS_PORT));

		GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->SetWindowText(GetResString(IDS_WEB_ALLOWHILEVFUNC));

		// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
		GetDlgItem(IDC_ADVADMINENABLED)->SetWindowText(GetResString(IDS_ADVADMINENABLED));
		GetDlgItem(IDC_ADVADMIN_NOTE)->SetWindowText(GetResString(IDS_ADVADMIN_NOTE));
		GetDlgItem(IDC_STATIC_ADVADMIN)->SetWindowText(GetResString(IDS_ADVADMIN_GROUP));
		GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->SetWindowText(GetResString(IDS_ADVADMIN_ACC));
		GetDlgItem(IDC_ADVADMIN_DELETE)->SetWindowText(GetResString(IDS_ADVADMIN_DELETE));
		GetDlgItem(IDC_ADVADMIN_NEW)->SetWindowText(GetResString(IDS_ADVADMIN_NEW));
		GetDlgItem(IDC_ADVADMIN_KAD)->SetWindowText(GetResString(IDS_ADVADMIN_KAD));
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->SetWindowText(GetResString(IDS_ADVADMIN_TRANSFER));
		GetDlgItem(IDC_ADVADMIN_SEARCH)->SetWindowText(GetResString(IDS_ADVADMIN_SEARCH));
		GetDlgItem(IDC_ADVADMIN_SERVER)->SetWindowText(GetResString(IDS_ADVADMIN_SERVER));
		GetDlgItem(IDC_ADVADMIN_SHARED)->SetWindowText(GetResString(IDS_ADVADMIN_SHARED));
		GetDlgItem(IDC_ADVADMIN_STATS)->SetWindowText(GetResString(IDS_ADVADMIN_STATS));
		GetDlgItem(IDC_ADVADMIN_PREFS)->SetWindowText(GetResString(IDS_ADVADMIN_PREFS));
		GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->SetWindowText(GetResString(IDS_ADVADMIN_USERLEVEL));
		GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->SetWindowText(GetResString(IDS_ADVADMIN_PASS));
		GetDlgItem(IDC_STATIC_ADVADMIN_USER)->SetWindowText(GetResString(IDS_ADVADMIN_USER));
		GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->SetWindowText(GetResString(IDS_ADVADMIN_CAT));
		// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
	}
}

void CPPgWebServer::OnEnChangeWSEnabled()
{
	UINT bIsWIEnabled=IsDlgButtonChecked(IDC_WSENABLED);
	GetDlgItem(IDC_WSPASS)->EnableWindow(bIsWIEnabled);	
	GetDlgItem(IDC_WSPORT)->EnableWindow(bIsWIEnabled);	
	GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_TMPLPATH)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_TMPLBROWSE)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WS_GZIP)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WSTIMEOUT)->EnableWindow(bIsWIEnabled);
	GetDlgItem(IDC_WSPASSLOW)->EnableWindow(bIsWIEnabled && IsDlgButtonChecked(IDC_WSENABLEDLOW));
	GetDlgItem(IDC_WSUPNP)->EnableWindow(thePrefs.IsUPnPEnabled() && bIsWIEnabled);
	
	//GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow(bIsWIEnabled);
	SetTmplButtonState();


	SetModified();
}

void CPPgWebServer::OnEnChangeMMEnabled()
{
	GetDlgItem(IDC_MMPASSWORDFIELD)->EnableWindow(IsDlgButtonChecked(IDC_MMENABLED));	
	GetDlgItem(IDC_MMPORT_FIELD)->EnableWindow(IsDlgButtonChecked(IDC_MMENABLED));

	SetModified();
}

void CPPgWebServer::OnReloadTemplates()
{
	theApp.webserver->ReloadTemplates();
}

void CPPgWebServer::OnBnClickedTmplbrowse()
{
	CString strTempl;
	GetDlgItemText(IDC_TMPLPATH, strTempl);
	CString buffer;
	buffer=GetResString(IDS_WS_RELOAD_TMPL)+_T("(*.tmpl)|*.tmpl||");
    if (DialogBrowseFile(buffer, _T("Template ")+buffer, strTempl)){
		GetDlgItem(IDC_TMPLPATH)->SetWindowText(buffer);
		SetModified();
	}
	SetTmplButtonState();
}

void CPPgWebServer::SetTmplButtonState(){
	CString buffer;
	GetDlgItemText(IDC_TMPLPATH,buffer);

	GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow( thePrefs.GetWSIsEnabled() && (buffer.CompareNoCase(thePrefs.GetTemplate())==0));
}

void CPPgWebServer::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_WebInterface);
}

BOOL CPPgWebServer::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgWebServer::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CPPgWebServer::OnDestroy()
{
	CPropertyPage::OnDestroy();
	if (m_icoBrowse)
	{
		VERIFY( DestroyIcon(m_icoBrowse) );
		m_icoBrowse = NULL;
	}
}

// ==> Tabbed WebInterface settings panel [Stulle] - Stulle
void CPPgWebServer::InitTab(bool firstinit, int Page)
{
	if (firstinit) {
		m_tabCtr.DeleteAllItems();
		m_tabCtr.SetImageList(&m_imageList);
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, WEBSERVER, GetResString(IDS_TAB_WEB_SERVER), 0, (LPARAM)WEBSERVER); 
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, MULTIWEBSERVER, GetResString(IDS_TAB_MULTI_USER), 0, (LPARAM)MULTIWEBSERVER); 
	}

	if (m_tabCtr.GetSafeHwnd() != NULL)
		m_tabCtr.SetCurSel(Page);
}
void CPPgWebServer::OnTcnSelchangeTab(NMHDR * /* pNMHDR */, LRESULT *pResult)
{
//	int cur_sel = m_tabCtr.GetCurSel();
//	theApp.emuledlg->preferenceswnd->SwitchTab(cur_sel);

	// Retrieve tab to display
	TCITEM tabCtrlItem; 
	tabCtrlItem.mask = TCIF_PARAM;
	if(m_tabCtr.GetItem(m_tabCtr.GetCurSel(), &tabCtrlItem) == TRUE){
		SetTab(static_cast<eTab>(tabCtrlItem.lParam));
	}

	*pResult = 0;
}

void CPPgWebServer::SetTab(eTab tab){
	if(m_currentTab != tab){
		// Hide all control
		switch(m_currentTab){
			case WEBSERVER:
				GetDlgItem(IDC_WSENABLED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSENABLED)->EnableWindow(FALSE);
				GetDlgItem(IDC_WS_GZIP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WS_GZIP)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPORT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPORT)->EnableWindow(FALSE);
				GetDlgItem(IDC_TMPLPATH)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TMPLPATH)->EnableWindow(FALSE);
				GetDlgItem(IDC_TMPLBROWSE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TMPLBROWSE)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSRELOADTMPL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSTIMEOUT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSTIMEOUT)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASS)->EnableWindow(FALSE);
				GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSENABLEDLOW)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASSLOW)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASSLOW)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMENABLED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMENABLED)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPASSWORDFIELD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPASSWORDFIELD)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPORT_FIELD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPORT_FIELD)->EnableWindow(FALSE);
				GetDlgItem(IDC_GUIDELINK)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_GUIDELINK)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_GENERAL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_GENERAL)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPORT_LBL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPORT_LBL)->EnableWindow(FALSE);
				GetDlgItem(IDC_TEMPLATE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_TEMPLATE)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADMIN)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADMIN)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASS_LBL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASS_LBL)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_LOWUSER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_LOWUSER)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSPASS_LBL2)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSPASS_LBL2)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPASSWORD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPASSWORD)->EnableWindow(FALSE);
				GetDlgItem(IDC_MMPORT_LBL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MMPORT_LBL)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_MINS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_MINS)->EnableWindow(FALSE);
				GetDlgItem(IDC_WSUPNP)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_WSUPNP)->EnableWindow(FALSE);
				m_wndMobileLink.ShowWindow(SW_HIDE);
				m_wndMobileLink.EnableWindow(FALSE);
				break;
			case MULTIWEBSERVER:
				GetDlgItem(IDC_ADVADMINENABLED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN)->EnableWindow(FALSE);
				GetDlgItem(IDC_ACCOUNTSELECT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ACCOUNTSELECT)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_USER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_PASS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_KAD)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_SERVER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_SHARED)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_STATS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_PREFS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_CATS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_DELETE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_NEW)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_ADVADMIN_NOTE)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ADVADMIN_NOTE)->EnableWindow(FALSE);
				break;
			default:
				break;
		}

		// Show new controls
		m_currentTab = tab;
		switch(m_currentTab){
			case WEBSERVER:
				GetDlgItem(IDC_WSENABLED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSENABLED)->EnableWindow(TRUE);
				GetDlgItem(IDC_WS_GZIP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WS_GZIP)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPORT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPORT)->EnableWindow(TRUE);
				GetDlgItem(IDC_TMPLPATH)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TMPLPATH)->EnableWindow(TRUE);
				GetDlgItem(IDC_TMPLBROWSE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TMPLBROWSE)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSRELOADTMPL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSRELOADTMPL)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSTIMEOUT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSTIMEOUT)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPASS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASS)->EnableWindow(TRUE);
				GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSENABLEDLOW)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPASSLOW)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASSLOW)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMENABLED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMENABLED)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPASSWORDFIELD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPASSWORDFIELD)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPORT_FIELD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPORT_FIELD)->EnableWindow(TRUE);
				GetDlgItem(IDC_GUIDELINK)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_GUIDELINK)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_GENERAL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_GENERAL)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPORT_LBL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPORT_LBL)->EnableWindow(TRUE);
				GetDlgItem(IDC_TEMPLATE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_TEMPLATE)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADMIN)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADMIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPASS_LBL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASS_LBL)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_LOWUSER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_LOWUSER)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSPASS_LBL2)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSPASS_LBL2)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_MOBILEMULE)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPASSWORD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPASSWORD)->EnableWindow(TRUE);
				GetDlgItem(IDC_MMPORT_LBL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MMPORT_LBL)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSTIMEOUTLABEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_MINS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MINS)->EnableWindow(TRUE);
				GetDlgItem(IDC_WSUPNP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_WSUPNP)->EnableWindow(TRUE);
				if(m_wndMobileLink.GetSafeHwnd())
				{
					m_wndMobileLink.ShowWindow(SW_SHOW);
					m_wndMobileLink.EnableWindow(TRUE);
				}
				break;
			case MULTIWEBSERVER:
				GetDlgItem(IDC_ADVADMINENABLED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_ACCOUNTSELECT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ACCOUNTSELECT)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_USER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_PASS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_KAD)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_SERVER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_SHARED)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_STATS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_PREFS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_USERLEVEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_CATS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_DELETE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_NEW)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_USER)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->EnableWindow(TRUE);
				GetDlgItem(IDC_ADVADMIN_NOTE)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ADVADMIN_NOTE)->EnableWindow(TRUE);
				SetBoxes();
				break;
			default:
				break;
		}
	}
}
// <== Tabbed WebInterface settings panel [Stulle] - Stulle

// ==> Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle
BOOL CPPgWebServer::OnSetActive()
{
	/*
	if (IsDlgButtonChecked(IDC_ADVADMINENABLED)!=0)
	{
		GetDlgItem(IDC_WSPASS)->EnableWindow(FALSE);	
		GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(FALSE);	
//		GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(FALSE);	
		GetDlgItem(IDC_WSPASSLOW)->EnableWindow(FALSE);	
	}
	else
	{
		GetDlgItem(IDC_WSPASS)->EnableWindow(TRUE);	
		GetDlgItem(IDC_WSENABLEDLOW)->EnableWindow(TRUE);	
//		GetDlgItem(IDC_WS_ALLOWHILEVFUNC)->EnableWindow(TRUE);	
		GetDlgItem(IDC_WSPASSLOW)->EnableWindow(TRUE);	
	}
	*/

	if (IsWindow(m_hWnd))
	     SetBoxes();

	return TRUE;
}

void CPPgWebServer::OnEnableChange()
{
	thePrefs.m_bIonixWebsrv = (IsDlgButtonChecked(IDC_ADVADMINENABLED)!=0);

    SetBoxes();
	if (!thePrefs.m_bIonixWebsrv) {
	       		FillComboBox();
	            FillUserlevelBox();
	}
    SetModified();
}

afx_msg void CPPgWebServer::SetBoxes()
{
	bool bWSEnalbed = IsDlgButtonChecked(IDC_WSENABLED) && theApp.webserver->iMultiUserversion > 0;

	if(bWSEnalbed && IsDlgButtonChecked(IDC_ADVADMINENABLED)!=0)
	{
		GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
		m_cbAccountSelector.EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(TRUE);
		m_cbUserlevel.EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(TRUE);
		UpdateSelection();
	}
	else
	{	
		if(bWSEnalbed && theApp.webserver->iMultiUserversion)
			GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(FALSE);
		m_cbAccountSelector.EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(FALSE);
		m_cbUserlevel.EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(FALSE);
	}
}

void CPPgWebServer::UpdateSelection()
{
	int accountsel = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
	{
		//reset all if no selection possible
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(TRUE);
		CheckDlgButton(IDC_ADVADMIN_KAD, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_TRANSFER, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SEARCH, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SERVER, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SHARED, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_STATS, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_PREFS, BST_UNCHECKED);
		m_cbUserlevel.SetCurSel(0);
		GetDlgItem(IDC_ADVADMIN_PASS)->SetWindowText(_T(""));
		GetDlgItem(IDC_ADVADMIN_USER)->SetWindowText(_T(""));
		GetDlgItem(IDC_ADVADMIN_CATS)->SetWindowText(_T(""));
	}
	else
	{
		//set all data to our selectors
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		CheckDlgButton(IDC_ADVADMIN_KAD, tmp.RightsToKad);
		CheckDlgButton(IDC_ADVADMIN_TRANSFER, tmp.RightsToTransfered);
		CheckDlgButton(IDC_ADVADMIN_SEARCH, tmp.RightsToSearch);
		CheckDlgButton(IDC_ADVADMIN_SERVER, tmp.RightsToServers);
		CheckDlgButton(IDC_ADVADMIN_SHARED, tmp.RightsToSharedList);
		CheckDlgButton(IDC_ADVADMIN_STATS, tmp.RightsToStats);
		CheckDlgButton(IDC_ADVADMIN_PREFS, tmp.RightsToPrefs);
		m_cbUserlevel.SetCurSel(tmp.RightsToAddRemove);
		GetDlgItem(IDC_ADVADMIN_PASS)->SetWindowText(HIDDEN_PASSWORD);
		GetDlgItem(IDC_ADVADMIN_USER)->SetWindowText(tmp.User);
		GetDlgItem(IDC_ADVADMIN_CATS)->SetWindowText(tmp.RightsToCategories);
	}
}

void CPPgWebServer::FillComboBox()
{
	//clear old values first
	m_cbAccountSelector.ResetContent();
	m_cbAccountSelector.InsertString(0, GetResString(IDS_ADVADMIN_NEW));
	for(POSITION pos = theApp.webserver->AdvLogins.GetHeadPosition(); pos; theApp.webserver->AdvLogins.GetNext(pos))
		m_cbAccountSelector.InsertString(theApp.webserver->AdvLogins.GetKeyAt(pos), theApp.webserver->AdvLogins.GetValueAt(pos).User);	
}

void CPPgWebServer::FillUserlevelBox()
{
	//clear old values first
	m_cbUserlevel.ResetContent();
	m_cbUserlevel.InsertString(0, GetResString(IDS_ADVADMIN_GUEST));
	m_cbUserlevel.InsertString(1, GetResString(IDS_ADVADMIN_OPERATOR));
	m_cbUserlevel.InsertString(2, GetResString(IDS_ADVADMIN_ADMIN));
	m_cbUserlevel.InsertString(3, GetResString(IDS_ADVADMIN_HIADMIN));
}


#define SET_TCHAR_TO_STRING(t, s) {_stprintf(t, _T("%s"), s);}


void CPPgWebServer::OnSettingsChange()
{
	SetModified();

	int accountsel  = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel  == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
		return;

	tmp.RightsToKad = IsDlgButtonChecked(IDC_ADVADMIN_KAD)!=0;
	tmp.RightsToTransfered = IsDlgButtonChecked(IDC_ADVADMIN_TRANSFER)!=0;
	tmp.RightsToSearch = IsDlgButtonChecked(IDC_ADVADMIN_SEARCH)!=0;
	tmp.RightsToServers = IsDlgButtonChecked(IDC_ADVADMIN_SERVER)!=0;
	tmp.RightsToSharedList = IsDlgButtonChecked(IDC_ADVADMIN_SHARED)!=0;
	tmp.RightsToStats = IsDlgButtonChecked(IDC_ADVADMIN_STATS)!=0;
	tmp.RightsToPrefs = IsDlgButtonChecked(IDC_ADVADMIN_PREFS)!=0;
	//tmp.RightsToAddRemove = IsDlgButtonChecked(IDC_ADVADMIN_ADMIN)!=0;
	int j = m_cbUserlevel.GetCurSel();
	ASSERT(j <= 3); //only 0,1,2,3 allowed
	tmp.RightsToAddRemove = (uint8) j;
	
	CString buffer;
	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD)
		SET_TCHAR_TO_STRING(tmp.Pass, MD5Sum(buffer).GetHash());
	
	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.RightsToCategories, buffer);

	theApp.webserver->AdvLogins.SetAt(accountsel , tmp);

	FillComboBox();
	m_cbAccountSelector.SetCurSel(accountsel );
}

void CPPgWebServer::OnBnClickedNew()
{

    WebServDef tmp;
	CString buffer;
	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);
    if (_tcslen(tmp.User)==0) // username not filled?
       return; 

	SetModified();

	int i = theApp.webserver->AdvLogins.IsEmpty() ? 1 : theApp.webserver->AdvLogins.GetCount()+1;

	tmp.RightsToKad = IsDlgButtonChecked(IDC_ADVADMIN_KAD)!=0;
	tmp.RightsToTransfered = IsDlgButtonChecked(IDC_ADVADMIN_TRANSFER)!=0;
	tmp.RightsToSearch = IsDlgButtonChecked(IDC_ADVADMIN_SEARCH)!=0;
	tmp.RightsToServers = IsDlgButtonChecked(IDC_ADVADMIN_SERVER)!=0;
	tmp.RightsToSharedList = IsDlgButtonChecked(IDC_ADVADMIN_SHARED)!=0;
	tmp.RightsToStats = IsDlgButtonChecked(IDC_ADVADMIN_STATS)!=0;
	tmp.RightsToPrefs = IsDlgButtonChecked(IDC_ADVADMIN_PREFS)!=0;
	//tmp.RightsToAddRemove = IsDlgButtonChecked(IDC_ADVADMIN_ADMIN)!=0;
	int j = m_cbUserlevel.GetCurSel();
	ASSERT(j <= 3); //only 0,1,2,3 allowed
	tmp.RightsToAddRemove = (uint8) j;

	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD)
		SET_TCHAR_TO_STRING(tmp.Pass, MD5Sum(buffer).GetHash());

	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.RightsToCategories, buffer);

	theApp.webserver->AdvLogins.SetAt(i, tmp);
	theApp.webserver->SaveWebServConf(); //DEBUG should only be done at apply?
	FillComboBox();
	m_cbAccountSelector.SetCurSel(i);
    UpdateSelection();


	GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
	GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
}

void CPPgWebServer::OnBnClickedDel()
{
	SetModified();

	const int i = m_cbAccountSelector.GetCurSel();  
    WebServDef tmp;  
    if(i == -1 || !theApp.webserver->AdvLogins.Lookup(i, tmp))   
         return;  
  
	CRBMap<uint32, WebServDef> tmpmap;  
  
    //retrieve all "wrong" entries  
    for(POSITION pos = theApp.webserver->AdvLogins.GetHeadPosition(); pos;)  
    {  
         POSITION pos2 = pos;  
         theApp.webserver->AdvLogins.GetNext(pos);  
  
         const int j = theApp.webserver->AdvLogins.GetKeyAt(pos2);  
         if(j == i)  
              theApp.webserver->AdvLogins.RemoveAt(pos2);  
         else if(j > i)  
         {  
              tmpmap.SetAt(j-1, theApp.webserver->AdvLogins.GetValueAt(pos2));  
              theApp.webserver->AdvLogins.RemoveAt(pos2);  
         }  
    }
	
	//reinsert all "wrong" entries correctly
	for(POSITION pos = tmpmap.GetHeadPosition(); pos; tmpmap.GetNext(pos))
		theApp.webserver->AdvLogins.SetAt(tmpmap.GetKeyAt(pos), tmpmap.GetValueAt(pos));
	
	FillComboBox();
	m_cbAccountSelector.SetCurSel(0); //set to the empty field
	UpdateSelection();

	
}
// <== Ionix advanced (multiuser) webserver [iOniX/Aireoreion/wizard/leuk_he] - Stulle