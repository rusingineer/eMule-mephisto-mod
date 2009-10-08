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
#include <math.h>
#include <afxinet.h>
#define MMNODRV			// mmsystem: Installable driver support
//#define MMNOSOUND		// mmsystem: Sound support
#define MMNOWAVE		// mmsystem: Waveform support
#define MMNOMIDI		// mmsystem: MIDI support
#define MMNOAUX			// mmsystem: Auxiliary audio support
#define MMNOMIXER		// mmsystem: Mixer support
#define MMNOTIMER		// mmsystem: Timer support
#define MMNOJOY			// mmsystem: Joystick support
#define MMNOMCI			// mmsystem: MCI support
#define MMNOMMIO		// mmsystem: Multimedia file I/O support
#define MMNOMMSYSTEM	// mmsystem: General MMSYSTEM functions
#include <Mmsystem.h>
#include <HtmlHelp.h>
#include <share.h>
#include "emule.h"
#include "emuleDlg.h"
#include "ServerWnd.h"
#include "KademliaWnd.h"
#include "TransferWnd.h"
#include "SearchResultsWnd.h"
#include "SearchDlg.h"
#include "SharedFilesWnd.h"
#include "ChatWnd.h"
#include "IrcWnd.h"
#include "StatisticsDlg.h"
#include "CreditsDlg.h"
#include "PreferencesDlg.h"
#include "Sockets.h"
#include "KnownFileList.h"
#include "ServerList.h"
#include "Opcodes.h"
#include "SharedFileList.h"
#include "ED2KLink.h"
//Xman slpashscreen
/*
#include "Splashscreen.h"
*/
//Xman end
#include "PartFileConvert.h"
#include "EnBitmap.h"
#include "Wizard.h"
#include "Exceptions.h"
#include "SearchList.h"
#include "HTRichEditCtrl.h"
#include "FrameGrabThread.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/routing/RoutingZone.h"
#include "kademlia/routing/contact.h"
#include "kademlia/kademlia/prefs.h"
#include "KadSearchListCtrl.h"
#include "KadContactListCtrl.h"
#include "PerfLog.h"
#include "DropTarget.h"
//Xman
/*
#include "LastCommonRouteFinder.h"
*/
//Xman end
#include "WebServer.h"
#include "MMServer.h"
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "UploadQueue.h"
#include "ClientList.h"
#include "UploadBandwidthThrottler.h"
#include "FriendList.h"
#include "IPFilter.h"
#include "Statistics.h"
#include "MuleToolbarCtrl.h"
#include "TaskbarNotifier.h"
#include "MuleStatusbarCtrl.h"
#include "ListenSocket.h"
#include "Server.h"
#include "PartFile.h"
#include "Scheduler.h"
#include "ClientCredits.h"
#include "MenuCmds.h"
#include "MuleSystrayDlg.h"
#include "IPFilterDlg.h"
#include "WebServices.h"
#include "DirectDownloadDlg.h"
#include "PeerCacheFinder.h"
#include "Statistics.h"
#include "FirewallOpener.h"
#include "StringConversion.h"
#include "aichsyncthread.h"
#include "Log.h"
#include "MiniMule.h"
#include "UserMsgs.h"
//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include "DLP.h" //Xman DLP

#include "TextToSpeech.h"
#include "Collection.h"
#include "CollectionViewDialog.h"
#include "VisualStylesXP.h"
// ==> UPnP support [MoNKi] - leuk_he
/*
#include "UPnPImpl.h"
#include "UPnPImplWrapper.h"
*/
// <== UPnP support [MoNKi] - leuk_he
#include <dbt.h>

//zz_fly
#include "SR13-ImportParts.h" //MORPH - Added by SiRoB, Import Parts
// MORPH START - Added by Commander, Friendlinks [emulEspaa]
#include "Friend.h"
// MORPH END - Added by Commander, Friendlinks [emulEspaa]
//zz_fly end

#include "HttpDownloadDlg.h" // Advanced Updates [MorphXT/Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL FirstTimeWizard();

#define	SYS_TRAY_ICON_COOKIE_FORCE_UPDATE	(UINT)-1

const static UINT UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);
UINT g_uMainThreadId = 0;

// ==> Invisible Mode [TPT/MoNKi] - Stulle
// Allows "invisible mode" on multiple instances of eMule
#ifdef _DEBUG
#define EMULE_GUID_INVMODE				"EMULE-{4EADC6FC-516F-4b7c-9066-97D893649569}-DEBUG-INVISIBLEMODE"
#else
#define EMULE_GUID_INVMODE				"EMULE-{4EADC6FC-516F-4b7c-9066-97D893649569}-INVISIBLEMODE"
#endif
const static UINT UWM_RESTORE_WINDOW_IM=RegisterWindowMessage(_T(EMULE_GUID_INVMODE));
// <== Invisible Mode [TPT/MoNKi] - Stulle


///////////////////////////////////////////////////////////////////////////
// CemuleDlg Dialog

IMPLEMENT_DYNAMIC(CMsgBoxException, CException)

BEGIN_MESSAGE_MAP(CemuleDlg, CTrayDialog)
	///////////////////////////////////////////////////////////////////////////
	// Windows messages
	//
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ENDSESSION()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_WM_MENUCHAR()
	ON_WM_QUERYENDSESSION()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_COPYDATA, OnWMData)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_USERCHANGED, OnUserChanged)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_WM_SETTINGCHANGE()
	ON_WM_DEVICECHANGE()
	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)	// Invisible Mode [TPT/MoNKi] - Stulle

	ON_WM_MEASUREITEM() // XP Style Menu [Xanatos] - Stulle

	///////////////////////////////////////////////////////////////////////////
	// WM_COMMAND messages
	//
	ON_COMMAND(MP_CONNECT, StartConnection)
	ON_COMMAND(MP_DISCONNECT, CloseConnection)
	ON_COMMAND(MP_EXIT, OnClose)
	ON_COMMAND(MP_RESTORE, RestoreWindow)
	// quick-speed changer -- 
	ON_COMMAND_RANGE(MP_QS_U10, MP_QS_UP10, QuickSpeedUpload)
	ON_COMMAND_RANGE(MP_QS_D10, MP_QS_DC, QuickSpeedDownload)
	//--- quickspeed - paralize all ---
	ON_COMMAND_RANGE(MP_QS_PA, MP_QS_UA, QuickSpeedOther)
	// quick-speed changer -- based on xrmb	
	ON_NOTIFY_EX_RANGE(RBN_CHEVRONPUSHED, 0, 0xFFFF, OnChevronPushed)

	ON_REGISTERED_MESSAGE(UWM_ARE_YOU_EMULE, OnAreYouEmule)
	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	ON_REGISTERED_MESSAGE(UWM_RESTORE_WINDOW_IM, OnRestoreWindowInvisibleMode)
	// <== Invisible Mode [TPT/MoNKi] - Stulle
	ON_BN_CLICKED(IDC_HOTMENU, OnBnClickedHotmenu)

	///////////////////////////////////////////////////////////////////////////
	// WM_USER messages
	//
	ON_MESSAGE(UM_TASKBARNOTIFIERCLICKED, OnTaskbarNotifierClicked)
	ON_MESSAGE(UM_CLOSE_MINIMULE, OnCloseMiniMule)
	
	// Webserver messages
	ON_MESSAGE(WEB_GUI_INTERACTION, OnWebGUIInteraction)
	ON_MESSAGE(WEB_CLEAR_COMPLETED, OnWebServerClearCompleted)
	ON_MESSAGE(WEB_FILE_RENAME, OnWebServerFileRename)
	ON_MESSAGE(WEB_ADDDOWNLOADS, OnWebAddDownloads)
	ON_MESSAGE(WEB_CATPRIO, OnWebSetCatPrio)
	ON_MESSAGE(WEB_ADDREMOVEFRIEND, OnAddRemoveFriend)

	// Version Check DNS
	ON_MESSAGE(UM_VERSIONCHECK_RESPONSE, OnVersionCheckResponse)

	//Xman versions check
	ON_MESSAGE(UM_MVERSIONCHECK_RESPONSE, OnMVersionCheckResponse)
	ON_MESSAGE(UM_DLPVERSIONCHECK_RESPONSE, OnDLPVersionCheckResponse) //Xman DLP //MOD NOTE: if you are using DLP, don't remove/modify this versions-check
	//Xman end

	// ScarAngel Version Check - Stulle
	ON_MESSAGE(UM_SVERSIONCHECK_RESPONSE, OnSVersionCheckResponse)
	// Advanced Updates [MorphXT/Stulle] - Stulle
	ON_MESSAGE(UM_DLPAUTOVERCHECK_RESPONSE, OnDLPAutoVerCheckResponse)
	ON_MESSAGE(UM_IPFFILTERAUTOVERCHECK_RESPONSE, OnIPFilterAutoVerCheckResponse)

	// PeerCache DNS
	ON_MESSAGE(UM_PEERCHACHE_RESPONSE, OnPeerCacheResponse)

	// UPnP
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	ON_MESSAGE(UM_UPNP_RESULT, OnUPnPResult)
	*/
	// <== UPnP support [MoNKi] - leuk_he

	///////////////////////////////////////////////////////////////////////////
	// WM_APP messages
	//
	ON_MESSAGE(TM_FINISHEDHASHING, OnFileHashed)
	ON_MESSAGE(TM_FILEOPPROGRESS, OnFileOpProgress)
	ON_MESSAGE(TM_HASHFAILED, OnHashFailed)
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	ON_MESSAGE(TM_PARTHASHEDOK, OnPartHashedOK)
	ON_MESSAGE(TM_PARTHASHEDCORRUPT, OnPartHashedCorrupt)
	ON_MESSAGE(TM_PARTHASHEDOKAICHRECOVER, OnPartHashedOKAICHRecover)
	ON_MESSAGE(TM_PARTHASHEDCORRUPTAICHRECOVER, OnPartHashedCorruptAICHRecover)
	// END SLUGFILLER: SafeHash
	ON_MESSAGE(TM_READBLOCKFROMFILEDONE, OnReadBlockFromFileDone) // SiRoB: ReadBlockFromFileThread
	ON_MESSAGE(TM_FLUSHDONE, OnFlushDone) // SiRoB: Flush Thread
	ON_MESSAGE(TM_DOTIMER, DoTimer) //Xman process timer code via messages (Xanatos)
	ON_MESSAGE(TM_IMPORTPART, OnImportPart) //MORPH - Added by SiRoB, Import Parts - added by zz_fly
	//Xman end
	ON_MESSAGE(TM_SAVEDONE, OnSaveDone) // File Settings [sivka/Stulle] - Stulle
	ON_MESSAGE(TM_SAVEKNOWNDONE, OnSaveKnownDone) // Threaded Known Files Saving [Stulle] - Stulle
	ON_MESSAGE(TM_FRAMEGRABFINISHED, OnFrameGrabFinished)
	ON_MESSAGE(TM_FILEALLOCEXC, OnFileAllocExc)
	ON_MESSAGE(TM_FILECOMPLETED, OnFileCompleted)
	ON_MESSAGE(TM_CONSOLETHREADEVENT, OnConsoleThreadEvent)
END_MESSAGE_MAP()

CemuleDlg::CemuleDlg(CWnd* pParent /*=NULL*/)
	: CTrayDialog(CemuleDlg::IDD, pParent)
{
	g_uMainThreadId = GetCurrentThreadId();
	preferenceswnd = new CPreferencesDlg;
	serverwnd = new CServerWnd;
	kademliawnd = new CKademliaWnd;
	transferwnd = new CTransferWnd;
	sharedfileswnd = new CSharedFilesWnd;
	searchwnd = new CSearchDlg;
	chatwnd = new CChatWnd;
	ircwnd = new CIrcWnd;
	statisticswnd = new CStatisticsDlg;
	toolbar = new CMuleToolbarCtrl;
	statusbar = new CMuleStatusBarCtrl;
	m_wndTaskbarNotifier = new CTaskbarNotifier;

	m_hIcon = NULL;
	theApp.m_app_state = APP_STATE_RUNNING;
	ready = false; 
	m_bStartMinimizedChecked = false;
	m_bStartMinimized = false;
	memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
	m_uUpDatarate = 0;
	m_uDownDatarate = 0;
	status = 0;
	activewnd = NULL;
	for (int i = 0; i < _countof(connicons); i++)
		connicons[i] = NULL;
	transicons[0] = NULL;
	transicons[1] = NULL;
	transicons[2] = NULL;
	transicons[3] = NULL;
	imicons[0] = NULL;
	imicons[1] = NULL;
	imicons[2] = NULL;
	m_iMsgIcon = 0;
	m_iMsgBlinkState = false;
	m_icoSysTrayConnected = NULL;
	m_icoSysTrayDisconnected = NULL;
	m_icoSysTrayLowID = NULL;
	usericon = NULL;
	m_icoSysTrayCurrent = NULL;
	m_hTimer = 0;
	notifierenabled = false;
	m_pDropTarget = new CMainFrameDropTarget;
	//Xman new slpash-screen arrangement
	/*
	m_pSplashWnd = NULL;
	m_dwSplashTime = (DWORD)-1;
	*/
	//Xman end
	m_pSystrayDlg = NULL;
	m_pMiniMule = NULL;
	m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	m_hUPnPTimeOutTimer = 0;
	m_bConnectRequestDelayedForUPnP = false;
	*/
	// <== UPnP support [MoNKi] - leuk_he
	m_bEd2kSuspendDisconnect = false;
	m_bKadSuspendDisconnect = false;

	//Xman versions check
	m_bCheckwasDone=false;
	//Xman end

	// ==> Show in MSN7 [TPT] - Stulle
	m_dwMSNtime = 0;
	m_uMSNup2Date = 0;
	// <== Show in MSN7 [TPT] - Stulle

	b_HideApp = false; // Invisible Mode [TPT/MoNKi] - Stulle

	// ==> Completed in Tray [Stulle] - Stulle
	m_icoSysTrayConnectedPlus = NULL;
	m_icoSysTrayDisconnectedPlus = NULL;
	m_icoSysTrayLowIDPlus = NULL;
	m_dwTrayTime = 0;
	m_bTrayBool = false;
	m_bTrayBoolOld = false;
	// <== Completed in Tray [Stulle] - Stulle
}

CemuleDlg::~CemuleDlg()
{
	CloseTTS();
	DestroyMiniMule();
	if (m_icoSysTrayCurrent) VERIFY( DestroyIcon(m_icoSysTrayCurrent) );
	if (m_hIcon) VERIFY( ::DestroyIcon(m_hIcon) );
	for (int i = 0; i < _countof(connicons); i++){
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	if (imicons[0]) VERIFY( ::DestroyIcon(imicons[0]) );
	if (imicons[1]) VERIFY( ::DestroyIcon(imicons[1]) );
	if (imicons[2]) VERIFY( ::DestroyIcon(imicons[2]) );
	if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	// ==> Completed in Tray [Stulle] - Stulle
	if (m_icoSysTrayConnectedPlus) VERIFY( ::DestroyIcon(m_icoSysTrayConnectedPlus) );
	if (m_icoSysTrayDisconnectedPlus) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnectedPlus) );
	if (m_icoSysTrayLowIDPlus) VERIFY( ::DestroyIcon(m_icoSysTrayLowIDPlus) );
	// <== Completed in Tray [Stulle] - Stulle
	if (usericon) VERIFY( ::DestroyIcon(usericon) );

	// already destroyed by windows?
	//VERIFY( m_menuUploadCtrl.DestroyMenu() );
	//VERIFY( m_menuDownloadCtrl.DestroyMenu() );
	//VERIFY( m_SysMenuOptions.DestroyMenu() );

	delete preferenceswnd;
	delete serverwnd;
	delete kademliawnd;
	delete transferwnd;
	delete sharedfileswnd;
	delete chatwnd;
	delete ircwnd;
	delete statisticswnd;
	delete toolbar;
	delete statusbar;
	delete m_wndTaskbarNotifier;
	delete m_pDropTarget;

	theApp.DestroyExtraFonts(); // Design Settings [eWombat/Stulle] - Stulle
}

void CemuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CTrayDialog::DoDataExchange(pDX);
}

LRESULT CemuleDlg::OnAreYouEmule(WPARAM, LPARAM)
{
	return UWM_ARE_YOU_EMULE;
}

void DialogCreateIndirect(CDialog *pWnd, UINT uID)
{
#if 0
	// This could be a nice way to change the font size of the main windows without needing
	// to re-design the dialog resources. However, that technique does not work for the
	// SearchWnd and it also introduces new glitches (which would need to get resolved)
	// in almost all of the main windows.
	CDialogTemplate dlgTempl;
	dlgTempl.Load(MAKEINTRESOURCE(uID));
	dlgTempl.SetFont(_T("MS Shell Dlg"), 8);
	pWnd->CreateIndirect(dlgTempl.m_hTemplate);
	FreeResource(dlgTempl.Detach());
#else
	pWnd->Create(uID);
#endif
}

BOOL CemuleDlg::OnInitDialog()
{
	m_bStartMinimized = thePrefs.GetStartMinimized();
	if (!m_bStartMinimized)
		m_bStartMinimized = theApp.DidWeAutoStart();

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	if (thePrefs.GetInvisibleMode() && (theApp.DidWeAutoStart() || thePrefs.GetInvisibleModeStart()))
		m_bStartMinimized = true;
	// <== Invisible Mode [TPT/MoNKi] - Stulle

	// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
	if (thePrefs.IsFirstStart())
		m_bStartMinimized = false;

	//Xman new slpash-screen arrangement
	/*
	// show splashscreen as early as possible to "entertain" user while starting emule up
	if (thePrefs.UseSplashScreen() && !m_bStartMinimized)
		ShowSplash();
	*/
	//Xman end
	// ==> Startupsound [Commander] - mav744
	if (thePrefs.UseStartupSound()){
		if(PathFileExists(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("startup.wav"))) 
			PlaySound(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("startup.wav"), NULL, SND_FILENAME | SND_NOSTOP | SND_NOWAIT | SND_ASYNC);
		else
			AddLogLine(false,GetResString(IDS_MISSING_STARTUPSOUND));
		}
	// <== Startupsound [Commander] - mav744

	// Create global GUI objects
	theApp.CreateAllFonts();
	theApp.CreateBackwardDiagonalBrush();
	m_wndTaskbarNotifier->SetTextDefaultFont();
	CTrayDialog::OnInitDialog();
	InitWindowStyles(this);
	CreateToolbarCmdIconMap();

	// ==> Static Tray Icon [MorphXT] - MyTh88
	if(thePrefs.GetStaticIcon())
		// ==> TBH: minimule - Stulle
		/*
		TrayShow();
		*/
		TrayShow(FALSE);
		// <== TBH: minimule - Stulle
	// <== Static Tray Icon [MorphXT] - MyTh88

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL){
		pSysMenu->AppendMenu(MF_SEPARATOR);

		ASSERT( (MP_ABOUTBOX & 0xFFF0) == MP_ABOUTBOX && MP_ABOUTBOX < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX));

		ASSERT( (MP_VERSIONCHECK & 0xFFF0) == MP_VERSIONCHECK && MP_VERSIONCHECK < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK));

		//Xman versions check
		// ==> Removed Xtreme version check [Stulle] - Stulle
		/*
		ASSERT( (MP_MVERSIONCHECK & 0xFFF0) == MP_MVERSIONCHECK && MP_MVERSIONCHECK < 0xF000);
		pSysMenu->AppendMenu(MF_STRING, MP_MVERSIONCHECK, _T("Xtreme-Version-Check"));
		*/
		// <== Removed Xtreme version check [Stulle] - Stulle
		//Xman end

		// ==> ScarAngel Version Check - Stulle
		ASSERT( (MP_SVERSIONCHECK & 0xFFF0) == MP_SVERSIONCHECK && MP_SVERSIONCHECK < 0xF000);
		// ==> Mephisto mod [Stulle] - Mephisto
		/*
		pSysMenu->AppendMenu(MF_STRING, MP_SVERSIONCHECK, GetResString(IDS_SVERSIONCHECK));
		*/
		pSysMenu->AppendMenu(MF_STRING, MP_SVERSIONCHECK, GetResString(IDS_MVERSIONCHECK));
		// <== Mephisto mod [Stulle] - Mephisto
		// <== ScarAngel Version Check - Stulle

		// remaining system menu entries are created later...
	}

	CWnd* pwndToolbarX = toolbar;
	if (toolbar->Create(WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, IDC_TOOLBAR))
	{
		toolbar->Init();
		if (thePrefs.GetUseReBarToolbar())
		{
		    if (m_ctlMainTopReBar.Create(WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
									     RBS_BANDBORDERS | RBS_AUTOSIZE | CCS_NODIVIDER, 
									     CRect(0, 0, 0, 0), this, AFX_IDW_REBAR))
		    {
			    CSize sizeBar;
			    VERIFY( toolbar->GetMaxSize(&sizeBar) );
			    REBARBANDINFO rbbi = {0};
			    rbbi.cbSize = sizeof(rbbi);
				rbbi.fMask = RBBIM_STYLE | RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_IDEALSIZE | RBBIM_ID;
			    rbbi.fStyle = RBBS_NOGRIPPER | RBBS_BREAK | RBBS_USECHEVRON;
			    rbbi.hwndChild = toolbar->m_hWnd;
			    rbbi.cxMinChild = sizeBar.cy;
			    rbbi.cyMinChild = sizeBar.cy;
			    rbbi.cxIdeal = sizeBar.cx;
			    rbbi.cx = rbbi.cxIdeal;
				rbbi.wID = 0;
			    VERIFY( m_ctlMainTopReBar.InsertBand((UINT)-1, &rbbi) );
				toolbar->SaveCurHeight();
		    	toolbar->UpdateBackground();
    
			    pwndToolbarX = &m_ctlMainTopReBar;
		    }
		}
	}

	theApp.CreateExtraFonts(GetFont()); // Design Settings [eWombat/Stulle] - Stulle

	// set title
	// Maella -Support for tag ET_MOD_VERSION 0x55
	/*
	SetWindowText(_T("eMule v") + theApp.m_strCurVersionLong);
	*/
	// ==> ModID [itsonlyme/SiRoB] - Stulle
	/*
	SetWindowText(_T("eMule v") + theApp.m_strCurVersionLong + _T(" ") + MOD_VERSION);
	*/
	SetWindowText(_T("eMule v") + theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]"));
	// <== ModID [itsonlyme/SiRoB] - Stulle
	//Xman end

	// Init taskbar notifier
	m_wndTaskbarNotifier->Create(this);
	LoadNotifier(thePrefs.GetNotifierConfiguration());

	// ==> TBH: minimule - Max
	if (theApp.minimule != NULL)
		theApp.minimule->Create(IDD_MINI_MULE,this);
	// <== TBH: minimule - Max

	// set statusbar
	// the statusbar control is created as a custom control in the dialog resource,
	// this solves font and sizing problems when using large system fonts
	statusbar->SubclassWindow(GetDlgItem(IDC_STATUSBAR)->m_hWnd);
	statusbar->EnableToolTips(true);

	statusbar->UpdateColor();// Design Settings [eWombat/Stulle] - Max

	SetStatusBarPartsSize();

	// create main window dialog pages
	//zz_fly :: the fix(Xtreme 7.0&7.1) is not needed. but the gui in WinXP still has something wrong. see also: CTransferWnd::ResetTransToolbar
	DialogCreateIndirect(serverwnd, IDD_SERVER);
	DialogCreateIndirect(sharedfileswnd, IDD_FILES);
	searchwnd->Create(this); // can not use 'DialogCreateIndirect' for the SearchWnd, grrr..
	DialogCreateIndirect(chatwnd, IDD_CHAT);
	DialogCreateIndirect(transferwnd, IDD_TRANSFER);
	DialogCreateIndirect(statisticswnd, IDD_STATISTICS);
	DialogCreateIndirect(kademliawnd, IDD_KADEMLIAWND);
	DialogCreateIndirect(ircwnd, IDD_IRC);

	// with the top rebar control, some XP themes look better with some additional lite borders.. some not..
	//serverwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//sharedfileswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//searchwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//chatwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//transferwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//statisticswnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//kademliawnd->ModifyStyleEx(0, WS_EX_STATICEDGE);
	//ircwnd->ModifyStyleEx(0, WS_EX_STATICEDGE);

	// optional: restore last used main window dialog
	if (thePrefs.GetRestoreLastMainWndDlg()){
		switch (thePrefs.GetLastMainWndDlgID()){
		case IDD_SERVER:
			SetActiveDialog(serverwnd);
			break;
		case IDD_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
		case IDD_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case IDD_CHAT:
			SetActiveDialog(chatwnd);
			break;
		case IDD_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case IDD_STATISTICS:
			SetActiveDialog(statisticswnd);
			break;
		case IDD_KADEMLIAWND:
			SetActiveDialog(kademliawnd);
			break;
		case IDD_IRC:
			SetActiveDialog(ircwnd);
			break;
		}
	}

	// if still no active window, activate server window
	if (activewnd == NULL)
		SetActiveDialog(serverwnd);

	SetAllIcons();
	Localize();

	// set updateintervall of graphic rate display (in seconds)
	//ShowConnectionState(false);

	// adjust all main window sizes for toolbar height and maximize the child windows
	CRect rcClient, rcToolbar, rcStatusbar;
	GetClientRect(&rcClient);
	pwndToolbarX->GetWindowRect(&rcToolbar);
	statusbar->GetWindowRect(&rcStatusbar);
	rcClient.top += rcToolbar.Height();
	rcClient.bottom -= rcStatusbar.Height();

	CWnd* apWnds[] =
	{
		serverwnd,
		kademliawnd,
		transferwnd,
		sharedfileswnd,
		searchwnd,
		chatwnd,
		ircwnd,
		statisticswnd
	};
	for (int i = 0; i < _countof(apWnds); i++)
		apWnds[i]->SetWindowPos(NULL, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);

	// ==> High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
	CRect		rect;
	CRect rect1,rect2;
	toolbar->GetClientRect(&rect);

	// set updateintervall of graphic rate display (in seconds)
	//rect1.top = rect.top+2;
	rect1.top = 2; // we need to make it clear on startup...
	rect1.right = rect.right-2;
	//rect1.bottom = rect.top+(rect.Height()/2)-1;
	rect1.bottom = 26; // we need to make it clear on startup...
	rect1.left = rect.right-150;

	//rect2.top = rect.top+(rect.Height()/2)+1;
	rect2.top = 28; // we need to make it clear on startup...
	rect2.right = rect.right-2;
	//rect2.bottom = rect.bottom-2;
	rect2.bottom = 52; // we need to make it clear on startup...
	rect2.left = rect.right-150;

	CSize	csMaxSize;
	bool	bTooSmall = false;

	toolbar->GetMaxSize(&csMaxSize);
	if (rect.left + csMaxSize.cx > rect.right - 150)
		bTooSmall = true;
	m_co_UpTrafficGraph.Create(IDD_SPEEDGRAPH,rect1,this);
	m_co_DownTrafficGraph.Create(IDD_SPEEDGRAPH,rect2,this);
	if(bTooSmall || thePrefs.GetShowSpeedMeter() == false)
	{
		m_co_UpTrafficGraph.ShowWindow(SW_HIDE);
		m_co_DownTrafficGraph.ShowWindow(SW_HIDE);
	}

	// Traffic graph
	m_co_UpTrafficGraph.Init_Graph(_T("Up"),(UINT)thePrefs.GetMaxGraphUploadRate());
	m_co_DownTrafficGraph.Init_Graph(_T("Down"),(UINT)thePrefs.GetMaxGraphDownloadRate());
	// <== High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88

	// anchors
	AddAnchor(*serverwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*kademliawnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*transferwnd,		TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*sharedfileswnd,	TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*searchwnd,		TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(*chatwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*ircwnd,			TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*statisticswnd,	TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(*pwndToolbarX,	TOP_LEFT, TOP_RIGHT);
	AddAnchor(*statusbar,		BOTTOM_LEFT, BOTTOM_RIGHT);

	statisticswnd->ShowInterval();

	// tray icon
	TraySetMinimizeToTray(thePrefs.GetMinTrayPTR());
	TrayMinimizeToTrayChange();

	ShowTransferRate(true);
    ShowPing();
	searchwnd->UpdateCatTabs();

	///////////////////////////////////////////////////////////////////////////
	// Restore saved window placement
	//
	WINDOWPLACEMENT wp = {0};
	wp.length = sizeof(wp);
	wp = thePrefs.GetEmuleWindowPlacement();
	if (m_bStartMinimized)
	{
		// To avoid the window flickering during startup we try to set the proper window show state right here.
		// ==> Invisible Mode [TPT/MoNKi] - Stulle
		/*
		if (*thePrefs.GetMinTrayPTR())
		*/
		if (*thePrefs.GetMinTrayPTR() || thePrefs.GetInvisibleMode() && (theApp.DidWeAutoStart() || thePrefs.GetInvisibleModeStart()))
		// <== Invisible Mode [TPT/MoNKi] - Stulle
		{
			// Minimize to System Tray
			//
			// Unfortunately this does not work. The eMule main window is a modal dialog which is invoked
			// by CDialog::DoModal which eventually calls CWnd::RunModalLoop. Look at 'MLF_SHOWONIDLE' and
			// 'bShowIdle' in the above noted functions to see why it's not possible to create the window
			// right in hidden state.

			//--- attempt #1
			//wp.showCmd = SW_HIDE;
			//TrayShow();
			//--- doesn't work at all

			//--- attempt #2
			//if (wp.showCmd == SW_SHOWMAXIMIZED)
			//	wp.flags = WPF_RESTORETOMAXIMIZED;
			//m_bStartMinimizedChecked = false; // post-hide the window..
			//--- creates window flickering

			//--- attempt #3
			// Minimize the window into the task bar and later move it into the tray bar
			if (wp.showCmd == SW_SHOWMAXIMIZED)
				wp.flags = WPF_RESTORETOMAXIMIZED;
			wp.showCmd = SW_MINIMIZE;
			m_bStartMinimizedChecked = false;

			// to get properly restored from tray bar (after attempt #3) we have to use a patched 'restore' window cmd..
			m_wpFirstRestore = thePrefs.GetEmuleWindowPlacement();
			m_wpFirstRestore.length = sizeof(m_wpFirstRestore);
			if (m_wpFirstRestore.showCmd != SW_SHOWMAXIMIZED)
				m_wpFirstRestore.showCmd = SW_SHOWNORMAL;
		}
		else {
			// Minimize to System Taskbar
			//
			if (wp.showCmd == SW_SHOWMAXIMIZED)
				wp.flags = WPF_RESTORETOMAXIMIZED;
			wp.showCmd = SW_MINIMIZE; // Minimize window but do not activate it.
			m_bStartMinimizedChecked = true;
		}
	}
	else
	{
		// Allow only SW_SHOWNORMAL and SW_SHOWMAXIMIZED. Ignore SW_SHOWMINIMIZED to make sure the window
		// becomes visible. If user wants SW_SHOWMINIMIZED, we already have an explicit option for this (see above).
		if (wp.showCmd != SW_SHOWMAXIMIZED)
			wp.showCmd = SW_SHOWNORMAL;
		m_bStartMinimizedChecked = true;
	}
	SetWindowPlacement(&wp);

	if (thePrefs.GetWSIsEnabled())
		theApp.webserver->StartServer();
	theApp.mmserver->Init();

	VERIFY( (m_hTimer = ::SetTimer(NULL, NULL, 300, StartupTimer)) != NULL );
	if (thePrefs.GetVerbose() && !m_hTimer)
		AddDebugLogLine(true,_T("Failed to create 'startup' timer - %s"),GetErrorMessage(GetLastError()));

	theStats.starttime = GetTickCount();
	m_bTbhMiniMuleVis = false; // TBH: minimule - Stulle

	// ==> UPnP support [MoNKi] - leuk_he
	/*
	// Start UPnP prot forwarding
	if (thePrefs.IsUPnPEnabled())
		StartUPnP();
	*/
	// <== UPnP support [MoNKi] - leuk_he

	if (thePrefs.IsFirstStart())
	{
		// temporary disable the 'startup minimized' option, otherwise no window will be shown at all
		m_bStartMinimized = false;
		
		//Xman
		// SLUGFILLER: SafeHash remove - wait until emule is ready before opening the wizard
		/*
		DestroySplash();

		if (FirstTimeWizard()){
			// start connection wizard
			CConnectionWizardDlg conWizard;
			conWizard.DoModal();
		}
		*/
		//Xman end
	}

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	if(thePrefs.GetInvisibleMode())
		RegisterInvisibleHotKey();
	// <== Invisible Mode [TPT/MoNKi] - Stulle

	VERIFY( m_pDropTarget->Register(this) );

	// initalize PeerCache
	theApp.m_pPeerCache->Init(thePrefs.GetPeerCacheLastSearch(), thePrefs.WasPeerCacheFound(), thePrefs.IsPeerCacheDownloadEnabled(), thePrefs.GetPeerCachePort());
	
	//Xman
	// SiRoB: SafeHash fix (see StartupTimer)
	/*
	// start aichsyncthread
	AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
	*/
	//Xman end

	// debug info
	DebugLog(_T("Using '%s' as config directory"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR)); 
	return TRUE;
}

// modders: dont remove or change the original versioncheck! (additionals are ok)
void CemuleDlg::DoVersioncheck(bool manual) {

	if (!manual && thePrefs.GetLastVC()!=0) {
		CTime last(thePrefs.GetLastVC());
		struct tm tmTemp;
		time_t tLast = safe_mktime(last.GetLocalTm(&tmTemp));
		time_t tNow = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
#ifndef _BETA
		if ( (difftime(tNow,tLast) / 86400) < thePrefs.GetUpdateDays() ){
#else
		if ( (difftime(tNow,tLast) / 86400) < 3 ){
#endif
			return;
		}
	}
#ifndef _BETA
	if (WSAAsyncGetHostByName(m_hWnd, UM_VERSIONCHECK_RESPONSE, "vcdns2.emule-project.org", m_acVCDNSBuffer, sizeof(m_acVCDNSBuffer)) == 0){
#else
	if (WSAAsyncGetHostByName(m_hWnd, UM_VERSIONCHECK_RESPONSE, "vcdns1.emule-project.org", m_acVCDNSBuffer, sizeof(m_acVCDNSBuffer)) == 0){
#endif
		AddLogLine(true,GetResString(IDS_NEWVERSIONFAILED));
	}
}

//Xman versions check
void CemuleDlg::DoMVersioncheck(bool manual) {
	if (!manual && thePrefs.GetLastMVC()!=0) {
		CTime last(thePrefs.GetLastMVC());
		struct tm tmTemp;
		time_t tLast=safe_mktime(last.GetLocalTm(&tmTemp));
		time_t tNow=safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
		if ( (difftime(tNow,tLast) / 86400)<thePrefs.GetUpdateDays() )
			return;
	}
	// ==> Removed Xtreme version check [Stulle] - Stulle
	/*
	if (WSAAsyncGetHostByName(m_hWnd, UM_MVERSIONCHECK_RESPONSE, "xtreme.dyndns.info", m_acMVCDNSBuffer, sizeof(m_acMVCDNSBuffer)) == 0){
		AddLogLine(true,GetResString(IDS_NEWVERSIONFAILED));
	}
	//Xman DLP //MOD NOTE: if you are using DLP, don't remove/modify this versions-check
	else if (WSAAsyncGetHostByName(m_hWnd, UM_DLPVERSIONCHECK_RESPONSE, "dlp.dyndns.info", m_acDLPBuffer, sizeof(m_acDLPBuffer)) == 0){
		AddLogLine(true,_T("DLP version check failed"));
	}
	//Xman End
	*/
	; // Do nothing
	// <== Removed Xtreme version check [Stulle] - Stulle
}
//Xman end

void CALLBACK CemuleDlg::StartupTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/)
{
	//Xman
	// SLUGFILLER: doubleLucas - not ready to init, come back next cycle
	if (!::IsWindow(theApp.emuledlg->m_hWnd))
		return;
	if (!::IsWindow(theApp.emuledlg->sharedfileswnd->sharedfilesctrl.m_hWnd))
		return;
	if (!::IsWindow(theApp.emuledlg->serverwnd->serverlistctrl.m_hWnd))
		return;
	if (!::IsWindow(theApp.emuledlg->transferwnd->downloadlistctrl.m_hWnd))
		return;
	//Xman end
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	try
	{
		switch(theApp.emuledlg->status){
			case 0:
				theApp.emuledlg->status++;
				theApp.emuledlg->ready = true;
				//Xman
				// SLUGFILLER: SafeHash remove - moved down
				/*
				theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
				*/
				//Xman end
				theApp.emuledlg->status++;
				break;
			case 1:
				break;
			case 2:
				theApp.emuledlg->status++;
				try{
					theApp.serverlist->Init();
				}
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize server list - Unknown exception"));
				}
				theApp.emuledlg->status++;
				break;
			case 3:
				break;
			case 4:{
				bool bError = false;
				theApp.emuledlg->status++;
				theApp.UpdateSplash(_T("initializing  files to download ...")); //Xman new slpash-screen arrangement

				// NOTE: If we have an unhandled exception in CDownloadQueue::Init, MFC will silently catch it
				// and the creation of the TCP and the UDP socket will not be done -> client will get a LowID!
				try{
					theApp.downloadqueue->Init();
				}
				catch(...){
					ASSERT(0);
					LogError(LOG_STATUSBAR,_T("Failed to initialize download queue - Unknown exception"));
					bError = true;
				}
				if (!theApp.listensocket->StartListening()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
					bError = true;
				}
				if (!theApp.clientudp->Create()) {
					CString strError;
					strError.Format(GetResString(IDS_MAIN_SOCKETERROR), thePrefs.GetUDPPort());
					LogError(LOG_STATUSBAR, _T("%s"), strError);
					if (thePrefs.GetNotifierOnImportantError())
						theApp.emuledlg->ShowNotifier(strError, TBN_IMPORTANTEVENT);
				}
				
				// ==> Random Ports [MoNKi] - Stulle
				theApp.emuledlg->serverwnd->UpdateMyInfo();
				// <== Random Ports [MoNKi] - Stulle
				if (!bError) // show the success msg, only if we had no serious error
				{ //Xman
					//<<< eWombat [WINSOCK2] for Pawcio: BC
					AddLogLine(false,_T("***************Winsock***************"));
					AddLogLine(false,_T("Winsock: Version %d.%d [%.40s] %.40s"), HIBYTE( theApp.m_wsaData.wVersion ),LOBYTE(theApp.m_wsaData.wVersion ),
						CString(theApp.m_wsaData.szDescription), CString(theApp.m_wsaData.szSystemStatus));
					if (theApp.m_wsaData.iMaxSockets!=0)
						AddLogLine(false,_T("Winsock: max. sockets %d"), theApp.m_wsaData.iMaxSockets);
					else
						AddLogLine(false,_T("Winsock: unlimited sockets"));
					AddLogLine(false,_T("***************Winsock***************"));
					//>>> eWombat [WINSOCK2]
					// XMan // Maella -Support for tag ET_MOD_VERSION 0x55
					/*
					AddLogLine(true, GetResString(IDS_MAIN_READY), theApp.m_strCurVersionLong);
					*/
					// ==> ModID [itsonlyme/SiRoB] - Stulle
					/*
					AddLogLine(true, GetResString(IDS_MAIN_READY),theApp.m_strCurVersionLong + _T(" ") + MOD_VERSION);
					*/
					AddLogLine(true, GetResString(IDS_MAIN_READY),theApp.m_strCurVersionLong + _T(" ") + theApp.m_strModLongVersion);
					// <== ModID [itsonlyme/SiRoB] - Stulle
					//Xman end

				//Xman
				}
				// SLUGFILLER: SafeHash remove - moved down
				/*
				if (thePrefs.DoAutoConnect())
					theApp.emuledlg->OnBnClickedButton2();
				*/
				//Xman end
				break;
			}
			case 5:
				if (thePrefs.IsStoringSearchesEnabled())
					theApp.searchlist->LoadSearches();
				theApp.emuledlg->status++;
				break;
			//Xman
			// BEGIN SLUGFILLER: SafeHash - delay load shared files
			case 6:
				theApp.emuledlg->status++;
				theApp.sharedfiles->SetOutputCtrl(&theApp.emuledlg->sharedfileswnd->sharedfilesctrl);
				theApp.emuledlg->sharedfileswnd->historylistctrl.Init(); //Xman [MoNKi: -Downloaded History-]

				// BEGIN SiRoB: SafeHash fix originaly in OnInitDialog (delay load shared files)
				// start aichsyncthread
				theApp.UpdateSplash(_T("Synchronize AICH-Hashes...")); //Xman new slpash-screen arrangement
				AfxBeginThread(RUNTIME_CLASS(CAICHSyncThread), THREAD_PRIORITY_BELOW_NORMAL,0);
				// END SiRoB: SafeHash
				theApp.emuledlg->status++;
				break;
			case 7:
				break;
			case 255:
				break;
			// END SLUGFILLER: SafeHash
			default:
			//Xman
			// BEGIN SLUGFILLER: SafeHash
				theApp.emuledlg->status = 255;
				theApp.UpdateSplash(_T("Ready")); //Xman new slpash-screen arrangement
				//autoconnect only after emule loaded completely
				if(thePrefs.DoAutoConnect())
					theApp.emuledlg->OnBnClickedButton2();
				// wait until emule is ready before opening the wizard
				if (thePrefs.IsFirstStart())
				{
					//Xman new slpash-screen arrangement
					//DestroySplash();
					theApp.DestroySplash();
					//Xman end
					if (FirstTimeWizard()){
						// start connection wizard
						CConnectionWizardDlg conWizard;
						conWizard.DoModal();
					}
				}
			// END SLUGFILLER: SafeHash
				theApp.emuledlg->StopTimer();
		}
	}
	CATCH_DFLT_EXCEPTIONS(_T("CemuleDlg::StartupTimer"))
		// Maella -Code Improvement-
		// Remark: The macro CATCH_DFLT_EXCEPTIONS will not catch all types of exception.
		//         The exceptions thrown in callback function are not intercepted by the dbghelp.dll (e.g. eMule Dump, crashRpt, etc...)
		catch(...) {
			ASSERT( false ); //zz_fly :: let me know it
			if(theApp.emuledlg != NULL)
				AddLogLine(true, _T("Unknown %s exception in "), __FUNCTION__);
		}
		// Maella end

}

void CemuleDlg::StopTimer()
{
	if (m_hTimer){
		VERIFY( ::KillTimer(NULL, m_hTimer) );
		m_hTimer = 0;
	}
	
	theApp.spashscreenfinished=true; //Xman new slpash-screen arrangement
	
	if (thePrefs.UpdateNotify())
		DoVersioncheck(false);
	
	//MOD NOTE: if you are using DLP, don't remove/modify this versions-check
	//Xman versions check
	if (thePrefs.UpdateNotifyMod())
		DoMVersioncheck(false);
	//Xman end

	// ==> ScarAngel Version Check - Stulle
	if (thePrefs.UpdateNotify())
		DoSVersioncheck(false);
	// <== ScarAngel Version Check - Stulle

	if (theApp.pstrPendingLink != NULL){
		OnWMData(NULL, (LPARAM)&theApp.sendstruct);
		delete theApp.pstrPendingLink;
	}
}

void CemuleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Systemmenu-Speedselector
	if (nID >= MP_QS_U10 && nID <= MP_QS_UP10) {
		QuickSpeedUpload(nID);
		return;
	}
	if (nID >= MP_QS_D10 && nID <= MP_QS_DC) {
		QuickSpeedDownload(nID);
		return;
	}
	if (nID == MP_QS_PA || nID == MP_QS_UA) {
		QuickSpeedOther(nID);
		return;
	}
	
	switch (nID /*& 0xFFF0*/)
	{
		case MP_ABOUTBOX: {
			CCreditsDlg dlgAbout;
			dlgAbout.DoModal();
			break;
		}
		case MP_VERSIONCHECK:
			DoVersioncheck(true);
			break;

	//Xman versions check
	case MP_MVERSIONCHECK:
		DoMVersioncheck(true);
		break;
	//Xman end

		// ==> ScarAngel Version Check - Stulle
		case MP_SVERSIONCHECK:
			DoSVersioncheck(true);
			break;
		// <== ScarAngel Version Check - Stulle

		case MP_CONNECT:
			StartConnection();
			break;
		case MP_DISCONNECT:
			CloseConnection();
			break;
		default:
			CTrayDialog::OnSysCommand(nID, lParam);
	}

	if ((nID & 0xFFF0) == SC_MINIMIZE		||
		(nID & 0xFFF0) == MP_MINIMIZETOTRAY	||
		(nID & 0xFFF0) == SC_RESTORE		||
		(nID & 0xFFF0) == SC_MAXIMIZE)
	{
		ShowTransferRate(true);
		ShowPing();
		transferwnd->UpdateCatTabTitles();
	}
}

void CemuleDlg::PostStartupMinimized()
{
	if (!m_bStartMinimizedChecked)
	{
		//TODO: Use full initialized 'WINDOWPLACEMENT' and remove the 'OnCancel' call...
		// Isn't that easy.. Read comments in OnInitDialog..
		m_bStartMinimizedChecked = true;
		if (m_bStartMinimized)
		{
			if (theApp.DidWeAutoStart())
			{
				if (!thePrefs.mintotray) {
					thePrefs.mintotray = true;
					MinimizeWindow();
					thePrefs.mintotray = false;
				}
				else
					MinimizeWindow();
			}
			else
				MinimizeWindow();
		}
	}
}

void CemuleDlg::OnPaint()
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
		CTrayDialog::OnPaint();
}

HCURSOR CemuleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CemuleDlg::OnBnClickedButton2(){
	if (!theApp.IsConnected())
		//connect if not currently connected
		if (!theApp.serverconnect->IsConnecting() && !Kademlia::CKademlia::IsRunning() ){
			StartConnection();
		}
		else {
			CloseConnection();
		}
	else{
		//disconnect if currently connected
		CloseConnection();
	}
}

void CemuleDlg::ResetServerInfo(){
	serverwnd->servermsgbox->Reset();
}
void CemuleDlg::ResetLog(){
	serverwnd->logbox->Reset();
}

void CemuleDlg::ResetDebugLog(){
	serverwnd->debuglog->Reset();
}

//Xman Anti-Leecher-Log
void CemuleDlg::ResetLeecherLog()
{
	serverwnd->leecherlog->Reset();
}
//Xman end

void CemuleDlg::AddLogText(UINT uFlags, LPCTSTR pszText)
{
	if (GetCurrentThreadId() != g_uMainThreadId)
	{
		theApp.QueueLogLineEx(uFlags, _T("%s"), pszText);
		return;
	}

	if (uFlags & LOG_STATUSBAR)
	{
        if (statusbar->m_hWnd /*&& ready*/)
		{
			if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN)
				statusbar->SetText(pszText, SBarLog, 0);
		}
		else
		{ //Xman
			theApp.DestroySplash(); //Xman new slpash-screen arrangement
			AfxMessageBox(pszText);
		} //Xman
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	Debug(_T("%s\n"), pszText);
#endif

	if ((uFlags & LOG_DEBUG) && !thePrefs.GetVerbose())
		return;

	//Xman Anti-Leecher-Log
	if ((uFlags & LOG_LEECHER) && !thePrefs.GetVerbose())
		return;


	TCHAR temp[1060];
	int iLen = _sntprintf(temp, _countof(temp), _T("%s: %s\r\n"), CTime::GetCurrentTime().Format(thePrefs.GetDateTimeFormat4Log()), pszText);
	if (iLen >= 0)
	{
		//Xman Anti-Leecher-Log
		/*
		if (!(uFlags & LOG_DEBUG))
		*/
		if (!(uFlags & LOG_DEBUG) && !(uFlags & LOG_LEECHER))
		//Xman end
		{
			serverwnd->logbox->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLog, TRUE);
			if (!(uFlags & LOG_DONTNOTIFY) && ready)
				ShowNotifier(pszText, TBN_LOG);
			if (thePrefs.GetLog2Disk())
				theLog.Log(temp, iLen);
		}
		//Xman Anti-Leecher-Log
		else
		if (thePrefs.GetVerbose() && (uFlags & LOG_LEECHER) )
		{
			serverwnd->leecherlog->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneLeecherLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneLeecherLog, TRUE);

			if (thePrefs.GetDebug2Disk())
				theVerboseLog.Log(temp, iLen);
		}
		else
		//Xman end

		if (thePrefs.GetVerbose() && ((uFlags & LOG_DEBUG) || thePrefs.GetFullVerbose()))
		{
			serverwnd->debuglog->AddTyped(temp, iLen, uFlags & LOGMSGTYPEMASK);
			if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneVerboseLog)
				serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneVerboseLog, TRUE);

			if (thePrefs.GetDebug2Disk())
				theVerboseLog.Log(temp, iLen);
		}
	}
}

CString CemuleDlg::GetLastLogEntry()
{
	return serverwnd->logbox->GetLastLogEntry();
}

CString CemuleDlg::GetAllLogEntries()
{
	return serverwnd->logbox->GetAllLogEntries();
}

CString CemuleDlg::GetLastDebugLogEntry()
{
	return serverwnd->debuglog->GetLastLogEntry();
}

CString CemuleDlg::GetAllDebugLogEntries()
{
	return serverwnd->debuglog->GetAllLogEntries();
}
CString CemuleDlg::GetServerInfoText()
{
	return serverwnd->servermsgbox->GetText();
}

void CemuleDlg::AddServerMessageLine(UINT uFlags, LPCTSTR pszLine)
{
	CString strMsgLine(pszLine);
	strMsgLine += _T('\n');
	if ((uFlags & LOGMSGTYPEMASK) == LOG_INFO)
		serverwnd->servermsgbox->AppendText(strMsgLine);
	else
		serverwnd->servermsgbox->AddTyped(strMsgLine, strMsgLine.GetLength(), uFlags & LOGMSGTYPEMASK);
	if (IsWindow(serverwnd->StatusSelector) && serverwnd->StatusSelector.GetCurSel() != CServerWnd::PaneServerInfo)
		serverwnd->StatusSelector.HighlightItem(CServerWnd::PaneServerInfo, TRUE);
}

UINT CemuleDlg::GetConnectionStateIconIndex() const
{
	if (theApp.serverconnect->IsConnected() && !Kademlia::CKademlia::IsConnected())
	{
		if (theApp.serverconnect->IsLowID())
			return 3; // LowNot
		else
			return 6; // HighNot
	}
	else if (!theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	{
		if (Kademlia::CKademlia::IsFirewalled())
			return 1; // NotLow
		else
			return 2; // NotHigh
	}
	else if (theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsConnected())
	{
		if (theApp.serverconnect->IsLowID() && Kademlia::CKademlia::IsFirewalled())
			return 4; // LowLow
		else if (theApp.serverconnect->IsLowID())
			return 5; // LowHigh
		else if (Kademlia::CKademlia::IsFirewalled())
			return 7; // HighLow
		else
			return 8; // HighHigh
	}
	else
	{
		return 0; // NotNot
	}
}

void CemuleDlg::ShowConnectionStateIcon()
{
	UINT uIconIdx = GetConnectionStateIconIndex();
	if (uIconIdx >= _countof(connicons)){
		ASSERT(0);
		uIconIdx = 0;
	}
	statusbar->SetIcon(SBarConnected, connicons[uIconIdx]);
}

CString CemuleDlg::GetConnectionStateString()
{
	CString status;
	if (theApp.serverconnect->IsConnected())
		status = _T("eD2K:") + GetResString(IDS_CONNECTED);
	else if (theApp.serverconnect->IsConnecting())
		status = _T("eD2K:") + GetResString(IDS_CONNECTING);
	else
		status = _T("eD2K:") + GetResString(IDS_NOTCONNECTED);

	if (Kademlia::CKademlia::IsConnected())
		status += _T("|Kad:") + GetResString(IDS_CONNECTED);
	else if (Kademlia::CKademlia::IsRunning())
		status += _T("|Kad:") + GetResString(IDS_CONNECTING);
	else
		status += _T("|Kad:") + GetResString(IDS_NOTCONNECTED);
	return status;
}

void CemuleDlg::ShowConnectionState()
{
	theApp.downloadqueue->OnConnectionState(theApp.IsConnected());
	serverwnd->UpdateMyInfo();
	serverwnd->UpdateControlsState();
	kademliawnd->UpdateControlsState();

	ShowConnectionStateIcon();
	statusbar->SetText(GetConnectionStateString(), SBarConnected, 0);

	if (theApp.IsConnected())
	{
		CString strPane(GetResString(IDS_MAIN_BTN_DISCONNECT));
		TBBUTTONINFO tbi;
		tbi.cbSize = sizeof(TBBUTTONINFO);
		tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
		tbi.iImage = 1;
		tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
		toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
	}
	else
	{
		if (theApp.serverconnect->IsConnecting() || Kademlia::CKademlia::IsRunning()) 
		{
			CString strPane(GetResString(IDS_MAIN_BTN_CANCEL));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 2;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
		} 
		else 
		{
			CString strPane(GetResString(IDS_MAIN_BTN_CONNECT));
			TBBUTTONINFO tbi;
			tbi.cbSize = sizeof(TBBUTTONINFO);
			tbi.dwMask = TBIF_IMAGE | TBIF_TEXT;
			tbi.iImage = 0;
			tbi.pszText = const_cast<LPTSTR>((LPCTSTR)strPane);
			toolbar->SetButtonInfo(TBBTN_CONNECT, &tbi);
		}
	}
	ShowUserCount();
}

void CemuleDlg::ShowUserCount()
{
	uint32 totaluser, totalfile;
	totaluser = totalfile = 0;
	theApp.serverlist->GetUserFileStatus( totaluser, totalfile );
	CString buffer;
	if (theApp.serverconnect->IsConnected() && Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsConnected())
		buffer.Format(_T("%s:%s(%s)|%s:%s(%s)"), GetResString(IDS_UUSERS), CastItoIShort(totaluser, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(), false, 1), GetResString(IDS_FILES), CastItoIShort(totalfile, false, 1), CastItoIShort(Kademlia::CKademlia::GetKademliaFiles(), false, 1));
	else if (theApp.serverconnect->IsConnected())
		buffer.Format(_T("%s:%s|%s:%s"), GetResString(IDS_UUSERS), CastItoIShort(totaluser, false, 1), GetResString(IDS_FILES), CastItoIShort(totalfile, false, 1));
	else if (Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsConnected())
		buffer.Format(_T("%s:%s|%s:%s"), GetResString(IDS_UUSERS), CastItoIShort(Kademlia::CKademlia::GetKademliaUsers(), false, 1), GetResString(IDS_FILES), CastItoIShort(Kademlia::CKademlia::GetKademliaFiles(), false, 1));
	else
		buffer.Format(_T("%s:0|%s:0"), GetResString(IDS_UUSERS), GetResString(IDS_FILES));
	statusbar->SetText(buffer, SBarUsers, 0);
}

void CemuleDlg::ShowMessageState(UINT iconnr)
{
	m_iMsgIcon = iconnr;
	statusbar->SetIcon(SBarChatMsg, imicons[m_iMsgIcon]);
}

void CemuleDlg::ShowTransferStateIcon()
{
	if (m_uUpDatarate && m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[3]);
	else if (m_uUpDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[2]);
	else if (m_uDownDatarate)
		statusbar->SetIcon(SBarUpDown, transicons[1]);
	else
		statusbar->SetIcon(SBarUpDown, transicons[0]);
}

//Xman Code Improvement
//it's enough to update the datarate in ShowTransferrate
/*
CString CemuleDlg::GetUpDatarateString(UINT uUpDatarate)
{
	m_uUpDatarate = uUpDatarate != (UINT)-1 ? uUpDatarate : theApp.uploadqueue->GetDatarate();
*/
CString CemuleDlg::GetUpDatarateString(UINT /*uUpDatarate*/)
{
//Xman end
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead()) {
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uUpDatarate/1024, (float)theStats.GetUpDatarateOverhead()/1024);
		*/
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024);
		//Xman end
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else {
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f"), (float)m_uUpDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

//Xman Code Improvement
//it's enough to update the datarate in ShowTransferrate
/*
CString CemuleDlg::GetDownDatarateString(UINT uDownDatarate)
{
	m_uDownDatarate = uDownDatarate != (UINT)-1 ? uDownDatarate : theApp.downloadqueue->GetDatarate();
*/
CString CemuleDlg::GetDownDatarateString(UINT /*uDownDatarate*/)
{
//Xman end

	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead()) {
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uDownDatarate/1024, (float)theStats.GetDownDatarateOverhead()/1024);
		*/
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f (%.1f)"), (float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024);
		//Xman end
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else {
		_sntprintf(szBuff, _countof(szBuff), _T("%.1f"), (float)m_uDownDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

CString CemuleDlg::GetTransferRateString()
{
	TCHAR szBuff[128];
	if (thePrefs.ShowOverhead()) {
		_sntprintf(szBuff, _countof(szBuff), GetResString(IDS_UPDOWN),
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
			       (float)m_uUpDatarate/1024, (float)theStats.GetUpDatarateOverhead()/1024,
			       (float)m_uDownDatarate/1024, (float)theStats.GetDownDatarateOverhead()/1024);
		*/
		(float)m_uUpDatarate/1024, (float)m_uploadOverheadRate/1024,
		(float)m_uDownDatarate/1024, (float)m_downloadOverheadRate/1024);
		//Xman end
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	else {
		_sntprintf(szBuff, _countof(szBuff), GetResString(IDS_UPDOWNSMALL), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024);
		szBuff[_countof(szBuff) - 1] = _T('\0');
	}
	return szBuff;
}

void CemuleDlg::ShowTransferRate(bool bForceAll)
{
	if (bForceAll)
		m_uLastSysTrayIconCookie = SYS_TRAY_ICON_COOKIE_FORCE_UPDATE;

	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	/*
	m_uDownDatarate = theApp.downloadqueue->GetDatarate();
	m_uUpDatarate = theApp.uploadqueue->GetDatarate();
	*/
	// Retrieve the current datarates
	uint32 eMuleIn;	uint32 eMuleInOverall;
	uint32 eMuleOut; uint32 eMuleOutOverall;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
										   eMuleIn, eMuleInOverall,
										   eMuleOut, eMuleOutOverall,
										   notUsed, notUsed);

	
	m_uDownDatarate = eMuleIn;
	m_uUpDatarate = eMuleOut;
	m_uploadOverheadRate=eMuleOutOverall-eMuleOut;
	m_downloadOverheadRate=eMuleInOverall-eMuleIn;
	//Xman end

	// ==> Show in MSN7 [TPT] - Stulle
	const float uploadRate = (float)m_uUpDatarate/1024;
	const float downloadRate = (float)m_uDownDatarate/1024;
	const float uploadOverheadRate = (float)m_uploadOverheadRate/1024;
	const float downloadOverheadRate = (float)m_downloadOverheadRate/1024;
	// <== Show in MSN7 [TPT] - Stulle

	// ==> Completed in Tray [Stulle] - Stulle
	int iTotal;
	if (thePrefs.GetTrayComplete() && ::GetTickCount() - m_dwTrayTime > SEC2MS(10))
	{
		m_bTrayBool = (theApp.emuledlg->transferwnd->downloadlistctrl.GetCompleteDownloads(-1, iTotal) > 0);
		m_dwTrayTime = ::GetTickCount();
	}
	// <== Completed in Tray [Stulle] - Stulle

	CString strTransferRate = GetTransferRateString();
	if (TrayIsVisible() || bForceAll)
	{
		TCHAR buffer2[64];
		// set trayicon-icon
		int iDownRateProcent = (int)ceil((m_uDownDatarate/10.24) / thePrefs.GetMaxGraphDownloadRate());
		if (iDownRateProcent > 100)
			iDownRateProcent = 100;
		UpdateTrayIcon(iDownRateProcent);

		if (theApp.IsConnected()) {
			_sntprintf(buffer2, _countof(buffer2), _T("eMule v%s (%s)\r\n%s"), theApp.m_strCurVersionLong, GetResString(IDS_CONNECTED), strTransferRate);
			buffer2[_countof(buffer2) - 1] = _T('\0');
		}
		else {
			_sntprintf(buffer2, _countof(buffer2), _T("eMule v%s (%s)\r\n%s"), theApp.m_strCurVersionLong, GetResString(IDS_DISCONNECTED), strTransferRate);
			buffer2[_countof(buffer2) - 1] = _T('\0');
		}

		// ==> Drop Win95 support [MorphXT] - Stulle
		/*
		// Win98: '\r\n' is not displayed correctly in tooltip
		if (afxIsWin95()) {
			LPTSTR psz = buffer2;
			while (*psz) {
				if (*psz == _T('\r') || *psz == _T('\n'))
					*psz = _T(' ');
				psz++;
			}
		}
		*/
		// <== Drop Win95 support [MorphXT] - Stulle
		TraySetToolTip(buffer2);
	}

	//Xman see all sources
	if (activewnd == transferwnd && IsWindowVisible()){
		transferwnd->downloadlistctrl.ShowFilesCount();
	}
	//Xman end


	if (IsWindowVisible() || bForceAll)
	{
		//Xman GlobalMaxHarlimit for fairness
		if(theApp.downloadqueue->GetLimitState()==1)
			strTransferRate.Append(_T(" r"));
		else if(theApp.downloadqueue->GetLimitState()>=2)
			strTransferRate.Append(_T(" R"));
		//Xman end
		statusbar->SetText(strTransferRate, SBarUpDown, 0);
		ShowTransferStateIcon();
	}

	// ==> Show sources on title - Stulle
	// ==> show overhead on title - Stulle
	/*
	if (IsWindowVisible() && thePrefs.ShowRatesOnTitle())
	{
		TCHAR szBuff[128];
		// Maella -Support for tag ET_MOD_VERSION 0x55
		/*
		_sntprintf(szBuff, _countof(szBuff), _T("(U:%.1f D:%.1f) eMule v%s"), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024, theApp.m_strCurVersionLong);
		*//*
		_sntprintf(szBuff, _countof(szBuff), _T("(U:%.1f D:%.1f) eMule v%s"), (float)m_uUpDatarate/1024, (float)m_uDownDatarate/1024, theApp.m_strCurVersionLong + _T(" ") + MOD_VERSION);
		//Xman end
		szBuff[_countof(szBuff) - 1] = _T('\0');
		SetWindowText(szBuff);
	}
	*/
	uint16 m_uGlobalSources = theApp.downloadqueue->GetGlobalSourceCount();

	if (IsWindowVisible() && thePrefs.ShowRatesOnTitle() && thePrefs.ShowSrcOnTitle())
	{
		TCHAR szBuff[128];
		if (thePrefs.ShowOverheadOnTitle())
		{
			_sntprintf(szBuff,_countof(szBuff),_T("(U:%.1f(%.1f) D:%.1f(%.1f)) eMule v%s [%s] --> %s %u"),uploadRate, uploadOverheadRate, downloadRate, downloadOverheadRate, theApp.m_strCurVersionLong,theApp.m_strModLongVersion,GetResString(IDS_SHOWSRCSTRING),m_uGlobalSources);
			szBuff[_countof(szBuff) - 1] = _T('\0');
		}
		else
		{
			_sntprintf(szBuff,_countof(szBuff),_T("(U:%.1f D:%.1f) eMule v%s [%s] --> %s %u"),uploadRate, downloadRate, theApp.m_strCurVersionLong,theApp.m_strModLongVersion,GetResString(IDS_SHOWSRCSTRING),m_uGlobalSources);
			szBuff[_countof(szBuff) - 1] = _T('\0');
		}
		SetWindowText(szBuff);
	}
	else if (IsWindowVisible() && thePrefs.ShowRatesOnTitle())
	{
		TCHAR szBuff[128];
		if (thePrefs.ShowOverheadOnTitle())
		{
			_sntprintf(szBuff,_countof(szBuff),_T("(U:%.1f(%.1f) D:%.1f(%.1f)) eMule v%s [%s]"),uploadRate, uploadOverheadRate, downloadRate, downloadOverheadRate, theApp.m_strCurVersionLong,theApp.m_strModLongVersion);
			szBuff[_countof(szBuff) - 1] = _T('\0');
		}
		else
		{
			_sntprintf(szBuff,_countof(szBuff),_T("(U:%.1f D:%.1f) eMule v%s [%s]"),uploadRate, downloadRate, theApp.m_strCurVersionLong,theApp.m_strModLongVersion);
			szBuff[_countof(szBuff) - 1] = _T('\0');
		}
		SetWindowText(szBuff);
	}
	// <== show overhead on title - Stulle
	else if (IsWindowVisible() && thePrefs.ShowSrcOnTitle())
	{
		TCHAR szBuff[128];
		_sntprintf(szBuff,_countof(szBuff),_T("eMule v%s [%s] --> %s %u"),theApp.m_strCurVersionLong,theApp.m_strModLongVersion,GetResString(IDS_SHOWSRCSTRING),m_uGlobalSources);
		szBuff[_countof(szBuff) - 1] = _T('\0');
		SetWindowText(szBuff);
	}
	// <== Show sources on title - Stulle

	if (m_pMiniMule && m_pMiniMule->m_hWnd && m_pMiniMule->IsWindowVisible() && !m_pMiniMule->GetAutoClose())
	{
		m_pMiniMule->UpdateContent(m_uUpDatarate, m_uDownDatarate);
	}

	// ==> Show in MSN7 [TPT] - Stulle
	//Only update every 10 seconds and if enabled
	if(thePrefs.GetShowMSN7())
	{
		if (m_uMSNup2Date == 0 && ::GetTickCount() - m_dwMSNtime > SEC2MS(10))
		{
			UpdateMSN(uploadRate, uploadOverheadRate, downloadRate, downloadOverheadRate);
			m_uMSNup2Date = 1;
		}
		else if (m_uMSNup2Date == 1 && ::GetTickCount() - m_dwMSNtime > SEC2MS(20))
		{
			UpdateMSN1(theApp.pBandWidthControl->GeteMuleIn(), theApp.pBandWidthControl->GeteMuleOut(),m_uGlobalSources);
			m_uMSNup2Date = 2;
		}
		else if (m_uMSNup2Date == 2 && ::GetTickCount() - m_dwMSNtime > SEC2MS(30))
		{
			CString buffer = _T("");
			if(Kademlia::CKademlia::IsConnected())
				buffer.AppendFormat(_T("KAD"));
			if(Kademlia::CKademlia::IsConnected() && theApp.serverconnect->IsConnected())
				buffer.AppendFormat(_T(" + "));
			else if (!Kademlia::CKademlia::IsConnected() && !theApp.serverconnect->IsConnected())
				buffer.AppendFormat(_T("Not Connected"));
			if(theApp.serverconnect->IsConnected())
				buffer.AppendFormat(_T("%s"), theApp.serverconnect->GetCurrentServer()->GetListName());
			UpdateMSN2(buffer);
			m_uMSNup2Date = 0;
			m_dwMSNtime = ::GetTickCount();
		}
	}
	// <== Show in MSN7 [TPT] - Stulle
}

void CemuleDlg::ShowPing()
{
	//Xman
	/*
    if (IsWindowVisible())
	{
        CString buffer;
        if (thePrefs.IsDynUpEnabled())
		{
			CurrentPingStruct lastPing = theApp.lastCommonRouteFinder->GetCurrentPing();
            if (lastPing.state.GetLength() == 0)
			{
                if (lastPing.lowest > 0 && !thePrefs.IsDynUpUseMillisecondPingTolerance())
                    buffer.Format(_T("%.1f | %ims | %i%%"),lastPing.currentLimit/1024.0f, lastPing.latency, lastPing.latency*100/lastPing.lowest);
                else
                    buffer.Format(_T("%.1f | %ims"),lastPing.currentLimit/1024.0f, lastPing.latency);
            }
			else
                buffer.SetString(lastPing.state);
        }
		statusbar->SetText(buffer, SBarChatMsg, 0);
    }
	*/
	//Xman end
}

void CemuleDlg::OnOK()
{
}

void CemuleDlg::OnCancel()
{
	if (!thePrefs.GetStraightWindowStyles())
		MinimizeWindow();
}

void CemuleDlg::MinimizeWindow()
{
	if (*thePrefs.GetMinTrayPTR())
	{
		// ==> Static Tray Icon [MorphXT] - MyTh88
		m_bMaximized = IsZoomed();
		// <== Static Tray Icon [MorphXT] - MyTh88
		TrayShow();
		ShowWindow(SW_HIDE);

		// ==> Invisible Mode [TPT/MoNKi] - Stulle
		if (thePrefs.GetInvisibleMode() && (theApp.DidWeAutoStart() || thePrefs.GetInvisibleModeStart()))
			ToggleHide();
		// <== Invisible Mode [TPT/MoNKi] - Stulle
	}
	else
	{
		ShowWindow(SW_MINIMIZE);
	}
	ShowTransferRate();
	ShowPing();
}

void CemuleDlg::SetActiveDialog(CWnd* dlg)
{
	if (dlg == activewnd)
		return;
	if (activewnd)
		activewnd->ShowWindow(SW_HIDE);
	dlg->ShowWindow(SW_SHOW);
	dlg->SetFocus();
	activewnd = dlg;
	int iToolbarButtonID = MapWindowToToolbarButton(dlg);
	if (iToolbarButtonID != -1)
		toolbar->PressMuleButton(iToolbarButtonID);
	if (dlg == transferwnd){
		if (thePrefs.ShowCatTabInfos())
			transferwnd->UpdateCatTabTitles();
	}
	else if (dlg == chatwnd){
		chatwnd->chatselector.ShowChat();
	}
	else if (dlg == statisticswnd){
		statisticswnd->ShowStatistics();
	}
}

void CemuleDlg::SetStatusBarPartsSize()
{
	CRect rect;
	statusbar->GetClientRect(&rect);
	int ussShift = 0;
	
	//Xman
	//only used for with USS
	/*
	if(thePrefs.IsDynUpEnabled())
	{
        if (thePrefs.IsDynUpUseMillisecondPingTolerance())
            ussShift = 45;
        else
            ussShift = 90;
	}
	*/
	//Xman end
	
	//Xman changed
	/*
	int aiWidths[5] =
	{ 
		rect.right - 675 - ussShift,
		rect.right - 440 - ussShift,
		rect.right - 250 - ussShift,
		rect.right -  25 - ussShift,
		-1
	};
	*/
	int aiWidths[5] =
	{ 
		rect.right - 675 - ussShift,
			rect.right - 440 - ussShift,
			rect.right - 235 - ussShift,
			rect.right -  25 - ussShift,
			-1
	};
	//Xman end

	statusbar->SetParts(_countof(aiWidths), aiWidths);
}

void CemuleDlg::OnSize(UINT nType, int cx, int cy)
{
	CTrayDialog::OnSize(nType, cx, cy);
	SetStatusBarPartsSize();
	transferwnd->VerifyCatTabSize();
}

void CemuleDlg::ProcessED2KLink(LPCTSTR pszData)
{
	try {
		CString link2;
		CString link;
		link2 = pszData;
		link2.Replace(_T("%7c"),_T("|"));
		link = OptUtf8ToStr(URLDecode(link2));
		CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(link);
		_ASSERT( pLink !=0 );
		switch (pLink->GetKind()) {
		case CED2KLink::kFile:
			{
				// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
				/*
				CED2KFileLink* pFileLink = pLink->GetFileLink();
				_ASSERT(pFileLink !=0);
				//Xman [MoNKi: -Check already downloaded files-]
				if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(pFileLink->GetHashKey(),pFileLink->GetName()))
				{
					theApp.downloadqueue->AddFileLinkToDownload(pFileLink,searchwnd->GetSelectedCat());
				} //Xman end
				*/
				CED2KFileLink* pFileLink = (CED2KFileLink*)CED2KLink::CreateLinkFromUrl(link.Trim());
				_ASSERT(pFileLink !=0);
				if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(pFileLink->GetHashKey(),pFileLink->GetName()))
				{
					theApp.downloadqueue->AddFileLinkToDownload(pFileLink, -1, true);
				}
				// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
			}
			break;
		case CED2KLink::kServerList:
			{
				CED2KServerListLink* pListLink = pLink->GetServerListLink(); 
				_ASSERT( pListLink !=0 ); 
				CString strAddress = pListLink->GetAddress(); 
				if(strAddress.GetLength() != 0)
					serverwnd->UpdateServerMetFromURL(strAddress);
			}
			break;
		case CED2KLink::kNodesList:
			{
				CED2KNodesListLink* pListLink = pLink->GetNodesListLink(); 
				_ASSERT( pListLink !=0 ); 
				CString strAddress = pListLink->GetAddress();
				// Becasue the nodes.dat is vital for kad and its routing and doesn't needs to be updated in general
				// we request a confirm to avoid accidental / malicious updating of this file. This is a bit inconsitent
				// as the same kinda applies to the server.met, but those require more updates and are easier to understand
				CString strConfirm;
				strConfirm.Format(GetResString(IDS_CONFIRMNODESDOWNLOAD), strAddress);
				if(strAddress.GetLength() != 0 && AfxMessageBox(strConfirm, MB_YESNO | MB_ICONQUESTION, 0) == IDYES)
					kademliawnd->UpdateNodesDatFromURL(strAddress);
			}
			break;
		case CED2KLink::kServer:
			{
				CString defName;
				CED2KServerLink* pSrvLink = pLink->GetServerLink();
				_ASSERT( pSrvLink !=0 );
				CServer* pSrv = new CServer(pSrvLink->GetPort(), pSrvLink->GetAddress());
				_ASSERT( pSrv !=0 );
				pSrvLink->GetDefaultName(defName);
				pSrv->SetListName(defName);

				// Barry - Default all new servers to high priority
				if (thePrefs.GetManualAddedServersHighPriority())
					pSrv->SetPreference(SRV_PR_HIGH);

				if (!serverwnd->serverlistctrl.AddServer(pSrv,true)) 
					delete pSrv; 
				else
					AddLogLine(true,GetResString(IDS_SERVERADDED), pSrv->GetListName());
			}
			break;
			// MORPH START - Added by Commander, Friendlinks [emulEspaa] - added by zz_fly
			case CED2KLink::kFriend:
			{
				// Better with dynamic_cast, but no RTTI enabled in the project
				CED2KFriendLink* pFriendLink = static_cast<CED2KFriendLink*>(pLink);
				uchar userHash[16];
				pFriendLink->GetUserHash(userHash);

				if ( ! theApp.friendlist->IsAlreadyFriend(userHash) )
					theApp.friendlist->AddFriend(userHash, 0U, 0U, 0U, 0U, pFriendLink->GetUserName(), 1U);
				else
				{
					CString msg;
					msg.Format(GetResString(IDS_USER_ALREADY_FRIEND), pFriendLink->GetUserName());
					AddLogLine(true, msg);
				}
			}
			break;
			case CED2KLink::kFriendList:
			{
				// Better with dynamic_cast, but no RTTI enabled in the project
				CED2KFriendListLink* pFrndLstLink = static_cast<CED2KFriendListLink*>(pLink);
				CString sAddress = pFrndLstLink->GetAddress(); 
				if ( !sAddress.IsEmpty() )
					this->chatwnd->UpdateEmfriendsMetFromURL(sAddress);
			}
			break;
			// MORPH END - Added by Commander, Friendlinks [emulEspaa]
		default:
			break;
		}
		delete pLink;
	}
	catch(CString strError){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_LINKNOTADDED) + _T(" - ") + strError);
	}
	catch(...){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_LINKNOTADDED));
	}
}

LRESULT CemuleDlg::OnWMData(WPARAM /*wParam*/, LPARAM lParam)
{
	PCOPYDATASTRUCT data = (PCOPYDATASTRUCT)lParam;
	if (data->dwData == OP_ED2KLINK)
	{
		if (thePrefs.IsBringToFront())
		{
			FlashWindow(TRUE);
			if (IsIconic())
				ShowWindow(SW_SHOWNORMAL);
			// ==> Static Tray Icon [MorphXT] - MyTh88
			/*
			else if (TrayHide())
			*/
			else if (thePrefs.GetStaticIcon() || TrayHide())
			// <== Static Tray Icon [MorphXT] - MyTh88
				RestoreWindow();
			else
				SetForegroundWindow();
		}
		ProcessED2KLink((LPCTSTR)data->lpData);
	}
	else if(data->dwData == OP_COLLECTION){
		FlashWindow(TRUE);
		if (IsIconic())
			ShowWindow(SW_SHOWNORMAL);
		// ==> Static Tray Icon [MorphXT] - MyTh88
		/*
		else if (TrayHide())
		*/
		else if (thePrefs.GetStaticIcon() || TrayHide())
		// <== Static Tray Icon [MorphXT] - MyTh88
			RestoreWindow();
		else
			SetForegroundWindow();

		CCollection* pCollection = new CCollection();
		CString strPath = CString((LPCTSTR)data->lpData);
		if (pCollection->InitCollectionFromFile(strPath, strPath.Right((strPath.GetLength()-1)-strPath.ReverseFind('\\')))){
			CCollectionViewDialog dialog;
			dialog.SetCollection(pCollection);
			dialog.DoModal();
		}
		delete pCollection;
	}
	else if (data->dwData == OP_CLCOMMAND){
		// command line command received
		CString clcommand((LPCTSTR)data->lpData);
		clcommand.MakeLower();
		AddLogLine(true,_T("CLI: %s"),clcommand);

		if (clcommand==_T("connect")) {StartConnection(); return true;}
		if (clcommand==_T("disconnect")) {theApp.serverconnect->Disconnect(); return true;}
		if (clcommand==_T("resume")) {theApp.downloadqueue->StartNextFile(); return true;}
		//Xman do not ask exit from command prompt (leuk_he)
		/*
		if (clcommand==_T("exit")) {OnClose(); return true;}
		*/
		if (clcommand==_T("exit")) {theApp.m_app_state = APP_STATE_SHUTTINGDOWN;OnClose(); return true;}
		//Xman end
		if (clcommand==_T("restore")) {RestoreWindow();return true;}
		if (clcommand==_T("reloadipf")) {theApp.ipfilter->LoadFromDefaultFile(); return true;}
		if (clcommand.Left(7).MakeLower()==_T("limits=") && clcommand.GetLength()>8) {
			CString down;
			CString up=clcommand.Mid(7);
			int pos=up.Find(_T(','));
			if (pos>0) {
				down=up.Mid(pos+1);
				up=up.Left(pos);
			}
			//Xman
			// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
			/*
			if (down.GetLength()>0) thePrefs.SetMaxDownload(_tstoi(down));
			if (up.GetLength()>0) thePrefs.SetMaxUpload(_tstoi(up));
			*/
			if (down.GetLength()>0) thePrefs.SetMaxDownload((float)_tstof(down));
			if (up.GetLength()>0) thePrefs.SetMaxUpload((float)_tstof(up));
			//Xman Xtreme Upload:
			thePrefs.CheckSlotSpeed();
			//Xman end
			theApp.scheduler->SaveOriginals(); // Don't reset Connection Settings for Webserver/CML/MM [Stulle] - Stulle

			return true;
		}

		if (clcommand==_T("help") || clcommand==_T("/?")) {
			// show usage
			return true;
		}

		if (clcommand==_T("status")) {
			CString strBuff;
			strBuff.Format(_T("%sstatus.log"), thePrefs.GetMuleDirectory(EMULE_CONFIGBASEDIR));
			FILE* file = _tfsopen(strBuff, _T("wt"), _SH_DENYWR);
			if (file){
				if (theApp.serverconnect->IsConnected())
					strBuff = GetResString(IDS_CONNECTED);
				else if (theApp.serverconnect->IsConnecting())
					strBuff = GetResString(IDS_CONNECTING);
				else
					strBuff = GetResString(IDS_DISCONNECTED);
				_ftprintf(file, _T("%s\n"), strBuff);

				// Xman
				// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
				/*
				strBuff.Format(GetResString(IDS_UPDOWNSMALL), (float)theApp.uploadqueue->GetDatarate()/1024, (float)theApp.downloadqueue->GetDatarate()/1024);
				*/
				uint32 eMuleIn;
				uint32 eMuleOut;
				uint32 notUsed;
				theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
														eMuleIn, notUsed,
													   eMuleOut, notUsed,
													   notUsed, notUsed);
				

				strBuff.Format(GetResString(IDS_UPDOWNSMALL), (float)eMuleOut/1024, (float)eMuleIn/1024);
				// Maella end
				_ftprintf(file, _T("%s"), strBuff); // next string (getTextList) is already prefixed with '\n'!
				_ftprintf(file, _T("%s\n"), transferwnd->downloadlistctrl.getTextList());
				
				fclose(file);
			}
			return true;
		}
		// show "unknown command";
	}
	return true;
}

LRESULT CemuleDlg::OnFileHashed(WPARAM wParam, LPARAM lParam)
{
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	CKnownFile* result = (CKnownFile*)lParam;
	ASSERT( result->IsKindOf(RUNTIME_CLASS(CKnownFile)) );

	if (wParam)
	{
		// File hashing finished for a part file when:
		// - part file just completed
		// - part file was rehashed at startup because the file date of part.met did not match the part file date

		CPartFile* requester = (CPartFile*)wParam;
		ASSERT( requester->IsKindOf(RUNTIME_CLASS(CPartFile)) );

		// SLUGFILLER: SafeHash - could have been canceled
		if (theApp.downloadqueue->IsPartFile(requester))
			requester->PartFileHashFinished(result);
		else
			delete result;
		// SLUGFILLER: SafeHash
	}
	else
	{
		ASSERT( !result->IsKindOf(RUNTIME_CLASS(CPartFile)) );

		// File hashing finished for a shared file (none partfile)
		//	- reading shared directories at startup and hashing files which were not found in known.met
		//	- reading shared directories during runtime (user hit Reload button, added a shared directory, ...)
		theApp.sharedfiles->FileHashingFinished(result);
	}
	return TRUE;
}

LRESULT CemuleDlg::OnFileOpProgress(WPARAM wParam, LPARAM lParam)
{
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	CKnownFile* pKnownFile = (CKnownFile*)lParam;
	ASSERT( pKnownFile->IsKindOf(RUNTIME_CLASS(CKnownFile)) );

	if (pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile)))
	{
		CPartFile* pPartFile = static_cast<CPartFile*>(pKnownFile);
		pPartFile->SetFileOpProgress(wParam);
		pPartFile->UpdateDisplayedInfo(true);
	}

	return 0;
}

// SLUGFILLER: SafeHash
LRESULT CemuleDlg::OnHashFailed(WPARAM /*wParam*/, LPARAM lParam)
{
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN) {
		UnknownFile_Struct* hashed = (UnknownFile_Struct*)lParam;
		delete hashed;
		return FALSE;
	}
	// END SiRoB: Fix crash at shutdown
	theApp.sharedfiles->HashFailed((UnknownFile_Struct*)lParam);
	return 0;
}
// SLUGFILLER: SafeHash

//Xman
LRESULT CemuleDlg::OnPartHashedOK(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished((UINT)wParam, false);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedCorrupt(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinished((UINT)wParam, true);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedOKAICHRecover(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinishedAICHRecover((UINT)wParam, false);
	return 0;
}

LRESULT CemuleDlg::OnPartHashedCorruptAICHRecover(WPARAM wParam,LPARAM lParam)
{
	//Xman
	// BEGIN SiRoB: Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL)
		return FALSE;
	// END SiRoB: Fix crash at shutdown
	CPartFile* pOwner = (CPartFile*)lParam;
	if (theApp.downloadqueue->IsPartFile(pOwner))	// could have been canceled
		pOwner->PartHashFinishedAICHRecover((UINT)wParam, true);
	return 0;
}
// END SLUGFILLER: SafeHash

// BEGIN SiRoB: ReadBlockFromFileThread
LRESULT CemuleDlg::OnReadBlockFromFileDone(WPARAM wParam,LPARAM lParam)
{
	CUpDownClient* client = (CUpDownClient*) lParam;
	if (theApp.m_app_state == APP_STATE_RUNNING && theApp.uploadqueue->IsDownloading(client))	// could have been canceled
	{
		client->SetReadBlockFromFileBuffer((byte*)wParam);
		client->CreateNextBlockPackage(); //complete the process
	}
	else if (wParam != -1 && wParam != -2 && wParam != NULL)
		delete[] (byte*)wParam;
	return 0;
}
// END SiRoB: ReadBlockFromFileThread
// BEGIN SiRoB: Flush Thread
LRESULT CemuleDlg::OnFlushDone(WPARAM /*wParam*/,LPARAM lParam)
{
	ASSERT(!(theApp.m_app_state == APP_STATE_RUNNING && theApp.downloadqueue==NULL)); 

	CPartFile* partfile = (CPartFile*) lParam;
	if (theApp.m_app_state == APP_STATE_RUNNING && theApp.downloadqueue!=NULL && theApp.downloadqueue->IsPartFile(partfile))	// could have been canceled
		partfile->FlushDone();
	return 0;
}
// END SiRoB: Flush Thread
//Xman end

//MORPH START - Added by SiRoB, Import Parts - added by zz_fly
LRESULT CemuleDlg::OnImportPart(WPARAM wParam,LPARAM lParam)
{
	CPartFile* partfile = (CPartFile*) lParam;
	if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN && AfxIsValidAddress(partfile,sizeof(CPartFile)) &&  theApp.downloadqueue->IsPartFile(partfile)) {	// could have been canceled 
		ImportPart_Struct* importpart = (ImportPart_Struct*)wParam;
		partfile->WriteToBuffer(importpart->end-importpart->start+1, importpart->data,importpart->start, importpart->end, NULL, NULL);
	}
	delete[] ((ImportPart_Struct*)wParam)->data;
	delete	(ImportPart_Struct*)wParam;

	return 0;
}
//MORPH END   - Added by SiRoB, Import Parts

LRESULT CemuleDlg::OnFileAllocExc(WPARAM wParam,LPARAM lParam)
{
	//Xman
	//MORPH START - Added by SiRoB, Fix crash at shutdown
	
	ASSERT(!(theApp.m_app_state == APP_STATE_RUNNING && theApp.downloadqueue==NULL)); 

	CFileException* error = (CFileException*)lParam;
	if (theApp.m_app_state != APP_STATE_RUNNING || theApp.downloadqueue==NULL || !theApp.downloadqueue->IsPartFile((CPartFile*)wParam)) { //MORPH - Changed by SiRoB, Flush Thread
		if (error != NULL)
			error->Delete();
		return FALSE;
	}
	//MORPH END   - Added by SiRoB, Fix crash at shutdown

	if (lParam == 0)
		((CPartFile*)wParam)->FlushBuffersExceptionHandler();
	else
		((CPartFile*)wParam)->FlushBuffersExceptionHandler((CFileException*)lParam);
	return 0;
}

LRESULT CemuleDlg::OnFileCompleted(WPARAM wParam, LPARAM lParam)
{
	//MORPH START - Added by SiRoB, Fix crash at shutdown
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;
	//MORPH END   - Added by SiRoB, Fix crash at shutdown
	CPartFile* partfile = (CPartFile*)lParam;
	ASSERT( partfile != NULL );
	if (partfile)
		partfile->PerformFileCompleteEnd(wParam);
	return 0;
}

#ifdef _DEBUG
void BeBusy(UINT uSeconds, LPCSTR pszCaller)
{
	UINT s = 0;
	while (uSeconds--) {
		theVerboseLog.Logf(_T("%hs: called=%hs, waited %u sec."), __FUNCTION__, pszCaller, s++);
		Sleep(1000);
	}
}
#endif

BOOL CemuleDlg::OnQueryEndSession()
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);
	if (!CTrayDialog::OnQueryEndSession())
		return FALSE;

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning TRUE"), __FUNCTION__);
	return TRUE;
}

void CemuleDlg::OnEndSession(BOOL bEnding)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: bEnding=%d"), __FUNCTION__, bEnding);
	if (bEnding && theApp.m_app_state == APP_STATE_RUNNING)
	{
		// If eMule was *not* started with "RUNAS":
		// When user is logging of (or reboots or shutdown system), Windows sends the
		// WM_QUERYENDSESSION/WM_ENDSESSION to all top level windows.
		// Here we can consume as much time as we need to perform our shutdown. Even if we
		// take longer than 20 seconds, Windows will just show a dialog box that 'emule'
		// is not terminating in time and gives the user a chance to cancel that. If the user
		// does not cancel the Windows dialog, Windows will though wait until eMule has 
		// terminated by itself - no data loss, no file corruption, everything is fine.
		theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
		OnClose();
	}

	CTrayDialog::OnEndSession(bEnding);
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning"), __FUNCTION__);
}

LRESULT CemuleDlg::OnUserChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);
	// Just want to know if we ever get this message. Maybe it helps us to handle the
	// logoff/reboot/shutdown problem when eMule was started with "RUNAS".
	return Default();
}

LRESULT CemuleDlg::OnConsoleThreadEvent(WPARAM wParam, LPARAM lParam)
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs: nEvent=%u, nThreadID=%u"), __FUNCTION__, wParam, lParam);

	// If eMule was started with "RUNAS":
	// This message handler receives a 'console event' from the concurrently and thus
	// asynchronously running console control handler thread which was spawned by Windows
	// in case the user logs off/reboots/shutdown. Even if the console control handler thread
	// is waiting on the result from this message handler (is waiting until the main thread
	// has finished processing this inter-application message), the application will get
	// forcefully terminated by Windows after 20 seconds! There is no known way to prevent
	// that. This means, that if we would invoke our standard shutdown code ('OnClose') here
	// and the shutdown takes longer than 20 sec, we will get forcefully terminated by 
	// Windows, regardless of what we are doing. This means, MET-file and PART-file corruption
	// may occure. Because the shutdown code in 'OnClose' does also shutdown Kad (which takes
	// a noticeable amount of time) it is not that unlikely that we run into problems with
	// not being finished with our shutdown in 20 seconds.
	// 
	if (theApp.m_app_state == APP_STATE_RUNNING)
	{
#if 1
		// And it really should be OK to expect that emule can shutdown in 20 sec on almost
		// all computers. So, use the proper shutdown.
		theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
		OnClose();	// do not invoke if shutdown takes longer than 20 sec, read above
#else
		// As a minimum action we at least set the 'shutting down' flag, this will help e.g.
		// the CUploadQueue::UploadTimer to not start any file save actions which could get
		// interrupted by windows and which would then lead to corrupted MET-files.
		// Setting this flag also helps any possible running threads to stop their work.
		theApp.m_app_state = APP_STATE_SHUTTINGDOWN;

#ifdef _DEBUG
		// Simulate some work.
		//
		// NOTE: If the console thread has already exited, Windows may terminate the process
		// even before the 20 sec. timeout!
		//BeBusy(70, __FUNCTION__);
#endif

		// Actually, just calling 'ExitProcess' should be the most safe thing which we can
		// do here. Because we received this message via the main message queue we are 
		// totally in-sync with the application and therefore we know that we are currently
		// not within a file save action and thus we simply can not cause any file corruption
		// when we exit right now.
		//
		// Of course, there may be some data loss. But it's the same amount of data loss which
		// could occure if we keep running. But if we keep running and wait until Windows
		// terminates us after 20 sec, there is also the chance for file corruption.
		if (thePrefs.GetDebug2Disk()) {
			theVerboseLog.Logf(_T("%hs: ExitProcess"), __FUNCTION__);
			theVerboseLog.Close();
		}
		ExitProcess(0);
#endif
	}

	AddDebugLogLine(DLP_VERYLOW, _T("%hs: returning"), __FUNCTION__);
	return 1;
}

void CemuleDlg::OnDestroy()
{
	AddDebugLogLine(DLP_VERYLOW, _T("%hs"), __FUNCTION__);

	// If eMule was started with "RUNAS":
	// When user is logging of (or reboots or shutdown system), Windows may or may not send 
	// a WM_DESTROY (depends on how long the application needed to process the 
	// CTRL_LOGOFF_EVENT). But, regardless of what happened and regardless of how long any
	// application specific shutdown took, Windows fill forcefully terminate the process 
	// after 1-2 seconds after WM_DESTROY! So, we can not use WM_DESTROY for any lengthy
	// shutdown actions in that case.
	CTrayDialog::OnDestroy();
}

bool CemuleDlg::CanClose()
{
	if (theApp.m_app_state == APP_STATE_RUNNING && thePrefs.IsConfirmExitEnabled())
	{
		if (AfxMessageBox(GetResString(IDS_MAIN_EXIT), MB_YESNO | MB_DEFBUTTON2) == IDNO)
			return false;
	}
	return true;
}

void CemuleDlg::OnClose()
{
	if(theApp.IsSplash()) theApp.DestroySplash(); //Xman new slpash-screen arrangement

	if (!CanClose())
		return;

	//Xman new slpash-screen arrangement
	if (thePrefs.UseSplashScreen())
		{
			//Xman don't show splash on old windows->crash
			switch (thePrefs.GetWindowsVersion())
			{
			case _WINVER_98_:
			case _WINVER_95_:	
			case _WINVER_ME_:
				break;
			default:
				theApp.ShowSplash(false);
			}
		}
	theApp.UpdateSplash(_T("Shutting down ..."));
	//Xman end

	Log(_T("Closing eMule"));
	CloseTTS();
	m_pDropTarget->Revoke();

	// ==> Show in MSN7 [TPT] - Stulle
	//if we have enabled on close, we must send the kill message
	if(thePrefs.GetShowMSN7())
		UpdateMSN(0,0,0,0, true);
	// <== Show in MSN7 [TPT] - Stulle

	theApp.EndDirectoryWatcher(); // Automatic shared files updater [MoNKi] - Stulle

	theApp.m_app_state = APP_STATE_SHUTTINGDOWN;

	//Xman queued disc-access for read/flushing-threads
	theApp.ForeAllDiscAccessThreadsToFinish();

	theApp.serverconnect->Disconnect();
	theApp.OnlineSig(); // Added By Bouc7 

	// get main window placement
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	GetWindowPlacement(&wp);
	ASSERT( wp.showCmd == SW_SHOWMAXIMIZED || wp.showCmd == SW_SHOWMINIMIZED || wp.showCmd == SW_SHOWNORMAL );
	if (wp.showCmd == SW_SHOWMINIMIZED && (wp.flags & WPF_RESTORETOMAXIMIZED))
		wp.showCmd = SW_SHOWMAXIMIZED;
	wp.flags = 0;
	thePrefs.SetWindowLayout(wp);

	// get active main window dialog
	if (activewnd){
		if (activewnd->IsKindOf(RUNTIME_CLASS(CServerWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_SERVER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CSharedFilesWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_FILES);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CSearchDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_SEARCH);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CChatWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_CHAT);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CTransferWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_TRANSFER);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CStatisticsDlg)))
			thePrefs.SetLastMainWndDlgID(IDD_STATISTICS);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CKademliaWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_KADEMLIAWND);
		else if (activewnd->IsKindOf(RUNTIME_CLASS(CIrcWnd)))
			thePrefs.SetLastMainWndDlgID(IDD_IRC);
		else{
			ASSERT(0);
			thePrefs.SetLastMainWndDlgID(0);
		}
	}

	Kademlia::CKademlia::Stop();	// couple of data files are written

	theApp.UpdateSplash(_T("waiting for hash end")); //Xman new slpash-screen arrangement

	// try to wait untill the hashing thread notices that we are shutting down
	CSingleLock sLock1(&theApp.hashing_mut); // only one filehash at a time
	sLock1.Lock(2000);

	//Xman queued disc-access for read/flushing-threads
	//if we don't unlock we can cause a deadlock here:
	//resuming the disc-access-thread with th above ForeAllDiscAccessThreadsToFinish
	//doesn't mean the thread start at once... they start later and the main app has the mutex
	//so the threads wait for ever.. and also the main app for the threads.
	sLock1.Unlock(); 
	//Xman end

	theApp.UpdateSplash(_T("saving settings ...")); //Xman new slpash-screen arrangement

	// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
	/*
	// saving data & stuff
	theApp.emuledlg->preferenceswnd->m_wndSecurity.DeleteDDB();

	theApp.knownfiles->Save();										// CKnownFileList::Save
	theApp.sharedfiles->Save();
	//transferwnd->downloadlistctrl.SaveSettings();
	//transferwnd->downloadclientsctrl.SaveSettings();
	//transferwnd->uploadlistctrl.SaveSettings();
	//transferwnd->queuelistctrl.SaveSettings();
	//transferwnd->clientlistctrl.SaveSettings();
	//sharedfileswnd->sharedfilesctrl.SaveSettings();
	//chatwnd->m_FriendListCtrl.SaveSettings();
	searchwnd->SaveAllSettings();
	serverwnd->SaveAllSettings();
	kademliawnd->SaveAllSettings();
	//sharedfileswnd->historylistctrl.SaveSettings(CPreferences::tableHistory); //Xman [MoNKi: -Downloaded History-]

	//Xman new adapter selection
	if(theApp.pBandWidthControl->GetwasNAFCLastActive()==true)
		thePrefs.SetNAFCFullControl(true);
	//Xman end

	theApp.m_pPeerCache->Save();
	theApp.scheduler->RestoreOriginals();
	theApp.searchlist->SaveSpamFilter();
	if (thePrefs.IsStoringSearchesEnabled())
		theApp.searchlist->StoreSearches();
	
	// close uPnP Ports
	theApp.m_pUPnPFinder->GetImplementation()->StopAsyncFind();
	if (thePrefs.CloseUPnPOnExit())
		theApp.m_pUPnPFinder->GetImplementation()->DeletePorts();

	//Xman don't overwrite bak files if last sessions crashed
	//remark: it would be better to set the flag after all deletions, but this isn't possible, because the prefs need access to the objects when saving
	thePrefs.m_this_session_aborted_in_an_unnormal_way=false;
	//Xman end
	thePrefs.Save();
	thePerfLog.Shutdown();
	*/
	SaveSettings(true);

	if (thePrefs.GetAutoBackup2())
		theApp.emuledlg->preferenceswnd->m_wndScar.Backup3();
	if (thePrefs.GetAutoBackup())
	{
		theApp.emuledlg->preferenceswnd->m_wndScar.Backup(_T("*.ini"), false);
		theApp.emuledlg->preferenceswnd->m_wndScar.Backup(_T("*.dat"), false);
		theApp.emuledlg->preferenceswnd->m_wndScar.Backup(_T("*.met"), false);
	}
	// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	if(thePrefs.GetInvisibleMode())
		UnRegisterInvisibleHotKey();
	// <== Invisible Mode [TPT/MoNKi] - Stulle

	theApp.UpdateSplash(_T("clearing displayed items ...")); //Xman new slpash-screen arrangement

	// explicitly delete all listview items which may hold ptrs to objects which will get deleted
	// by the dtors (some lines below) to avoid potential problems during application shutdown.
	transferwnd->downloadlistctrl.DeleteAllItems();
	chatwnd->chatselector.DeleteAllItems();
	chatwnd->m_FriendListCtrl.DeleteAllItems();
	theApp.clientlist->DeleteAll();
	searchwnd->DeleteAllSearchListCtrlItems();
	sharedfileswnd->sharedfilesctrl.DeleteAllItems();
    transferwnd->queuelistctrl.DeleteAllItems();
	transferwnd->clientlistctrl.DeleteAllItems();
	transferwnd->uploadlistctrl.DeleteAllItems();
	transferwnd->downloadclientsctrl.DeleteAllItems();
	serverwnd->serverlistctrl.DeleteAllItems();
	sharedfileswnd->historylistctrl.DeleteAllItems(); //Xman [MoNKi: -Downloaded History-]

	CPartFileConvert::CloseGUI();
	CPartFileConvert::RemoveAllJobs();

	// ==> UPnP support [MoNKi] - leuk_he
	AfxBeginThread(theApp.m_UPnP_IGDControlPoint->RemoveInstance,NULL); // seperate thread since devic may have hickups...
	// <== UPnP support [MoNKi] - leuk_he
    theApp.uploadBandwidthThrottler->EndThread();
	//Xman
	/*
    theApp.lastCommonRouteFinder->EndThread();
	*/
	//Xman end

	theApp.sharedfiles->DeletePartFileInstances();

	searchwnd->SendMessage(WM_CLOSE);

	theApp.UpdateSplash(_T("clearing lists ..."));  //Xman new slpash-screen arrangement

	//Xman
	theApp.m_threadlock.WriteLock();	// SLUGFILLER: SafeHash - Last chance, let all running threads close before we start deleting

    // NOTE: Do not move those dtors into 'CemuleApp::InitInstance' (althought they should be there). The
	// dtors are indirectly calling functions which access several windows which would not be available 
	// after we have closed the main window -> crash!
	delete theApp.mmserver;			theApp.mmserver = NULL;
	delete theApp.listensocket;		theApp.listensocket = NULL;
	delete theApp.clientudp;		theApp.clientudp = NULL;
	delete theApp.sharedfiles;		theApp.sharedfiles = NULL;
	delete theApp.serverconnect;	theApp.serverconnect = NULL;
	delete theApp.serverlist;		theApp.serverlist = NULL;		// CServerList::SaveServermetToFile
	delete theApp.knownfiles;		theApp.knownfiles = NULL;
	delete theApp.searchlist;		theApp.searchlist = NULL;
	theApp.UpdateSplash(_T("saving credits ..."));  //Xman new slpash-screen arrangement
	delete theApp.clientcredits;	theApp.clientcredits = NULL;	// CClientCreditsList::SaveList
	theApp.UpdateSplash(_T("clearing queues ..."));  //Xman new slpash-screen arrangement
	delete theApp.downloadqueue;	theApp.downloadqueue = NULL;	// N * (CPartFile::FlushBuffer + CPartFile::SavePartFile)
	delete theApp.uploadqueue;		theApp.uploadqueue = NULL;
	delete theApp.clientlist;		theApp.clientlist = NULL;
	delete theApp.friendlist;		theApp.friendlist = NULL;		// CFriendList::SaveList
	delete theApp.scheduler;		theApp.scheduler = NULL;
	theApp.UpdateSplash(_T("Unloading IPfilter ..."));  //Xman new slpash-screen arrangement
	delete theApp.ipfilter;			theApp.ipfilter = NULL;			// CIPFilter::SaveToDefaultFile
	delete theApp.webserver;		theApp.webserver = NULL;
	delete theApp.m_pPeerCache;		theApp.m_pPeerCache = NULL;
	delete theApp.m_pFirewallOpener;theApp.m_pFirewallOpener = NULL;
	theApp.UpdateSplash(_T("shutdown bandwidthcontrol ..."));  //Xman new slpash-screen arrangement
	delete theApp.uploadBandwidthThrottler; theApp.uploadBandwidthThrottler = NULL;
	//Xman
	/*
	delete theApp.lastCommonRouteFinder; theApp.lastCommonRouteFinder = NULL;
	*/
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	delete theApp.m_pUPnPFinder;	theApp.m_pUPnPFinder = NULL;
	*/
	// <== UPnP support [MoNKi] - leuk_he

	//Xman
	// Maella [patch] -Bandwidth: overall bandwidth measure-	
	delete theApp.pBandWidthControl;theApp.pBandWidthControl = NULL;
	// Maella end

	theApp.UpdateSplash(_T("unload IP to Country ..."));  //Xman new slpash-screen arrangement

	//EastShare Start - added by AndCycle, IP to Country
	delete theApp.ip2country;		theApp.ip2country = NULL;
	//EastShare End   - added by AndCycle, IP to Country

	delete theApp.dlp; theApp.dlp=NULL; //Xman DLP
	
	// ==> TBH: minimule - Max
	theApp.UpdateSplash(_T("destroy TBH: MiniMule ..."));
	theApp.minimule->DestroyWindow();
	delete theApp.minimule;			theApp.minimule = NULL;
	// <== TBH: minimule - Max

	// ==> CPU/MEM usage [$ick$/Stulle] - Max 
	theApp.UpdateSplash(_T("destroy SysInfo ..."));
	delete theApp.sysinfo;			theApp.sysinfo = NULL;
	// <== CPU/MEM usage [$ick$/Stulle] - Max 

	theApp.UpdateSplash(_T("Shutdown done")); //Xman new slpash-screen arrangement

	thePrefs.Uninit();
	theApp.m_app_state = APP_STATE_DONE;
	CTrayDialog::OnCancel();
	AddDebugLogLine(DLP_VERYLOW, _T("Closed eMule"));
}

void CemuleDlg::DestroyMiniMule()
{
	if (m_pMiniMule)
	{
		if (!m_pMiniMule->IsInCallback()) // for safety
		{
			TRACE("%s - m_pMiniMule->DestroyWindow();\n", __FUNCTION__);
			m_pMiniMule->DestroyWindow();
			ASSERT( m_pMiniMule == NULL );
			m_pMiniMule = NULL;
		}
		else
			ASSERT(0);
	}
}

LRESULT CemuleDlg::OnCloseMiniMule(WPARAM wParam, LPARAM /*lParam*/)
{
	TRACE("%s -> DestroyMiniMule();\n", __FUNCTION__);
	DestroyMiniMule();
	if (wParam)
		RestoreWindow();
	return 0;
}

// ==> TBH: minimule (open on tray) - Max
void CemuleDlg::RunMiniMule()
{
	try
	{
		TRACE("%s - m_pMiniMule = new CMiniMule(this);\n", __FUNCTION__);
		ASSERT( m_pMiniMule == NULL );
		m_pMiniMule = new CMiniMule(this);
		m_pMiniMule->Create(CMiniMule::IDD, this);
		//m_pMiniMule->ShowWindow(SW_SHOW);	// do not explicitly show the window, it will do that for itself when it's ready..
		m_pMiniMule->SetForegroundWindow();
		m_pMiniMule->BringWindowToTop();
	}
	catch(...)
	{
		ASSERT(0);
		m_pMiniMule = NULL;
	}
}
// <== TBH: minimule (open on tray) - Max

void CemuleDlg::OnTrayLButtonUp(CPoint /*pt*/)
{
	if(!IsRunning())
		return;

	// Avoid reentrancy problems with main window, options dialog and mini mule window
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}

	if (m_pMiniMule) {
		TRACE("%s - m_pMiniMule->ShowWindow(SW_SHOW);\n", __FUNCTION__);
		m_pMiniMule->ShowWindow(SW_SHOW);
		m_pMiniMule->SetForegroundWindow();
		m_pMiniMule->BringWindowToTop();
		return;
	}

	// ==> TBH: minimule - Max
	/*
	if (thePrefs.GetEnableMiniMule())
	{
		try
		{
			TRACE("%s - m_pMiniMule = new CMiniMule(this);\n", __FUNCTION__);
			ASSERT( m_pMiniMule == NULL );
			m_pMiniMule = new CMiniMule(this);
			m_pMiniMule->Create(CMiniMule::IDD, this);
			//m_pMiniMule->ShowWindow(SW_SHOW);	// do not explicitly show the window, it will do that for itself when it's ready..
			m_pMiniMule->SetForegroundWindow();
			m_pMiniMule->BringWindowToTop();
		}
		catch(...)
		{
			ASSERT(0);
			m_pMiniMule = NULL;
		}
	}
	*/
	if (thePrefs.GetEnableMiniMule() && !thePrefs.IsMiniMuleEnabled())
		RunMiniMule(); // TBH: minimule (open on tray) - Stulle
	else if (thePrefs.IsMiniMuleEnabled())
	{
		if (thePrefs.GetMiniMuleLives())
			theApp.minimule->RunMiniMule();
		else
			theApp.minimule->RunMiniMule(true);

		theApp.minimule->ShowWindow(SW_SHOW);
	}
	// <== TBH: minimule - Max
}

void CemuleDlg::OnTrayRButtonUp(CPoint pt)
{
	if (!IsRunning())
		return;

	// Avoid reentrancy problems with main window, options dialog and mini mule window
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}

	if (m_pMiniMule)
	{
		if (m_pMiniMule->GetAutoClose())
		{
			TRACE("%s - m_pMiniMule->GetAutoClose() -> DestroyMiniMule();\n", __FUNCTION__);
			DestroyMiniMule();
		}
		else
		{
			// Avoid reentrancy problems with main window, options dialog and mini mule window
			if (m_pMiniMule->m_hWnd && !m_pMiniMule->IsWindowEnabled()) {
				MessageBeep(MB_OK);
				return;
			}
		}
	}

	if (m_pSystrayDlg) {
		m_pSystrayDlg->BringWindowToTop();
		return;
	}

	m_pSystrayDlg = new CMuleSystrayDlg(this, pt,
										//Xman
										// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
										/*
										thePrefs.GetMaxGraphUploadRate(true), thePrefs.GetMaxGraphDownloadRate(),
										thePrefs.GetMaxUpload(), thePrefs.GetMaxDownload());
										*/
										thePrefs.GetMaxGraphUploadRate(), thePrefs.GetMaxGraphDownloadRate(),
										thePrefs.GetMaxUpload(), thePrefs.GetMaxDownload());
										//Xman end
	if (m_pSystrayDlg)
	{
		UINT nResult = m_pSystrayDlg->DoModal();
		delete m_pSystrayDlg;
		m_pSystrayDlg = NULL;
		switch (nResult)
		{
			case IDC_TOMAX:
				QuickSpeedOther(MP_QS_UA);
				break;
			case IDC_TOMIN:
				QuickSpeedOther(MP_QS_PA);
				break;
			case IDC_RESTORE:
				RestoreWindow();
				break;
			case IDC_CONNECT:
				StartConnection();
				break;
			case IDC_DISCONNECT:
				CloseConnection();
				break;
			case IDC_EXIT:
				OnClose();
				break;
			case IDC_PREFERENCES:
				ShowPreferences();
				break;
		}
	}
}

void CemuleDlg::AddSpeedSelectorMenus(CMenu* addToMenu)
{
	CString text;

	// Create UploadPopup Menu
	ASSERT( m_menuUploadCtrl.m_hMenu == NULL );
	if (m_menuUploadCtrl.CreateMenu())
	{
		//Xman modified
		/*
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate(true)*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U80,  text);
		text.Format(_T("100%%\t%i %s"), (uint16)(thePrefs.GetMaxGraphUploadRate(true)),GetResString(IDS_KBYTESPERSEC));		m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U100, text);
		*/
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U80,  text);
		text.Format(_T("90%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphUploadRate()*0.9),GetResString(IDS_KBYTESPERSEC));	m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_U90, text);
		//Xman end
		m_menuUploadCtrl.AppendMenu(MF_SEPARATOR);
		
		if (GetRecMaxUpload() > 0) {
			//Xman modified
			/*
			text.Format(GetResString(IDS_PW_MINREC) + GetResString(IDS_KBYTESPERSEC), GetRecMaxUpload());
			*/
			text.Format(GetResString(IDS_PW_MINREC) + GetResString(IDS_KBYTESPERSEC), (uint16)GetRecMaxUpload());
			//Xman end
			m_menuUploadCtrl.AppendMenu(MF_STRING, MP_QS_UP10, text);
		}

		text.Format(_T("%s:"), GetResString(IDS_PW_UPL));
		addToMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_menuUploadCtrl.m_hMenu, text);
	}

	// Create DownloadPopup Menu
	ASSERT( m_menuDownloadCtrl.m_hMenu == NULL );
	if (m_menuDownloadCtrl.CreateMenu())
	{
		text.Format(_T("20%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.2),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D20,  text);
		text.Format(_T("40%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.4),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D40,  text);
		text.Format(_T("60%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.6),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D60,  text);
		text.Format(_T("80%%\t%i %s"),  (uint16)(thePrefs.GetMaxGraphDownloadRate()*0.8),GetResString(IDS_KBYTESPERSEC));	m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D80,  text);
		text.Format(_T("100%%\t%i %s"), (uint16)(thePrefs.GetMaxGraphDownloadRate()),GetResString(IDS_KBYTESPERSEC));		m_menuDownloadCtrl.AppendMenu(MF_STRING|MF_POPUP, MP_QS_D100, text);

		text.Format(_T("%s:"), GetResString(IDS_PW_DOWNL));
		addToMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_menuDownloadCtrl.m_hMenu, text);
	}

	addToMenu->AppendMenu(MF_SEPARATOR);
	addToMenu->AppendMenu(MF_STRING, MP_CONNECT, GetResString(IDS_MAIN_BTN_CONNECT));
	addToMenu->AppendMenu(MF_STRING, MP_DISCONNECT, GetResString(IDS_MAIN_BTN_DISCONNECT)); 
}

void CemuleDlg::StartConnection()
{
	if (   (!theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsConnected())
		|| !Kademlia::CKademlia::IsRunning())
	{
		// ==> UPnP support [MoNKi] - leuk_he
		/*
		// UPnP is still trying to open the ports. In order to not get a LowID by connecting to the servers / kad before
		// the ports are opened we delay the connection untill UPnP gets a result or the timeout is reached
		// If the user clicks two times on the button, let him have his will and connect regardless
		if (m_hUPnPTimeOutTimer != 0 && !m_bConnectRequestDelayedForUPnP){
			AddLogLine(false, GetResString(IDS_DELAYEDBYUPNP));
			AddLogLine(true, GetResString(IDS_DELAYEDBYUPNP2));
			m_bConnectRequestDelayedForUPnP = true;
			return;
		}
		else{
			m_bConnectRequestDelayedForUPnP = false;
			if (m_hUPnPTimeOutTimer != 0){
				VERIFY( ::KillTimer(NULL, m_hUPnPTimeOutTimer) );
				m_hUPnPTimeOutTimer = 0;
			}
		*/
		// <== UPnP support [MoNKi] - leuk_he
			AddLogLine(true, GetResString(IDS_CONNECTING));

			// ed2k
			if ((thePrefs.GetNetworkED2K() || m_bEd2kSuspendDisconnect) && !theApp.serverconnect->IsConnecting() && !theApp.serverconnect->IsConnected()) {
				theApp.serverconnect->ConnectToAnyServer();
			}

			// kad
			if ((thePrefs.GetNetworkKademlia() || m_bKadSuspendDisconnect) && !Kademlia::CKademlia::IsRunning()) {
				Kademlia::CKademlia::Start();
			}
		// ==> UPnP support [MoNKi] - leuk_he
		/*
		}
		*/
		// <== UPnP support [MoNKi] - leuk_he

		ShowConnectionState();
	}
	m_bEd2kSuspendDisconnect = false;
	m_bKadSuspendDisconnect = false;
}

void CemuleDlg::CloseConnection()
{
	if (theApp.serverconnect->IsConnected()){
		theApp.serverconnect->Disconnect();
	}

	if (theApp.serverconnect->IsConnecting()){
		theApp.serverconnect->StopConnectionTry();
	}
	Kademlia::CKademlia::Stop();
	theApp.OnlineSig(); // Added By Bouc7 
	ShowConnectionState();
}

void CemuleDlg::RestoreWindow()
{
	if (IsPreferencesDlgOpen()) {
		MessageBeep(MB_OK);
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return;
	}
	// ==> Static Tray Icon [MorphXT] - MyTh88
	/*
	if (TrayIsVisible())
	*/
	if (thePrefs.GetStaticIcon() == false)
	// <== Static Tray Icon [MorphXT] - MyTh88
		TrayHide();

	DestroyMiniMule();

	if (m_wpFirstRestore.length)
	{
		SetWindowPlacement(&m_wpFirstRestore);
		memset(&m_wpFirstRestore, 0, sizeof m_wpFirstRestore);
		SetForegroundWindow();
		BringWindowToTop();
	}
	else
		CTrayDialog::RestoreWindow();
}

void CemuleDlg::UpdateTrayIcon(int iPercent)
{
	// compute an id of the icon to be generated
	UINT uSysTrayIconCookie = (iPercent > 0) ? (16 - ((iPercent*15/100) + 1)) : 0;
	if (theApp.IsConnected()) {
		if (!theApp.IsFirewalled())
			uSysTrayIconCookie += 50;
	}
	else
		uSysTrayIconCookie += 100;
	
	// dont update if the same icon as displayed would be generated
	// ==> Completed in Tray [Stulle] - Stulle
	/*
	if (m_uLastSysTrayIconCookie == uSysTrayIconCookie)
		return;
	*/
	if (m_uLastSysTrayIconCookie == uSysTrayIconCookie && m_bTrayBoolOld == m_bTrayBool)
		return;
	m_bTrayBoolOld = m_bTrayBool;
	// <== Completed in Tray [Stulle] - Stulle
	m_uLastSysTrayIconCookie = uSysTrayIconCookie;

	// prepare it up
	if (m_iMsgIcon!=0 && thePrefs.DoFlashOnNewMessage()==true ) {
		m_iMsgBlinkState=!m_iMsgBlinkState;

		if (m_iMsgBlinkState)
			m_TrayIcon.Init(imicons[1], 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
	} else m_iMsgBlinkState=false;

	if (!m_iMsgBlinkState) {
		// ==> Completed in Tray [Stulle] - Stulle
		/*
		if (theApp.IsConnected()) {
			if (theApp.IsFirewalled())
				m_TrayIcon.Init(m_icoSysTrayLowID, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
			else
				m_TrayIcon.Init(m_icoSysTrayConnected, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
		}
		else
			m_TrayIcon.Init(m_icoSysTrayDisconnected, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
		*/
		if (theApp.IsConnected()) {
			if (thePrefs.GetTrayComplete() && m_bTrayBool)
			{
				if (theApp.IsFirewalled())
					m_TrayIcon.Init(m_icoSysTrayLowIDPlus, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
				else
					m_TrayIcon.Init(m_icoSysTrayConnectedPlus, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
			}
			else
			{
				if (theApp.IsFirewalled())
					m_TrayIcon.Init(m_icoSysTrayLowID, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
				else
					m_TrayIcon.Init(m_icoSysTrayConnected, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
			}
		}
		else if (thePrefs.GetTrayComplete() && m_bTrayBool)
			m_TrayIcon.Init(m_icoSysTrayDisconnectedPlus, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
		else
			m_TrayIcon.Init(m_icoSysTrayDisconnected, 100, 1, 1, 16, 16, thePrefs.GetStatsColor(11));
		// <== Completed in Tray [Stulle] - Stulle
	}

	// load our limit and color info
	static const int aiLimits[1] = { 100 }; // set the limits of where the bar color changes (low-high)
	COLORREF aColors[1] = { thePrefs.GetStatsColor(11) }; // set the corresponding color for each level
	m_TrayIcon.SetColorLevels(aiLimits, aColors, _countof(aiLimits));

	// generate the icon (do *not* destroy that icon using DestroyIcon(), that's done in 'TrayUpdate')
	int aiVals[1] = { iPercent };
	m_icoSysTrayCurrent = m_TrayIcon.Create(aiVals);
	ASSERT( m_icoSysTrayCurrent != NULL );
	if (m_icoSysTrayCurrent)
		TraySetIcon(m_icoSysTrayCurrent, true);
	TrayUpdate();
}

int CemuleDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	return CTrayDialog::OnCreate(lpCreateStruct);
}

void CemuleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (IsRunning()){
		ShowTransferRate(true);

		if (bShow == TRUE && activewnd == chatwnd)
			chatwnd->chatselector.ShowChat();

	}
	CTrayDialog::OnShowWindow(bShow, nStatus);
}

void CemuleDlg::ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink, bool bForceSoundOFF)
{
	if (!notifierenabled)
		return;

	LPCTSTR pszSoundEvent = NULL;
	int iSoundPrio = 0;
	bool bShowIt = false;
	switch (iMsgType)
	{
		case TBN_CHAT:
            if (thePrefs.GetNotifierOnChat())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_Chat");
				iSoundPrio = 1;
			}
			break;
		case TBN_DOWNLOADFINISHED:
            if (thePrefs.GetNotifierOnDownloadFinished())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_DownloadFinished");
				iSoundPrio = 1;
				SendNotificationMail(iMsgType, pszText);
			}
			break;
		case TBN_DOWNLOADADDED:
            if (thePrefs.GetNotifierOnNewDownload())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_DownloadAdded");
				iSoundPrio = 1;
			}
			break;
		case TBN_LOG:
            if (thePrefs.GetNotifierOnLog())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_LogEntryAdded");
			}
			break;
		case TBN_IMPORTANTEVENT:
			if (thePrefs.GetNotifierOnImportantError())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_Urgent");
				iSoundPrio = 1;
				SendNotificationMail(iMsgType, pszText);
			}
			break;

		case TBN_NEWVERSION:
			if (thePrefs.GetNotifierOnNewVersion())
			{
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_NewVersion");
				iSoundPrio = 1;
			}
			break;

		//Xman versions check
		case TBN_NEWMVERSION:
			{	
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_NewVersion");
				iSoundPrio = 1;
			}
			break;
		//Xman end

		// ==> ScarAngel Version Check - Stulle
		case TBN_NEWSVERSION:
			{	
				m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
				bShowIt = true;
				pszSoundEvent = _T("eMule_NewVersion");
				iSoundPrio = 1;
			}
			break;
		// <== ScarAngel Version Check - Stulle

		case TBN_NULL:
            m_wndTaskbarNotifier->Show(pszText, iMsgType, pszLink);
			bShowIt = true;
			break;
	}
	
	if (bShowIt && !bForceSoundOFF && thePrefs.GetNotifierSoundType() != ntfstNoSound)
	{
		bool bNotifiedWithAudio = false;
		if (thePrefs.GetNotifierSoundType() == ntfstSpeech)
			bNotifiedWithAudio = Speak(pszText);

		if (!bNotifiedWithAudio)
		{
			if (!thePrefs.GetNotifierSoundFile().IsEmpty())
			{
				PlaySound(thePrefs.GetNotifierSoundFile(), NULL, SND_FILENAME | SND_NOSTOP | SND_NOWAIT | SND_ASYNC);
			}
			else if (pszSoundEvent)
			{
				// use 'SND_NOSTOP' only for low priority events, otherwise the 'Log message' event may overrule a more important
				// event which is fired nearly at the same time.
				PlaySound(pszSoundEvent, NULL, SND_APPLICATION | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT | ((iSoundPrio > 0) ? 0 : SND_NOSTOP));
			}
		}
	}
}

void CemuleDlg::LoadNotifier(CString configuration)
{
	notifierenabled = m_wndTaskbarNotifier->LoadConfiguration(configuration)!=FALSE;
}

LRESULT CemuleDlg::OnTaskbarNotifierClicked(WPARAM /*wParam*/, LPARAM lParam)
{
	if (lParam)
	{
		LPTSTR pszLink = (LPTSTR)lParam;
		ShellOpenFile(pszLink, NULL);
		free(pszLink);
		pszLink = NULL;
	}

	switch (m_wndTaskbarNotifier->GetMessageType())
	{
		case TBN_CHAT:
			RestoreWindow();
			SetActiveDialog(chatwnd);
			break;

		case TBN_DOWNLOADFINISHED:
			// if we had a link and opened the downloaded file, dont restore the app window
			if (lParam==0)
			{
				RestoreWindow();
				SetActiveDialog(transferwnd);
			}
			break;

		case TBN_DOWNLOADADDED:
			RestoreWindow();
			SetActiveDialog(transferwnd);
			break;

		case TBN_IMPORTANTEVENT:
			RestoreWindow();
			SetActiveDialog(serverwnd);	
			break;

		case TBN_LOG:
			RestoreWindow();
			SetActiveDialog(serverwnd);	
			break;

		case TBN_NEWVERSION:
		{
			CString theUrl;
			theUrl.Format(_T("/en/version_check.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
			theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
			ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		}

		//Xman versions check
		case TBN_NEWMVERSION:
			{
				ShellExecute(NULL, NULL, MOD_HPLINK, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
				break;
			}
		//Xman end

		// ==> ScarAngel Version Check - Stulle
		case TBN_NEWSVERSION:
			{
				// ==> Mephisto mod [Stulle] - Mephisto
				/*
				ShellExecute(NULL, NULL, _T("http://scarangel.sourceforge.net/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
				*/
				ShellExecute(NULL, NULL, _T("http://mephisto.emule-web.de/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
				// <== Mephisto mod [Stulle] - Mephisto
				break;
			}
		// <== ScarAngel Version Check - Stulle
	}
    return 0;
}

void CemuleDlg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	TRACE(_T("CemuleDlg::OnSettingChange: uFlags=0x%08x  lpszSection=\"%s\"\n"), lpszSection);
	// Do not update the Shell's large icon size, because we still have an image list
	// from the shell which contains the old large icon size.
	//theApp.UpdateLargeIconSize();
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSettingChange(uFlags, lpszSection);
}

void CemuleDlg::OnSysColorChange()
{
	theApp.UpdateDesktopColorDepth();
	CTrayDialog::OnSysColorChange();
	SetAllIcons();
}

// ==> Design Settings [eWombat/Stulle] - Max
// See end of file for modded function
/*
HBRUSH CemuleDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}
*/
// <== Design Settings [eWombat/Stulle] - Max

HBRUSH CemuleDlg::GetCtlColor(CDC* /*pDC*/, CWnd* /*pWnd*/, UINT nCtlColor)
{
	UNREFERENCED_PARAMETER(nCtlColor);
	// This function could be used to give the entire eMule (at least all of the main windows)
	// a somewhat more Vista like look by giving them all a bright background color.
	// However, again, the ownerdrawn tab controls are noticeable disturbing that attempt. They
	// do not change their background color accordingly. They don't use NMCUSTOMDRAW nor to they
	// use WM_CTLCOLOR...
	//
	//if (theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6,16,0,0) && g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed()) {
	//	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	//		return GetSysColorBrush(COLOR_WINDOW);
	//}
	return NULL;
}

void CemuleDlg::SetAllIcons()
{
	// application icon (although it's not customizable, we may need to load a different color resolution)
	if (m_hIcon)
		VERIFY( ::DestroyIcon(m_hIcon) );
	// NOTE: the application icon name is prefixed with "AAA" to make sure it's alphabetically sorted by the
	// resource compiler as the 1st icon in the resource table!
	m_hIcon = AfxGetApp()->LoadIcon(_T("AAAEMULEAPP"));
	SetIcon(m_hIcon, TRUE);
	// this scales the 32x32 icon down to 16x16, does not look nice at least under WinXP
	//SetIcon(m_hIcon, FALSE);

	// connection state
	for (int i = 0; i < _countof(connicons); i++){
		if (connicons[i]) VERIFY( ::DestroyIcon(connicons[i]) );
	}
	connicons[0] = theApp.LoadIcon(_T("ConnectedNotNot"), 16, 16);
	connicons[1] = theApp.LoadIcon(_T("ConnectedNotLow"), 16, 16);
	connicons[2] = theApp.LoadIcon(_T("ConnectedNotHigh"), 16, 16);
	connicons[3] = theApp.LoadIcon(_T("ConnectedLowNot"), 16, 16);
	connicons[4] = theApp.LoadIcon(_T("ConnectedLowLow"), 16, 16);
	connicons[5] = theApp.LoadIcon(_T("ConnectedLowHigh"), 16, 16);
	connicons[6] = theApp.LoadIcon(_T("ConnectedHighNot"), 16, 16);
	connicons[7] = theApp.LoadIcon(_T("ConnectedHighLow"), 16, 16);
	connicons[8] = theApp.LoadIcon(_T("ConnectedHighHigh"), 16, 16);
	ShowConnectionStateIcon();

	// transfer state
	if (transicons[0]) VERIFY( ::DestroyIcon(transicons[0]) );
	if (transicons[1]) VERIFY( ::DestroyIcon(transicons[1]) );
	if (transicons[2]) VERIFY( ::DestroyIcon(transicons[2]) );
	if (transicons[3]) VERIFY( ::DestroyIcon(transicons[3]) );
	transicons[0] = theApp.LoadIcon(_T("UP0DOWN0"), 16, 16);
	transicons[1] = theApp.LoadIcon(_T("UP0DOWN1"), 16, 16);
	transicons[2] = theApp.LoadIcon(_T("UP1DOWN0"), 16, 16);
	transicons[3] = theApp.LoadIcon(_T("UP1DOWN1"), 16, 16);
	ShowTransferStateIcon();

	// users state
	if (usericon) VERIFY( ::DestroyIcon(usericon) );
	usericon = theApp.LoadIcon(_T("StatsClients"), 16, 16);
	ShowUserStateIcon();

	// traybar icons
	if (m_icoSysTrayConnected) VERIFY( ::DestroyIcon(m_icoSysTrayConnected) );
	if (m_icoSysTrayDisconnected) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnected) );
	if (m_icoSysTrayLowID) VERIFY( ::DestroyIcon(m_icoSysTrayLowID) );
	m_icoSysTrayConnected = theApp.LoadIcon(_T("TrayConnected"), 16, 16);
	m_icoSysTrayDisconnected = theApp.LoadIcon(_T("TrayNotConnected"), 16, 16);
	m_icoSysTrayLowID = theApp.LoadIcon(_T("TrayLowID"), 16, 16);
	// ==> Completed in Tray [Stulle] - Stulle
	if (m_icoSysTrayConnectedPlus) VERIFY( ::DestroyIcon(m_icoSysTrayConnectedPlus) );
	if (m_icoSysTrayDisconnectedPlus) VERIFY( ::DestroyIcon(m_icoSysTrayDisconnectedPlus) );
	if (m_icoSysTrayLowIDPlus) VERIFY( ::DestroyIcon(m_icoSysTrayLowIDPlus) );
	m_icoSysTrayConnectedPlus = theApp.LoadIcon(_T("TrayConnectedPlus"), 16, 16);
	m_icoSysTrayDisconnectedPlus = theApp.LoadIcon(_T("TrayNotConnectedPlus"), 16, 16);
	m_icoSysTrayLowIDPlus = theApp.LoadIcon(_T("TrayLowIDPlus"), 16, 16);
	// <== Completed in Tray [Stulle] - Stulle
	ShowTransferRate(true);

	if (imicons[0]) VERIFY( ::DestroyIcon(imicons[0]) );
	if (imicons[1]) VERIFY( ::DestroyIcon(imicons[1]) );
	if (imicons[2]) VERIFY( ::DestroyIcon(imicons[2]) );
	imicons[0] = NULL;
	imicons[1] = theApp.LoadIcon(_T("Message"), 16, 16);
	imicons[2] = theApp.LoadIcon(_T("MessagePending"), 16, 16);
	ShowMessageState(m_iMsgIcon);
}

void CemuleDlg::Localize()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu)
	{
		VERIFY( pSysMenu->ModifyMenu(MP_ABOUTBOX, MF_BYCOMMAND | MF_STRING, MP_ABOUTBOX, GetResString(IDS_ABOUTBOX)) );
		VERIFY( pSysMenu->ModifyMenu(MP_VERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_VERSIONCHECK, GetResString(IDS_VERSIONCHECK)) );
		//Xman versions check
		// ==> Removed Xtreme version check [Stulle] - Stulle
		/*
		VERIFY( pSysMenu->ModifyMenu(MP_MVERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_MVERSIONCHECK, _T("Xtreme_Version_Check")) );
		*/
		// <== Removed Xtreme version check [Stulle] - Stulle
		//Xman end
		// ==> ScarAngel Version Check - Stulle
		// ==> Mephisto mod [Stulle] - Mephisto
		/*
		VERIFY( pSysMenu->ModifyMenu(MP_SVERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_SVERSIONCHECK, GetResString(IDS_SVERSIONCHECK)) );
		*/
		VERIFY( pSysMenu->ModifyMenu(MP_SVERSIONCHECK, MF_BYCOMMAND | MF_STRING, MP_SVERSIONCHECK, GetResString(IDS_MVERSIONCHECK)) );
		// <== Mephisto mod [Stulle] - Mephisto
		// <== ScarAngel Version Check - Stulle

		switch (thePrefs.GetWindowsVersion())
		{
			case _WINVER_98_:
			case _WINVER_95_:
			case _WINVER_ME_:
				// NOTE: I think the reason why the old version of the following code crashed under Win9X was because
				// of the menus were destroyed right after they were added to the system menu. New code should work
				// under Win9X too but I can't test it.
				break;
			default:{
				// localize the 'speed control' sub menus by deleting the current menus and creating a new ones.

				// remove any already available 'speed control' menus from system menu
				UINT uOptMenuPos = pSysMenu->GetMenuItemCount() - 1;
				CMenu* pAccelMenu = pSysMenu->GetSubMenu(uOptMenuPos);
				if (pAccelMenu)
				{
					ASSERT( pAccelMenu->m_hMenu == m_SysMenuOptions.m_hMenu );
					VERIFY( pSysMenu->RemoveMenu(uOptMenuPos, MF_BYPOSITION) );
					pAccelMenu = NULL;
				}

				// destroy all 'speed control' menus
				if (m_menuUploadCtrl)
					VERIFY( m_menuUploadCtrl.DestroyMenu() );
				if (m_menuDownloadCtrl)
					VERIFY( m_menuDownloadCtrl.DestroyMenu() );
				if (m_SysMenuOptions)
					VERIFY( m_SysMenuOptions.DestroyMenu() );

				// create new 'speed control' menus
				if (m_SysMenuOptions.CreateMenu())
				{
					AddSpeedSelectorMenus(&m_SysMenuOptions);
					pSysMenu->AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)m_SysMenuOptions.m_hMenu, GetResString(IDS_EM_PREFS));
				}
			}
		}
	}

	ShowUserStateIcon();
	toolbar->Localize();
	ShowConnectionState();
	ShowTransferRate(true);
	ShowUserCount();
	CPartFileConvert::Localize();
	if (m_pMiniMule)
		m_pMiniMule->Localize();
}

void CemuleDlg::ShowUserStateIcon()
{
	statusbar->SetIcon(SBarUsers, usericon);
}

void CemuleDlg::QuickSpeedOther(UINT nID)
{
	switch (nID) {
		//Xman changed the values !
		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		/*
		case MP_QS_PA: 
			thePrefs.SetMaxUpload(1);
			thePrefs.SetMaxDownload(1);
			break ;
		case MP_QS_UA: 
			thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate(true));
			thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate());
		*/
		case MP_QS_PA: thePrefs.SetMaxUpload((float)(3));
			thePrefs.SetMaxDownload((float)(1));
			thePrefs.CheckSlotSpeed(); //XMan Xtreme Upload
			break ;
		case MP_QS_UA: 
			thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()-2));
			thePrefs.SetMaxDownload((float)(thePrefs.GetMaxGraphDownloadRate()));
			thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
		//Xman end
			break ;
	}
}


void CemuleDlg::QuickSpeedUpload(UINT nID)
{
	switch (nID) {
		//Xman
		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		/*
		case MP_QS_U10: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.1)); break ;
		case MP_QS_U20: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.2)); break ;
		case MP_QS_U30: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.3)); break ;
		case MP_QS_U40: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.4)); break ;
		case MP_QS_U50: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.5)); break ;
		case MP_QS_U60: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.6)); break ;
		case MP_QS_U70: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.7)); break ;
		case MP_QS_U80: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.8)); break ;
		case MP_QS_U90: thePrefs.SetMaxUpload((UINT)(thePrefs.GetMaxGraphUploadRate(true)*0.9)); break ;
		case MP_QS_U100: thePrefs.SetMaxUpload((UINT)thePrefs.GetMaxGraphUploadRate(true)); break ;
		*/
		case MP_QS_U10: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.1)); break ;
		case MP_QS_U20: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.2)); break ;
		case MP_QS_U30: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.3)); break ;
		case MP_QS_U40: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.4)); break ;
		case MP_QS_U50: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.5)); break ;
		case MP_QS_U60: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.6)); break ;
		case MP_QS_U70: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.7)); break ;
		case MP_QS_U80: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.8)); break ;
		case MP_QS_U90: thePrefs.SetMaxUpload((float)(thePrefs.GetMaxGraphUploadRate()*0.9)); break ;
		case MP_QS_U100: thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate()); break ;
		//Xman end
//		case MP_QS_UPC: thePrefs.SetMaxUpload(UNLIMITED); break ;
		case MP_QS_UP10: thePrefs.SetMaxUpload(GetRecMaxUpload()); break ;
	}
	thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
}

void CemuleDlg::QuickSpeedDownload(UINT nID)
{
	switch (nID) {
		//Xman
		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		/*
		case MP_QS_D10: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.1)); break ;
		case MP_QS_D20: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.2)); break ;
		case MP_QS_D30: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.3)); break ;
		case MP_QS_D40: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.4)); break ;
		case MP_QS_D50: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.5)); break ;
		case MP_QS_D60: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.6)); break ;
		case MP_QS_D70: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.7)); break ;
		case MP_QS_D80: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.8)); break ;
		case MP_QS_D90: thePrefs.SetMaxDownload((UINT)(thePrefs.GetMaxGraphDownloadRate()*0.9)); break ;
		case MP_QS_D100: thePrefs.SetMaxDownload((UINT)thePrefs.GetMaxGraphDownloadRate()); break ;
		*/
		case MP_QS_D10: thePrefs.SetMaxDownload(0.1f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D20: thePrefs.SetMaxDownload(0.2f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D30: thePrefs.SetMaxDownload(0.3f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D40: thePrefs.SetMaxDownload(0.4f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D50: thePrefs.SetMaxDownload(0.5f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D60: thePrefs.SetMaxDownload(0.6f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D70: thePrefs.SetMaxDownload(0.7f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D80: thePrefs.SetMaxDownload(0.8f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D90: thePrefs.SetMaxDownload(0.9f * thePrefs.GetMaxGraphDownloadRate()); break ;
		case MP_QS_D100: thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate()); break ;
		//Xman end
//		case MP_QS_DC: thePrefs.SetMaxDownload(UNLIMITED); break ;
	}
}
// quick-speed changer -- based on xrmb

//Xman
// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
/*
int CemuleDlg::GetRecMaxUpload() {
	
	if (thePrefs.GetMaxGraphUploadRate(true)<7) return 0;
	if (thePrefs.GetMaxGraphUploadRate(true)<15) return thePrefs.GetMaxGraphUploadRate(true)-3;
	return (thePrefs.GetMaxGraphUploadRate(true)-4);

}
*/
float CemuleDlg::GetRecMaxUpload() {
	//Xman changed 6.0.1
	if (thePrefs.GetMaxGraphUploadRate()<10) return thePrefs.GetMaxGraphUploadRate()-1.5f;
	if (thePrefs.GetMaxGraphUploadRate()<20) return thePrefs.GetMaxGraphUploadRate()-2.5f;
	if (thePrefs.GetMaxGraphUploadRate()<26) return thePrefs.GetMaxGraphUploadRate()-3.0f;
	return (thePrefs.GetMaxGraphUploadRate()*0.9f);

}
//Xman end

BOOL CemuleDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{	
		case TBBTN_CONNECT:
			OnBnClickedButton2();
			break;
		case MP_HM_KAD:
		case TBBTN_KAD:
			SetActiveDialog(kademliawnd);
			break;
		case TBBTN_SERVER:
		case MP_HM_SRVR:
			SetActiveDialog(serverwnd);
			break;
		case TBBTN_TRANSFERS:
		case MP_HM_TRANSFER:
			SetActiveDialog(transferwnd);
			break;
		case TBBTN_SEARCH:
		case MP_HM_SEARCH:
			SetActiveDialog(searchwnd);
			break;
		case TBBTN_SHARED:
		case MP_HM_FILES:
			SetActiveDialog(sharedfileswnd);
			break;
		case TBBTN_MESSAGES:
		case MP_HM_MSGS:
			SetActiveDialog(chatwnd);
			break;
		case TBBTN_IRC:
		case MP_HM_IRC:
			SetActiveDialog(ircwnd);
			break;
		case TBBTN_STATS:
		case MP_HM_STATS:
			SetActiveDialog(statisticswnd);
			break;
		case TBBTN_OPTIONS:
		case MP_HM_PREFS:
			toolbar->CheckButton(TBBTN_OPTIONS, TRUE);
			ShowPreferences();
			toolbar->CheckButton(TBBTN_OPTIONS, FALSE);
			break;
		case TBBTN_TOOLS:
			ShowToolPopup(true);
			break;
		case MP_HM_OPENINC:
			ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),NULL, NULL, SW_SHOW); 
			break;
		case MP_HM_HELP:
		case TBBTN_HELP:
			if (activewnd != NULL) {
				HELPINFO hi;
				ZeroMemory(&hi, sizeof(HELPINFO));
				hi.cbSize = sizeof(HELPINFO);
				activewnd->SendMessage(WM_HELP, 0, (LPARAM)&hi);
			}
			else
				wParam = ID_HELP;
			break;
		case MP_HM_CON:
			OnBnClickedButton2();
			break;
		case MP_HM_EXIT:
			OnClose();
			break;
		case MP_HM_LINK1: // MOD: dont remove!
			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL(), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK2:
			ShellExecute(NULL, NULL, thePrefs.GetHomepageBaseURL()+ CString(_T("/faq/")), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		case MP_HM_LINK3: {
			CString theUrl;
			theUrl.Format( thePrefs.GetVersionCheckBaseURL() + CString(_T("/en/version_check.php?version=%i&language=%i")),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
			ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
			break;
		}
		case MP_WEBSVC_EDIT:
			theWebServices.Edit();
			break;
		case MP_HM_CONVERTPF:
			CPartFileConvert::ShowGUI();
			break;
		case MP_HM_SCHEDONOFF:
			thePrefs.SetSchedulerEnabled(!thePrefs.IsSchedulerEnabled());
			theApp.scheduler->Check(true);
			break;
		case MP_HM_1STSWIZARD:
			if (FirstTimeWizard()){
				// start connection wizard
				CConnectionWizardDlg conWizard;
				conWizard.DoModal();
			}
			break;
		case MP_HM_IPFILTER:{
			CIPFilterDlg dlg;
			dlg.DoModal();
			break;
		}
		case MP_HM_DIRECT_DOWNLOAD:{
			CDirectDownloadDlg dlg;
			dlg.DoModal();
			break;
		}
	}	
	if (wParam>=MP_WEBURL && wParam<=MP_WEBURL+99) {
		theWebServices.RunURL(NULL, wParam);
	}
	else if (wParam>=MP_SCHACTIONS && wParam<=MP_SCHACTIONS+99) {
		theApp.scheduler->ActivateSchedule(wParam-MP_SCHACTIONS);
		theApp.scheduler->SaveOriginals(); // use the new settings as original
	}

	return CTrayDialog::OnCommand(wParam, lParam);
}

LRESULT CemuleDlg::OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu)
{
	// ==> XP Style Menu [Xanatos] - Stulle
	if (pMenu->IsKindOf(RUNTIME_CLASS(CTitleMenu)) )
		return CTitleMenu::OnMenuChar(nChar, nFlags, pMenu);
	// <== XP Style Menu [Xanatos] - Stulle

	UINT nCmdID;
	if (toolbar->MapAccelerator((TCHAR)nChar, &nCmdID)){
		OnCommand(nCmdID, 0);
		return MAKELONG(0,MNC_CLOSE);
	}
	return CTrayDialog::OnMenuChar(nChar, nFlags, pMenu);
}

// Barry - To find out if app is running or shutting/shut down
bool CemuleDlg::IsRunning()
{
	return (theApp.m_app_state == APP_STATE_RUNNING);
}


void CemuleDlg::OnBnClickedHotmenu()
{
	ShowToolPopup(false);
}

void CemuleDlg::ShowToolPopup(bool toolsonly)
{
	POINT point;
	::GetCursorPos(&point);

	CTitleMenu menu;
	menu.CreatePopupMenu();
	if (!toolsonly)
		menu.AddMenuTitle(GetResString(IDS_HOTMENU), true);
	else
		menu.AddMenuTitle(GetResString(IDS_TOOLS), true);

	CTitleMenu Links;
	Links.CreateMenu();
	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	Links.AddMenuTitle(NULL, true);
	*/
	Links.AddMenuTitle(GetResString(IDS_LINKS), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	Links.AppendMenu(MF_STRING, MP_HM_LINK1, GetResString(IDS_HM_LINKHP), _T("WEB"));
	Links.AppendMenu(MF_STRING, MP_HM_LINK2, GetResString(IDS_HM_LINKFAQ), _T("WEB"));
	Links.AppendMenu(MF_STRING, MP_HM_LINK3, GetResString(IDS_HM_LINKVC), _T("WEB"));
	theWebServices.GetGeneralMenuEntries(&Links);
	Links.InsertMenu(3, MF_BYPOSITION | MF_SEPARATOR);
	Links.AppendMenu(MF_STRING, MP_WEBSVC_EDIT, GetResString(IDS_WEBSVEDIT));

	// ==> XP Style Menu [Xanatos] - Stulle
	/*
	CMenu scheduler;
	scheduler.CreateMenu();
	*/
	CTitleMenu scheduler;
	scheduler.CreateMenu();
	scheduler.AddMenuTitle(GetResString(IDS_SCHEDULER), true, false);
	// <== XP Style Menu [Xanatos] - Stulle
	CString schedonoff= (!thePrefs.IsSchedulerEnabled())?GetResString(IDS_HM_SCHED_ON):GetResString(IDS_HM_SCHED_OFF);

	// ==> more icons - Stulle
	/*
	scheduler.AppendMenu(MF_STRING,MP_HM_SCHEDONOFF, schedonoff);
	*/
	scheduler.AppendMenu(MF_STRING,MP_HM_SCHEDONOFF, schedonoff, _T("SCHEDULER"));
	// <== more icons - Stulle
	if (theApp.scheduler->GetCount()>0) {
		scheduler.AppendMenu(MF_SEPARATOR);
		for (UINT i=0; i<theApp.scheduler->GetCount();i++)
			scheduler.AppendMenu(MF_STRING,MP_SCHACTIONS+i, theApp.scheduler->GetSchedule(i)->title);
	}

	if (!toolsonly) {
		if (theApp.serverconnect->IsConnected())
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_DISCONNECT), _T("DISCONNECT"));
		else if (theApp.serverconnect->IsConnecting())
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_CANCEL), _T("STOPCONNECTING"));
		else
			menu.AppendMenu(MF_STRING,MP_HM_CON,GetResString(IDS_MAIN_BTN_CONNECT), _T("CONNECT"));

		menu.AppendMenu(MF_STRING,MP_HM_KAD, GetResString(IDS_EM_KADEMLIA), _T("KADEMLIA") );
		menu.AppendMenu(MF_STRING,MP_HM_SRVR, GetResString(IDS_EM_SERVER), _T("SERVER") );
		menu.AppendMenu(MF_STRING,MP_HM_TRANSFER, GetResString(IDS_EM_TRANS),_T("TRANSFER") );
		menu.AppendMenu(MF_STRING,MP_HM_SEARCH, GetResString(IDS_EM_SEARCH), _T("SEARCH"));
		menu.AppendMenu(MF_STRING,MP_HM_FILES, GetResString(IDS_EM_FILES), _T("SharedFiles"));
		menu.AppendMenu(MF_STRING,MP_HM_MSGS, GetResString(IDS_EM_MESSAGES), _T("MESSAGES"));
		menu.AppendMenu(MF_STRING,MP_HM_IRC, GetResString(IDS_IRC), _T("IRC"));
		menu.AppendMenu(MF_STRING,MP_HM_STATS, GetResString(IDS_EM_STATISTIC), _T("STATISTICS"));
		menu.AppendMenu(MF_STRING,MP_HM_PREFS, GetResString(IDS_EM_PREFS), _T("PREFERENCES"));
		menu.AppendMenu(MF_STRING,MP_HM_HELP, GetResString(IDS_EM_HELP), _T("HELP"));
		menu.AppendMenu(MF_SEPARATOR);
	}

	menu.AppendMenu(MF_STRING,MP_HM_OPENINC, GetResString(IDS_OPENINC) + _T("..."), _T("INCOMING"));
	menu.AppendMenu(MF_STRING,MP_HM_CONVERTPF, GetResString(IDS_IMPORTSPLPF) + _T("..."), _T("CONVERT"));
	menu.AppendMenu(MF_STRING,MP_HM_1STSWIZARD, GetResString(IDS_WIZ1) + _T("..."), _T("WIZARD"));
	menu.AppendMenu(MF_STRING,MP_HM_IPFILTER, GetResString(IDS_IPFILTER) + _T("..."), _T("IPFILTER"));
	menu.AppendMenu(MF_STRING,MP_HM_DIRECT_DOWNLOAD, GetResString(IDS_SW_DIRECTDOWNLOAD) + _T("..."), _T("PASTELINK"));

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)Links.m_hMenu, GetResString(IDS_LINKS), _T("WEB") );
	menu.AppendMenu(MF_STRING|MF_POPUP,(UINT_PTR)scheduler.m_hMenu, GetResString(IDS_SCHEDULER), _T("SCHEDULER") );

	if (!toolsonly) {
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING,MP_HM_EXIT, GetResString(IDS_EXIT), _T("EXIT"));
	}
	menu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( Links.DestroyMenu() );
	VERIFY( scheduler.DestroyMenu() );
	VERIFY( menu.DestroyMenu() );
}


void CemuleDlg::ApplyHyperTextFont(LPLOGFONT plf)
{
	theApp.m_fontHyperText.DeleteObject();
	if (theApp.m_fontHyperText.CreateFontIndirect(plf))
	{
		thePrefs.SetHyperTextFont(plf);
		serverwnd->servermsgbox->SetFont(&theApp.m_fontHyperText);
		chatwnd->chatselector.UpdateFonts(&theApp.m_fontHyperText);
		ircwnd->UpdateFonts(&theApp.m_fontHyperText);
	}
}

void CemuleDlg::ApplyLogFont(LPLOGFONT plf)
{
	theApp.m_fontLog.DeleteObject();
	if (theApp.m_fontLog.CreateFontIndirect(plf))
	{
		thePrefs.SetLogFont(plf);
		serverwnd->logbox->SetFont(&theApp.m_fontLog);
		serverwnd->debuglog->SetFont(&theApp.m_fontLog);
		serverwnd->leecherlog->SetFont(&theApp.m_fontLog); //Xman Anti-Leecher-Log
	}
}

LRESULT CemuleDlg::OnFrameGrabFinished(WPARAM wParam,LPARAM lParam){
	CKnownFile* pOwner = (CKnownFile*)wParam;
	FrameGrabResult_Struct* result = (FrameGrabResult_Struct*)lParam;
	//MORPH START - Added by SiRoB, Fix crash at shutdown
	if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN) {
	//MORPH END   - Added by SiRoB, Fix crash at shutdown
		if (theApp.knownfiles->IsKnownFile(pOwner) || theApp.downloadqueue->IsPartFile(pOwner) ){
			pOwner->GrabbingFinished(result->imgResults,result->nImagesGrabbed, result->pSender);
		}
		else{
			ASSERT ( false );
		}
	}//MORPH - Added by SiRoB, Fix crash at shutdown
	delete result;
	return 0;
}

void StraightWindowStyles(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		StraightWindowStyles(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, _countof(szClassName)))
	{
		if (__ascii_stricmp(szClassName, "Button") == 0)
			pWnd->ModifyStyle(BS_FLAT, 0);
		else if (   (__ascii_stricmp(szClassName, "EDIT") == 0 && (pWnd->GetExStyle() & WS_EX_STATICEDGE))
			     || __ascii_stricmp(szClassName, "SysListView32") == 0
			     || __ascii_stricmp(szClassName, "msctls_trackbar32") == 0
			    )
		{
			pWnd->ModifyStyleEx(WS_EX_STATICEDGE, WS_EX_CLIENTEDGE);
		}
		//else if (__ascii_stricmp(szClassName, "SysTreeView32") == 0)
		//{
		//	pWnd->ModifyStyleEx(WS_EX_STATICEDGE, WS_EX_CLIENTEDGE);
		//}
	}
}

void ApplySystemFont(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		ApplySystemFont(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, _countof(szClassName)))
	{
		if (   __ascii_stricmp(szClassName, "SysListView32") == 0
			|| __ascii_stricmp(szClassName, "SysTreeView32") == 0)
		{
			pWnd->SendMessage(WM_SETFONT, NULL, FALSE);
		}
	}
}

static bool s_bIsXPStyle;

void FlatWindowStyles(CWnd* pWnd)
{
	CWnd* pWndChild = pWnd->GetWindow(GW_CHILD);
	while (pWndChild)
	{
		FlatWindowStyles(pWndChild);
		pWndChild = pWndChild->GetNextWindow();
	}

	CHAR szClassName[MAX_PATH];
	if (::GetClassNameA(*pWnd, szClassName, _countof(szClassName)))
	{
		if (__ascii_stricmp(szClassName, "Button") == 0)
		{
			if (!s_bIsXPStyle || (pWnd->GetStyle() & BS_ICON) == 0)
				pWnd->ModifyStyle(0, BS_FLAT);
		}
		else if (__ascii_stricmp(szClassName, "SysListView32") == 0)
		{
			pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
		}
		else if (__ascii_stricmp(szClassName, "SysTreeView32") == 0)
		{
			pWnd->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
		}
	}
}

void InitWindowStyles(CWnd* pWnd)
{
	//ApplySystemFont(pWnd);
	if (thePrefs.GetStraightWindowStyles() < 0)
		return;
	else if (thePrefs.GetStraightWindowStyles() > 0)
		/*StraightWindowStyles(pWnd)*/;	// no longer needed
	else
	{
		s_bIsXPStyle = g_xpStyle.IsAppThemed() && g_xpStyle.IsThemeActive();
		if (!s_bIsXPStyle)
			FlatWindowStyles(pWnd);
	}
}

LRESULT CemuleDlg::OnVersionCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acVCDNSBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;		
				// last byte contains informations about mirror urls, to avoid effects of future DDoS Attacks against eMules Homepage
				thePrefs.SetWebMirrorAlertLevel((uint8)(dwResult >> 24));
				uint8 abyCurVer[4] = { (uint8)(CemuleApp::m_nVersionBld + 1), (uint8)(CemuleApp::m_nVersionUpd), (uint8)(CemuleApp::m_nVersionMin), (uint8)0};
				dwResult &= 0x00FFFFFF;
				if (dwResult > *(uint32*)abyCurVer){
					if(theApp.IsSplash()) theApp.DestroySplash(); //Xman new slpash-screen arrangement
					SetActiveWindow();
#ifndef _BETA
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWVERSIONAVL));
					ShowNotifier(GetResString(IDS_NEWVERSIONAVLPOPUP), TBN_NEWVERSION);
					thePrefs.UpdateLastVC();
					if (!thePrefs.GetNotifierOnNewVersion()){
						if (AfxMessageBox(GetResString(IDS_NEWVERSIONAVL)+GetResString(IDS_VISITVERSIONCHECK),MB_YESNO)==IDYES) {
							CString theUrl;
							theUrl.Format(_T("/en/version_check.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
							theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
							ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
						}
					}
#else
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWVERSIONAVLBETA));
					if (AfxMessageBox(GetResString(IDS_NEWVERSIONAVLBETA)+GetResString(IDS_VISITVERSIONCHECK),MB_OK)==IDOK) {
						CString theUrl;
						theUrl.Format(_T("/en/download.php?version=%i&language=%i"),theApp.m_uCurVersionCheck,thePrefs.GetLanguageID());
						theUrl = thePrefs.GetVersionCheckBaseURL()+theUrl;
						ShellExecute(NULL, NULL, theUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
					}
#endif
				}
				else{
					thePrefs.UpdateLastVC();
					AddLogLine(true,GetResString(IDS_NONEWERVERSION));
				}
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_NEWVERSIONFAILED));
	return 0;
}

//Xman versions check
LRESULT CemuleDlg::OnMVersionCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	//Xman Info:
	//IP-samples:
	//1.5.4.99 --> Xtreme version 4.5 (.0)
	//2.5.4.99 --> Xtreme version 4.5.1 
	//1.6.4.99 --> Xtreme version 4.6 (.0) 
	//1.0.5.99 --> Xtreme version 5.0 (.0) 

	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acMVCDNSBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;
				uint8 abyCurVer[4] = { MOD_BUILD_VER , MOD_MIN_VER, MOD_MAIN_VER, 0};
				dwResult &= 0x00FFFFFF;
				thePrefs.UpdateLastMVC();
				if (dwResult > *(uint32*)abyCurVer){
					if(theApp.IsSplash()) theApp.DestroySplash(); //Xman new slpash-screen arrangement
					SetActiveWindow();
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWXTREMEVERSION));
					ShowNotifier(GetResString(IDS_NEWXTREMEVERSION), TBN_NEWMVERSION);
					m_bCheckwasDone=true;
					if (AfxMessageBox(GetResString(IDS_NEWXTREMEVERSIONDOWNLOAD),MB_YESNO)==IDYES) {
						ShellExecute(NULL, NULL, MOD_HPLINK, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
					}
				}
				else{
					AddLogLine(true,_T("No new Xtreme-Version found"));
				}
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_NEWVERSIONFAILED));
	return 0;
}

//MOD NOTE: if you are using DLP, don't remove/modify this versions-check
//Xman DLP
LRESULT CemuleDlg::OnDLPVersionCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	//Xman Info:
	//IP-samples:
	//5.0.0.99 --> DLP version 5
	//105.0.0.99 --> DLP version 105
	//0.1.0.99 --> DLP version 256 not allowed!
	//1.1.0.99 --> DLP version 257


	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acDLPBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;
				dwResult &= 0x00FFFFFF;
				thePrefs.UpdateLastMVC();
				if (theApp.dlp->IsDLPavailable() && dwResult > theApp.dlp->GetDLPVersion()){
					if(theApp.IsSplash()) theApp.DestroySplash(); //Xman new slpash-screen arrangement
					SetActiveWindow();
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWDLP));
					ShowNotifier(GetResString(IDS_NEWDLP), TBN_NEWMVERSION);
					if (m_bCheckwasDone==false && AfxMessageBox(GetResString(IDS_NEWDLPDOWNLOAD),MB_YESNO)==IDYES) {
						ShellExecute(NULL, NULL, MOD_DLPLINK, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT); //zz_fly :: update DLP link
					}
					m_bCheckwasDone=false;
				}
				else{
					AddLogLine(true,_T("No new DLP-Version found"));
				}
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_NEWVERSIONFAILED));
	return 0;
}
//Xman end (DLP and versionscheck)

//Xman new slpash-screen arrangement
//moved to emule.cpp
/*
void CemuleDlg::ShowSplash()
{
	ASSERT( m_pSplashWnd == NULL );
	if (m_pSplashWnd == NULL)
	{
		m_pSplashWnd = new CSplashScreen;
		if (m_pSplashWnd != NULL)
		{
			ASSERT(m_hWnd);
			if (m_pSplashWnd->Create(CSplashScreen::IDD, this))
			{
				m_pSplashWnd->ShowWindow(SW_SHOW);
				m_pSplashWnd->UpdateWindow();
				m_dwSplashTime = ::GetCurrentTime();
			}
			else
			{
				delete m_pSplashWnd;
				m_pSplashWnd = NULL;
			}
		}
	}
}

void CemuleDlg::DestroySplash()
{
	if (m_pSplashWnd != NULL)
	{
		m_pSplashWnd->DestroyWindow();
		delete m_pSplashWnd;
		m_pSplashWnd = NULL;
	}
#ifdef _BETA
	if (!thePrefs.IsFirstStart())
		AfxMessageBox(GetResString(IDS_BETANAG), MB_ICONINFORMATION | MB_OK, 0);
#endif
}
*/
//Xman end


BOOL CemuleApp::IsIdleMessage(MSG *pMsg)
{
	// This function is closely related to 'CemuleDlg::OnKickIdle'.
	//
	// * See MFC source code for 'CWnd::RunModalLoop' to see how those functions are related 
	//	 to each other.
	//
	// * See MFC documentation for 'CWnd::IsIdleMessage' to see why WM_TIMER messages are
	//	 filtered here.
	//
	// Generally we want to filter WM_TIMER messages because they are triggering idle
	// processing (e.g. cleaning up temp. MFC maps) and because they are occuring very often
	// in eMule (we have a rather high frequency timer in upload queue). To save CPU load but
	// do not miss the chance to cleanup MFC temp. maps and other stuff, we do not use each
	// occuring WM_TIMER message -- that would just be overkill! However, we can not simply
	// filter all WM_TIMER messages. If eMule is running in taskbar the only messages which
	// are received by main window are those WM_TIMER messages, thus those messages are the
	// only chance to trigger some idle processing. So, we must use at last some of those
	// messages because otherwise we would not do any idle processing at all in some cases.
	//

	static DWORD s_dwLastIdleMessage;
	if (pMsg->message == WM_TIMER)
	{
		// Allow this WM_TIMER message to trigger idle processing only if we did not do so
		// since some seconds.
		DWORD dwNow = GetTickCount();
		if (dwNow - s_dwLastIdleMessage >= SEC2MS(5))
		{
			s_dwLastIdleMessage = dwNow;
			return TRUE;// Request idle processing (will send a WM_KICKIDLE)
		}
		return FALSE;	// No idle processing
	}

	if (!CWinApp::IsIdleMessage(pMsg))
		return FALSE;	// No idle processing

	s_dwLastIdleMessage = GetTickCount();
	return TRUE;		// Request idle processing (will send a WM_KICKIDLE)
}

LRESULT CemuleDlg::OnKickIdle(UINT /*nWhy*/, long lIdleCount)
{
	LRESULT lResult = 0;

	//Xman new slpash-screen arrangement
	/*
	if (m_pSplashWnd)
	{
		if (::GetCurrentTime() - m_dwSplashTime > 2500)
		{
			// timeout expired, destroy the splash window
			DestroySplash();
	*/
	if (theApp.IsSplash() && theApp.spashscreenfinished)
	{
		if (::GetCurrentTime() - theApp.m_dwSplashTime > 3500) 
		{
			// timeout expired, destroy the splash window
			theApp.DestroySplash();
	//Xman end
			UpdateWindow();
		}
		else
		{
			// check again later...
			lResult = 1;
		}
	}

	if (m_bStartMinimized)
		PostStartupMinimized();

	if (searchwnd && searchwnd->m_hWnd)
	{
		if (theApp.m_app_state != APP_STATE_SHUTTINGDOWN)
		{
//#ifdef _DEBUG
//			TCHAR szDbg[80];
//			wsprintf(szDbg, L"%10u: lIdleCount=%d, %s", GetTickCount(), lIdleCount, (lIdleCount > 0) ? L"FreeTempMaps" : L"");
//			SetWindowText(szDbg);
//			TRACE(_T("%s\n"), szDbg);
//#endif
			// NOTE: See also 'CemuleApp::IsIdleMessage'. If 'CemuleApp::IsIdleMessage'
			// would not filter most of the WM_TIMER messages we might get a performance
			// problem here because the idle processing would be performed very, very often.
			//
			// The default MFC implementation of 'CWinApp::OnIdle' is sufficient for us. We
			// will get called with 'lIdleCount=0' and with 'lIdleCount=1'.
			//
			// CWinApp::OnIdle(0)	takes care about pending MFC GUI stuff and returns 'TRUE'
			//						to request another invocation to perform more idle processing
			// CWinApp::OnIdle(>=1)	frees temporary internally MFC maps and returns 'FALSE'
			//						because no more idle processing is needed.

			//Xman Code Improvement
			//enough to clean up handle maps every minute
			/*
			lResult = theApp.OnIdle(lIdleCount);
			*/
			static uint32 lastprocess;
			if(lIdleCount>0)
			{
				theApp.OnIdle(lIdleCount); //free maps
				lastprocess=::GetTickCount();
				return 0;
			}

			if(theApp.OnIdle(0 /*lIdleCount*/) && ::GetTickCount() - lastprocess > MIN2MS(1)) 
				lResult=1;
			else
				lResult=0;
			//Xman end
		}
	}

	return lResult;
}

int CemuleDlg::MapWindowToToolbarButton(CWnd* pWnd) const
{
	int iButtonID = -1;
	if (pWnd == transferwnd)        iButtonID = TBBTN_TRANSFERS;
	else if (pWnd == serverwnd)     iButtonID = TBBTN_SERVER;
	else if (pWnd == chatwnd)       iButtonID = TBBTN_MESSAGES;
	else if (pWnd == ircwnd)        iButtonID = TBBTN_IRC;
	else if (pWnd == sharedfileswnd)iButtonID = TBBTN_SHARED;
	else if (pWnd == searchwnd)     iButtonID = TBBTN_SEARCH;
	else if (pWnd == statisticswnd)	iButtonID = TBBTN_STATS;
	else if	(pWnd == kademliawnd)	iButtonID = TBBTN_KAD;
	else ASSERT(0);
	return iButtonID;
}

CWnd* CemuleDlg::MapToolbarButtonToWindow(int iButtonID) const
{
	CWnd* pWnd;
	switch (iButtonID)
	{
		case TBBTN_TRANSFERS:	pWnd = transferwnd;		break;
		case TBBTN_SERVER:		pWnd = serverwnd;		break;
		case TBBTN_MESSAGES:	pWnd = chatwnd;			break;
		case TBBTN_IRC:			pWnd = ircwnd;			break;
		case TBBTN_SHARED:		pWnd = sharedfileswnd;	break;
		case TBBTN_SEARCH:		pWnd = searchwnd;		break;
		case TBBTN_STATS:		pWnd = statisticswnd;	break;
		case TBBTN_KAD:			pWnd = kademliawnd;		break;
		default:				pWnd = NULL; ASSERT(0);
	}
	return pWnd;
}

bool CemuleDlg::IsWindowToolbarButton(int iButtonID) const
{
	switch (iButtonID)
	{
		case TBBTN_TRANSFERS:	return true;
		case TBBTN_SERVER:		return true;
		case TBBTN_MESSAGES:	return true;
		case TBBTN_IRC:			return true;
		case TBBTN_SHARED:		return true;
		case TBBTN_SEARCH:		return true;
		case TBBTN_STATS:		return true;
		case TBBTN_KAD:			return true;
	}
	return false;
}

int CemuleDlg::GetNextWindowToolbarButton(int iButtonID, int iDirection) const
{
	ASSERT( iDirection == 1 || iDirection == -1 );
	int iButtonCount = toolbar->GetButtonCount();
	if (iButtonCount > 0)
	{
		int iButtonIdx = toolbar->CommandToIndex(iButtonID);
		if (iButtonIdx >= 0 && iButtonIdx < iButtonCount)
		{
			int iEvaluatedButtons = 0;
			while (iEvaluatedButtons < iButtonCount)
			{
				iButtonIdx = iButtonIdx + iDirection;
				if (iButtonIdx < 0)
					iButtonIdx = iButtonCount - 1;
				else if (iButtonIdx >= iButtonCount)
					iButtonIdx = 0;

				TBBUTTON tbbt = {0};
				if (toolbar->GetButton(iButtonIdx, &tbbt))
				{
					if (IsWindowToolbarButton(tbbt.idCommand))
						return tbbt.idCommand;
				}
				iEvaluatedButtons++;
			}
		}
	}
	return -1;
}

BOOL CemuleDlg::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult = CTrayDialog::PreTranslateMessage(pMsg);

	//Xman new slpash-screen arrangement
	/*
	if (m_pSplashWnd && m_pSplashWnd->m_hWnd != NULL &&
	*/
	if (theApp.IsSplash() && 
	//Xman end
		(pMsg->message == WM_KEYDOWN	   ||
		 pMsg->message == WM_SYSKEYDOWN	   ||
		 pMsg->message == WM_LBUTTONDOWN   ||
		 pMsg->message == WM_RBUTTONDOWN   ||
		 pMsg->message == WM_MBUTTONDOWN   ||
		 pMsg->message == WM_NCLBUTTONDOWN ||
		 pMsg->message == WM_NCRBUTTONDOWN ||
		 pMsg->message == WM_NCMBUTTONDOWN))
	{
		//Xman new slpash-screen arrangement
		/*
		DestroySplash();
		*/
		theApp.DestroySplash();
		//Xman end
		UpdateWindow();
	}
	else
	{
		if (pMsg->message == WM_KEYDOWN)
		{
			// Handle Ctrl+Tab and Ctrl+Shift+Tab
			if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			{
				int iButtonID = MapWindowToToolbarButton(activewnd);
				if (iButtonID != -1)
				{
					int iNextButtonID = GetNextWindowToolbarButton(iButtonID, GetAsyncKeyState(VK_SHIFT) < 0 ? -1 : 1);
					if (iNextButtonID != -1)
					{
						CWnd* pWndNext = MapToolbarButtonToWindow(iNextButtonID);
						if (pWndNext)
							SetActiveDialog(pWndNext);
					}
				}
			}
		}
	}

	return bResult;
}

void CemuleDlg::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	ASSERT(pApp->m_pszHelpFilePath != NULL);
	// to call HtmlHelp the m_fUseHtmlHelp must be set in
	// the application's constructor
	ASSERT(pApp->m_eHelpType == afxHTMLHelp);

	CWaitCursor wait;

	PrepareForHelp();

	// need to use top level parent (for the case where m_hWnd is in DLL)
	CWnd* pWnd = GetTopLevelParent();

	TRACE(traceAppMsg, 0, _T("HtmlHelp: pszHelpFile = '%s', dwData: $%lx, fuCommand: %d.\n"), pApp->m_pszHelpFilePath, dwData, nCmd);

	bool bHelpError = false;
	CString strHelpError;
	int iTry = 0;
	while (iTry++ < 2)
	{
		if (!AfxHtmlHelp(pWnd->m_hWnd, pApp->m_pszHelpFilePath, nCmd, dwData))
		{
			bHelpError = true;
			strHelpError.LoadString(AFX_IDP_FAILED_TO_LAUNCH_HELP);

			typedef struct tagHH_LAST_ERROR
			{
				int      cbStruct;
				HRESULT  hr;
				BSTR     description;
			} HH_LAST_ERROR;
			HH_LAST_ERROR hhLastError = {0};
			hhLastError.cbStruct = sizeof hhLastError;
			HWND hwndResult = AfxHtmlHelp(pWnd->m_hWnd, NULL, HH_GET_LAST_ERROR, reinterpret_cast<DWORD>(&hhLastError));
			if (hwndResult != 0)
			{
				if (FAILED(hhLastError.hr))
				{
					if (hhLastError.description)
					{
						strHelpError = hhLastError.description;
						::SysFreeString(hhLastError.description);
					}
					if (   hhLastError.hr == 0x8004020A  /*no topics IDs available in Help file*/
						|| hhLastError.hr == 0x8004020B) /*requested Help topic ID not found*/
					{
						// try opening once again without help topic ID
						if (nCmd != HH_DISPLAY_TOC)
						{
							nCmd = HH_DISPLAY_TOC;
							dwData = 0;
							continue;
						}
					}
				}
			}
			break;
		}
		else
		{
			bHelpError = false;
			strHelpError.Empty();
			break;
		}
	}

	if (bHelpError)
	{
		if (AfxMessageBox(CString(pApp->m_pszHelpFilePath) + _T("\n\n") + strHelpError + _T("\n\n") + GetResString(IDS_ERR_NOHELP), MB_YESNO | MB_ICONERROR) == IDYES)
		{
			CString strUrl = thePrefs.GetHomepageBaseURL() + _T("/home/perl/help.cgi");
			ShellExecute(NULL, NULL, strUrl, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
		}
	}
}

LRESULT CemuleDlg::OnPeerCacheResponse(WPARAM wParam, LPARAM lParam)
{
	return theApp.m_pPeerCache->OnPeerCacheCheckResponse(wParam,lParam);
}

void CemuleDlg::CreateToolbarCmdIconMap()
{
	m_mapTbarCmdToIcon.SetAt(TBBTN_CONNECT, _T("Connect"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_KAD, _T("Kademlia"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SERVER, _T("Server"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_TRANSFERS, _T("Transfer"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SEARCH, _T("Search"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_SHARED, _T("SharedFiles"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_MESSAGES, _T("Messages"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_IRC, _T("IRC"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_STATS, _T("Statistics"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_OPTIONS, _T("Preferences"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_TOOLS, _T("Tools"));
	m_mapTbarCmdToIcon.SetAt(TBBTN_HELP, _T("Help"));
}

LPCTSTR CemuleDlg::GetIconFromCmdId(UINT uId)
{
	LPCTSTR pszIconId = NULL;
	if (m_mapTbarCmdToIcon.Lookup(uId, pszIconId))
		return pszIconId;
	return NULL;
}

BOOL CemuleDlg::OnChevronPushed(UINT id, NMHDR* pNMHDR, LRESULT* plResult)
{
	UNREFERENCED_PARAMETER(id);
	if (!thePrefs.GetUseReBarToolbar())
		return FALSE;

	NMREBARCHEVRON* pnmrc = (NMREBARCHEVRON*)pNMHDR;

	ASSERT( id == AFX_IDW_REBAR );
	ASSERT( pnmrc->uBand == 0 );
	ASSERT( pnmrc->wID == 0 );
	ASSERT( m_mapTbarCmdToIcon.GetSize() != 0 );

	// get visible area of rebar/toolbar
	CRect rcVisibleButtons;
	toolbar->GetClientRect(&rcVisibleButtons);

	// search the first toolbar button which is not fully visible
	int iButtons = toolbar->GetButtonCount();
	int i;
	for (i = 0; i < iButtons; i++)
	{
		CRect rcButton;
		toolbar->GetItemRect(i, &rcButton);

		CRect rcVisible;
		if (!rcVisible.IntersectRect(&rcVisibleButtons, &rcButton) || !EqualRect(rcButton, rcVisible))
			break;
	}

	// create menu for all toolbar buttons which are not (fully) visible
	BOOL bLastMenuItemIsSep = TRUE;
	CTitleMenu menu;
	menu.CreatePopupMenu();
	menu.AddMenuTitle(_T("eMule"), true);
	while (i < iButtons)
	{
		TCHAR szString[256];
		szString[0] = _T('\0');
		TBBUTTONINFO tbbi = {0};
		tbbi.cbSize = sizeof tbbi;
		tbbi.dwMask = TBIF_BYINDEX | TBIF_COMMAND | TBIF_STYLE | TBIF_STATE | TBIF_TEXT;
		tbbi.cchText = _countof(szString);
		tbbi.pszText = szString;
		if (toolbar->GetButtonInfo(i, &tbbi) != -1)
		{
			szString[_countof(szString) - 1] = _T('\0');
			if (tbbi.fsStyle & TBSTYLE_SEP)
			{
				if (!bLastMenuItemIsSep)
					bLastMenuItemIsSep = menu.AppendMenu(MF_SEPARATOR, 0, (LPCTSTR)NULL);
			}
			else
			{
				if (szString[0] != _T('\0') && menu.AppendMenu(MF_STRING, tbbi.idCommand, szString, GetIconFromCmdId(tbbi.idCommand)))
				{
					bLastMenuItemIsSep = FALSE;
					if (tbbi.fsState & TBSTATE_CHECKED)
						menu.CheckMenuItem(tbbi.idCommand, MF_BYCOMMAND | MF_CHECKED);
					if ((tbbi.fsState & TBSTATE_ENABLED) == 0)
						menu.EnableMenuItem(tbbi.idCommand, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
				}
			}
		}

		i++;
	}

	CPoint ptMenu(pnmrc->rc.left, pnmrc->rc.top);
	ClientToScreen(&ptMenu);
	ptMenu.y += rcVisibleButtons.Height();
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptMenu.x, ptMenu.y, this);
	VERIFY( menu.DestroyMenu() ); // XP Style Menu [Xanatos] - Stulle
	*plResult = 1;
	return FALSE;
}

bool CemuleDlg::IsPreferencesDlgOpen() const
{
	return (preferenceswnd->m_hWnd != NULL);
}

int CemuleDlg::ShowPreferences(UINT uStartPageID)
{
	if (IsPreferencesDlgOpen())
	{
		preferenceswnd->SetForegroundWindow();
		preferenceswnd->BringWindowToTop();
		return -1;
	}
	else
	{
		if (uStartPageID != (UINT)-1)
			preferenceswnd->SetStartPage(uStartPageID);
		return preferenceswnd->DoModal();
	}
}



//////////////////////////////////////////////////////////////////
// Webserver related

LRESULT CemuleDlg::OnWebAddDownloads(WPARAM wParam, LPARAM lParam)
{
	CString link=CString((TCHAR*)wParam);
	if (link.GetLength()==32 && link.Left(4).CompareNoCase(_T("ed2k"))!=0) {
		uchar fileid[16];
		DecodeBase16(link, link.GetLength(), fileid, _countof(fileid));
		theApp.searchlist->AddFileToDownloadByHash(fileid,(uint8)lParam);

	} else
		theApp.AddEd2kLinksToDownload((TCHAR*)wParam, (int)lParam);

	return 0;
}

LRESULT CemuleDlg::OnAddRemoveFriend(WPARAM wParam, LPARAM lParam)
{
	if (lParam==0) { // remove
		theApp.friendlist->RemoveFriend((CFriend*)wParam);
	} else {		// add
		theApp.friendlist->AddFriend((CUpDownClient*)wParam);
	}

	return 0;
}

LRESULT CemuleDlg::OnWebSetCatPrio(WPARAM wParam, LPARAM lParam)
{
	theApp.downloadqueue->SetCatPrio(wParam,(uint8)lParam);
	return 0;
}
LRESULT CemuleDlg::OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam)
{
	if(!wParam)
	{
		int cat=(int)lParam;
		transferwnd->downloadlistctrl.ClearCompleted(cat);
	}
	else
	{
		uchar* pFileHash = reinterpret_cast<uchar*>(lParam);
		CKnownFile* file=theApp.knownfiles->FindKnownFileByID(pFileHash);
		if (file)
			transferwnd->downloadlistctrl.RemoveFile((CPartFile*)file);
		delete[] pFileHash;
	}

	return 0;
}

LRESULT CemuleDlg::OnWebServerFileRename(WPARAM wParam, LPARAM lParam)
{
	CString sNewName = ((LPCTSTR)(lParam));

	((CPartFile*)wParam)->SetFileName(sNewName);
	((CPartFile*)wParam)->SavePartFile();
	((CPartFile*)wParam)->UpdateDisplayedInfo();
	sharedfileswnd->sharedfilesctrl.UpdateFile( (CKnownFile*)((CPartFile*)wParam));

	return 0;
}

LRESULT CemuleDlg::OnWebGUIInteraction(WPARAM wParam, LPARAM lParam) {

	switch (wParam) {
		case WEBGUIIA_UPDATEMYINFO:
			serverwnd->UpdateMyInfo();
			break;
		case WEBGUIIA_WINFUNC:{
			if (thePrefs.GetWebAdminAllowedHiLevFunc())
			{
				try {
					HANDLE hToken;
					TOKEN_PRIVILEGES tkp;	// Get a token for this process.
					
					if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
						throw;
					LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
					tkp.PrivilegeCount = 1;  // one privilege to set
					tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;	// Get the shutdown privilege for this process.
					AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

					if (lParam==1) {	// shutdown
						ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
					} else 
					if (lParam==2) {
						ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
					}

				} catch(...)
					{
						AddLogLine(true, GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_FAILED));
				}
			}
			else 
				AddLogLine(true, GetResString(IDS_WEB_REBOOT) + _T(' ') + GetResString(IDS_ACCESSDENIED));
			break;
		}
		case WEBGUIIA_UPD_CATTABS:
			theApp.emuledlg->transferwnd->UpdateCatTabTitles();
			break;
		case WEBGUIIA_UPD_SFUPDATE: {
			CKnownFile* kf=(CKnownFile*)lParam;
			if (kf)
				theApp.sharedfiles->UpdateFile(kf);
			}
			break;
		case WEBGUIIA_UPDATESERVER:
			serverwnd->serverlistctrl.RefreshServer((CServer*)lParam);
			break;
		case WEBGUIIA_STOPCONNECTING:
			theApp.serverconnect->StopConnectionTry();
			break;
		case WEBGUIIA_CONNECTTOSERVER: {
			CServer* server=(CServer*)lParam;
			if (server==NULL) 
				theApp.serverconnect->ConnectToAnyServer();
			else 
				theApp.serverconnect->ConnectToServer(server);
			break;
			}
		case WEBGUIIA_DISCONNECT:
			if (lParam!=2)	// !KAD
				theApp.serverconnect->Disconnect();
			if (lParam!=1)	// !ED2K
				Kademlia::CKademlia::Stop();
			break;

		case WEBGUIIA_SERVER_REMOVE: {
			serverwnd->serverlistctrl.RemoveServer((CServer*)lParam);
			break;
		}
		case WEBGUIIA_SHARED_FILES_RELOAD: {
			theApp.sharedfiles->Reload();
			break;
		}
		case WEBGUIIA_ADD_TO_STATIC: {
			serverwnd->serverlistctrl.StaticServerFileAppend((CServer*)lParam);
			break;
		}
		case WEBGUIIA_REMOVE_FROM_STATIC: {
			serverwnd->serverlistctrl.StaticServerFileRemove((CServer*)lParam);
			break;
		}
		case WEBGUIIA_UPDATESERVERMETFROMURL:
			theApp.emuledlg->serverwnd->UpdateServerMetFromURL((TCHAR*)lParam);
			break;
		case WEBGUIIA_SHOWSTATISTICS:
			theApp.emuledlg->statisticswnd->ShowStatistics(lParam!=0);
			break;
		case WEBGUIIA_DELETEALLSEARCHES:
			theApp.emuledlg->searchwnd->DeleteAllSearches();
			break;

		case WEBGUIIA_KAD_BOOTSTRAP:{
			CString dest=CString((TCHAR*)lParam);
			int pos=dest.Find(_T(':'));
			if (pos!=-1) {
				uint16 port = (uint16)_tstoi(dest.Right(dest.GetLength() - pos - 1));
				CString ip = dest.Left(pos);
				// JOHNTODO - Switch between Kad1 and Kad2
				Kademlia::CKademlia::Bootstrap(ip, port, true);
			}
			break;
		}
		case WEBGUIIA_KAD_START:
			Kademlia::CKademlia::Start();
			break;
		case WEBGUIIA_KAD_STOP:
			Kademlia::CKademlia::Stop();
			break;
		case WEBGUIIA_KAD_RCFW:
			Kademlia::CKademlia::RecheckFirewalled();
			break;
	}

	return 0;
}

void CemuleDlg::TrayMinimizeToTrayChange()
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		if (!thePrefs.GetMinToTray())
		{
			// just for safety, ensure that we are not adding duplicate menu entries..
			if (pSysMenu->EnableMenuItem(MP_MINIMIZETOTRAY, MF_BYCOMMAND | MF_ENABLED) == -1)
			{
				ASSERT( (MP_MINIMIZETOTRAY & 0xFFF0) == MP_MINIMIZETOTRAY && MP_MINIMIZETOTRAY < 0xF000);
				VERIFY( pSysMenu->InsertMenu(SC_MINIMIZE, MF_BYCOMMAND, MP_MINIMIZETOTRAY, GetResString(IDS_PW_TRAY)) );
			}
			else
				ASSERT(0);
		}
		else
		{
			(void)pSysMenu->RemoveMenu(MP_MINIMIZETOTRAY, MF_BYCOMMAND);
		}
	}
	CTrayDialog::TrayMinimizeToTrayChange();
}

void CemuleDlg::SetToolTipsDelay(UINT uMilliseconds)
{
	//searchwnd->SetToolTipsDelay(uMilliseconds);
	transferwnd->SetToolTipsDelay(uMilliseconds);
	sharedfileswnd->SetToolTipsDelay(uMilliseconds);
}

// ==> UPnP support [MoNKi] - leuk_he
//void CemuleDlg::UPnPTimeOutTimer(HWND /*hwnd*/, UINT /*uiMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/){
/*
	::PostMessage(theApp.emuledlg->GetSafeHwnd(), UM_UPNP_RESULT, (WPARAM)CUPnPImpl::UPNP_TIMEOUT, 0);
}

LRESULT CemuleDlg::OnUPnPResult(WPARAM wParam, LPARAM lParam){
	bool bWasRefresh = lParam != 0;
	if (!bWasRefresh && wParam == CUPnPImpl::UPNP_FAILED){		
		// UPnP failed, check if we can retry it with another implementation
		if (theApp.m_pUPnPFinder->SwitchImplentation()){
			StartUPnP(false);
			return 0;
		}
		else
			DebugLog(_T("No more available UPnP implementations left"));

	}
	if (m_hUPnPTimeOutTimer != 0){
		VERIFY( ::KillTimer(NULL, m_hUPnPTimeOutTimer) );
		m_hUPnPTimeOutTimer = 0;
	}
	if(IsRunning() && m_bConnectRequestDelayedForUPnP){
		StartConnection();
	}
	if (!bWasRefresh && wParam == CUPnPImpl::UPNP_OK){
		// remember the last working implementation
		thePrefs.SetLastWorkingUPnPImpl(theApp.m_pUPnPFinder->GetImplementation()->GetImplementationID());
		Log(GetResString(IDS_UPNPSUCCESS), theApp.m_pUPnPFinder->GetImplementation()->GetUsedTCPPort()
			, theApp.m_pUPnPFinder->GetImplementation()->GetUsedUDPPort());
	}
	else if (!bWasRefresh)
		LogWarning(GetResString(IDS_UPNPFAILED));

	serverwnd->UpdateMyInfo(); //zz_fly :: show UPnP status
	return 0;
}
*/
// <== UPnP support [MoNKi] - leuk_he

LRESULT  CemuleDlg::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	//DebugLog(_T("DEBUG:Power state change. wParam=%d lPararm=%ld"),wParam,lParam);
	switch (wParam) {
		case PBT_APMRESUMEAUTOMATIC:
		{
			if (m_bEd2kSuspendDisconnect || m_bKadSuspendDisconnect)
			{
				DebugLog(_T("Reconnect after Power state change. wParam=%d lPararm=%ld"),wParam,lParam);
				// ==> UPnP support [MoNKi] - leuk_he
				/*
				RefreshUPnP(true);
				*/
				// <== UPnP support [MoNKi] - leuk_he
				PostMessage(WM_SYSCOMMAND , MP_CONNECT, 0); // tell to connect.. a sec later...
			}
			return TRUE; // message processed.
			break;
		}
		case PBT_APMSUSPEND:
		{		
			// ==> Proper English [ginger] - Stulle
			/*
			DebugLog(_T("System is going is suspending operation, disconnecting. wParam=%d lPararm=%ld"),wParam,lParam);
			*/
			DebugLog(_T("System is suspending operation, disconnecting. wParam=%d lPararm=%ld"),wParam,lParam);
			// <== Proper English [ginger] - Stulle
			m_bEd2kSuspendDisconnect = theApp.serverconnect->IsConnected();
			m_bKadSuspendDisconnect = Kademlia::CKademlia::IsConnected();
			CloseConnection();
			return TRUE; // message processed.
			break;
		}
		default:
			return FALSE; // we do not process this message
	}

}

// ==> UPnP support [MoNKi] - leuk_he
/*
void CemuleDlg::StartUPnP(bool bReset, uint16 nForceTCPPort, uint16 nForceUDPPort) {
	if (theApp.m_pUPnPFinder != NULL && (m_hUPnPTimeOutTimer == 0 || !bReset)){
		if (bReset){
			theApp.m_pUPnPFinder->Reset();
			Log(GetResString(IDS_UPNPSETUP));
		}
		try
		{
			if (theApp.m_pUPnPFinder->GetImplementation()->IsReady()){
				theApp.m_pUPnPFinder->GetImplementation()->SetMessageOnResult(GetSafeHwnd(), UM_UPNP_RESULT);
				if (bReset)
					VERIFY( (m_hUPnPTimeOutTimer = ::SetTimer(NULL, NULL, SEC2MS(40), UPnPTimeOutTimer)) != NULL );
				theApp.m_pUPnPFinder->GetImplementation()->StartDiscovery(((nForceTCPPort != 0) ? nForceTCPPort : thePrefs.GetPort())
					, ((nForceUDPPort != 0) ? nForceUDPPort :thePrefs.GetUDPPort())
					, thePrefs.GetWSUseUPnP() ? thePrefs.GetWSPort() : 0);
			}
			else
				::PostMessage(theApp.emuledlg->GetSafeHwnd(), UM_UPNP_RESULT, (WPARAM)CUPnPImpl::UPNP_FAILED, 0);
		}
		catch ( CUPnPImpl::UPnPError& ) {}
		catch ( CException* e ) { e->Delete(); }
	}
	else
		ASSERT( false );
}

void CemuleDlg::RefreshUPnP(bool bRequestAnswer)
{
	if (!thePrefs.IsUPnPEnabled())
		return;
	if (theApp.m_pUPnPFinder != NULL && m_hUPnPTimeOutTimer == 0){
		try
		{
			if (theApp.m_pUPnPFinder->GetImplementation()->IsReady())
			{
				if (bRequestAnswer)
					theApp.m_pUPnPFinder->GetImplementation()->SetMessageOnResult(GetSafeHwnd(), UM_UPNP_RESULT);
				if (theApp.m_pUPnPFinder->GetImplementation()->CheckAndRefresh() && bRequestAnswer)
				{
					VERIFY( (m_hUPnPTimeOutTimer = ::SetTimer(NULL, NULL, SEC2MS(10), UPnPTimeOutTimer)) != NULL );
				}
				else
					theApp.m_pUPnPFinder->GetImplementation()->SetMessageOnResult(0, 0);
			}
			else
				DebugLogWarning(_T("RefreshUPnP, implementation not ready"));
		}
		catch ( CUPnPImpl::UPnPError& ) {}
		catch ( CException* e ) { e->Delete(); }
	}
	else
		ASSERT( false );
}
*/
// <== UPnP support [MoNKi] - leuk_he

BOOL CemuleDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	// WM_DEVICECHANGE is sent for:
	//	Drives which where created/deleted with "SUBST" command (handled like network drives)
	//	Drives which where created/deleted as regular network drives.
	//
	// WM_DEVICECHANGE is *NOT* sent for:
	//	Floppy disk drives
	//	ZIP disk drives (although Windows Explorer recognizes a changed media, we do not get a message)
	//	CD-ROM drives (although MSDN says different...)
	//
	if ((nEventType == DBT_DEVICEARRIVAL || nEventType == DBT_DEVICEREMOVECOMPLETE) && !IsBadReadPtr((void *)dwData, sizeof(DEV_BROADCAST_HDR)))
	{
#ifdef _DEBUG
		CString strMsg;
		if (nEventType == DBT_DEVICEARRIVAL)
			strMsg += _T("DBT_DEVICEARRIVAL");
		else if (nEventType == DBT_DEVICEREMOVECOMPLETE)
			strMsg += _T("DBT_DEVICEREMOVECOMPLETE");
#endif

		const DEV_BROADCAST_HDR *pHdr = (DEV_BROADCAST_HDR *)dwData;
		if (pHdr->dbch_devicetype == DBT_DEVTYP_VOLUME && !IsBadReadPtr((void *)dwData, sizeof(DEV_BROADCAST_VOLUME)))
		{
			const DEV_BROADCAST_VOLUME *pVol = (DEV_BROADCAST_VOLUME *)pHdr;
#ifdef _DEBUG
			strMsg += _T(" Volume");
			if (pVol->dbcv_flags & DBTF_MEDIA)
				strMsg += _T(" Media");
			if (pVol->dbcv_flags & DBTF_NET)
				strMsg += _T(" Net");
			if ((pVol->dbcv_flags & ~(DBTF_NET | DBTF_MEDIA)) != 0)
				strMsg.AppendFormat(_T(" flags=0x%08x"), pVol->dbcv_flags);
#endif

			bool bVolumesChanged = false;
			for (UINT uDrive = 0; uDrive <= 25; uDrive++)
			{
				UINT uMask = 1 << uDrive;
				if (pVol->dbcv_unitmask & uMask)
				{
					DEBUG_ONLY( strMsg.AppendFormat(_T(" %c:"), _T('A') + uDrive) );
					if (pVol->dbcv_flags & (DBTF_MEDIA | DBTF_NET))
						ClearVolumeInfoCache(uDrive);
					bVolumesChanged = true;
				}
			}
			if (bVolumesChanged && sharedfileswnd)
				sharedfileswnd->OnVolumesChanged();

			// ==> Automatic shared files updater [MoNKi] - Stulle
			if(thePrefs.GetDirectoryWatcher())
			{
				// Get the drive char
				// Note, this is not so nice because we do something similar above
				// and we might even miss a changed drive - the message calling this
				// function can also be send for more than one changed drive at once -
				// but still this is better than having more complex structures to
				// figure out if we should reload.
				char drive;
				ULONG umask = pVol->dbcv_unitmask;
				for (drive = 'a'; drive <= 'z'; drive++)
				{
					if (umask & 0x1)
						break;
					umask = umask >> 1;
				}
				// Check if there are shared folders in this drive

				// Get all shared directories
				CStringList dirList;
				CString curDir;
				
				// Incoming Dir
				curDir=thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR);
				dirList.AddTail( curDir );

				// Categories dirs
				for (int i=1; i < thePrefs.GetCatCount(); i++)
				{
					curDir=CString( thePrefs.GetCatPath(i) );

					if( dirList.Find( curDir ) == NULL ) {
						dirList.AddTail( curDir );
					}
				}

				// The other shared dirs
				POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
				while(pos){
					curDir = thePrefs.shareddir_list.GetNext(pos);

					if( dirList.Find( curDir ) == NULL ) {
						dirList.AddTail( curDir );
					}
				}

				// Dirs of single shared files
				if(thePrefs.GetSingleSharedDirWatcher()/* && theApp.sharedfiles->ProbablyHaveSingleSharedFiles()*/)
				{
					for (POSITION pos = theApp.sharedfiles->m_liSingleSharedFiles.GetHeadPosition(); pos != NULL; theApp.sharedfiles->m_liSingleSharedFiles.GetNext(pos))
					{
						curDir = theApp.sharedfiles->m_liSingleSharedFiles.GetAt(pos);
						curDir = curDir.Left(curDir.ReverseFind(_T('\\'))+1);

						if( dirList.Find( curDir ) == NULL ) {
							dirList.AddTail( curDir );
						}
					}
				}

				bool reload = false;

				// Checks if a shared drive is in this volume
				// or if it don't exits
				pos = dirList.GetHeadPosition();
				while(pos){
					curDir = dirList.GetNext(pos);
					if (curDir.Right(1)==_T("\\"))
						curDir = curDir.Left(curDir.GetLength() - 1);

					//this is a dir and not a drive
					if (curDir.Right(1) != _T(":")){
						if(CFileFind().FindFile(curDir) == FALSE){
							reload = true;
							pos = NULL;
						}
					}

					//this is a drive
					if(curDir.MakeLower().GetAt(0) == _TCHAR(drive)){
						reload = true;
						pos = NULL;
					}
				}

				if(reload){
					// Reload shared files
					if(theApp.emuledlg->IsRunning()){
						theApp.DirectoryWatcherExternalReload();
					}
				}
			}
			// <== Automatic shared files updater [MoNKi] - Stulle
		}
		else
		{
			DEBUG_ONLY( strMsg.AppendFormat(_T(" devicetype=0x%08x"), pHdr->dbch_devicetype) );
		}
#ifdef _DEBUG
		TRACE(_T("CemuleDlg::OnDeviceChange: %s\n"), strMsg);
#endif
	}
	else
		TRACE(_T("CemuleDlg::OnDeviceChange: nEventType=0x%08x  dwData=0x%08x\n"), nEventType, dwData);
	return __super::OnDeviceChange(nEventType, dwData);
}

//Xman process timer code via messages (Xanatos)
// Note: the timers does not crash on a exception so I use the messags to call the functions so when an error aprears it will be detected
afx_msg LRESULT CemuleDlg::DoTimer(WPARAM wParam, LPARAM /*lParam*/)
{
	if (!theApp.emuledlg->IsRunning())
		return 0;

	if(wParam == NULL)
		theApp.uploadqueue->UploadTimer();
	/* only used for uploadtimer
	else if(wParam == -1)
		theApp.emuledlg->StartupTimer();
	*/
	else
		ASSERT(0);
	return 0;
}

// ==> ScarAngel Version Check - Stulle
void CemuleDlg::DoSVersioncheck(bool manual) {
	if (!manual && thePrefs.GetLastSVC()!=0) {
		CTime last(thePrefs.GetLastSVC());
		struct tm tmTemp;
		time_t tLast=safe_mktime(last.GetLocalTm(&tmTemp));
		time_t tNow=safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
		if ( (difftime(tNow,tLast) / 86400)<thePrefs.GetUpdateDays() )
			return;
	}
	// ==> Mephisto mod [Stulle] - Mephisto
	/*
	if (WSAAsyncGetHostByName(m_hWnd, UM_SVERSIONCHECK_RESPONSE, "scarvercheck.dyndns.info", m_acSVCDNSBuffer, sizeof(m_acSVCDNSBuffer)) == 0){
	*/
	if (WSAAsyncGetHostByName(m_hWnd, UM_SVERSIONCHECK_RESPONSE, "mephvercheck.dyndns.info", m_acSVCDNSBuffer, sizeof(m_acSVCDNSBuffer)) == 0){
	// <== Mephisto mod [Stulle] - Mephisto
		AddLogLine(true,GetResString(IDS_NEWVERSIONFAILED));
	}
}

LRESULT CemuleDlg::OnSVersionCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acSVCDNSBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;
				uint8 abyCurVer[4] = { (uint8)(CemuleApp::m_nMVersionBld + 1), (uint8)(CemuleApp::m_nMVersionMin), (uint8)(CemuleApp::m_nMVersionMjr), 0};
				dwResult &= 0x00FFFFFF;
				if (dwResult > *(uint32*)abyCurVer){
					thePrefs.UpdateLastSVC();
					SetActiveWindow();
					// ==> Mephisto mod [Stulle] - Mephisto
					/*
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWSVERSIONAVL));
					ShowNotifier(GetResString(IDS_NEWSVERSIONAVLPOPUP), TBN_NEWSVERSION);
					if (AfxMessageBox(GetResString(IDS_NEWSVERSIONAVL)+GetResString(IDS_VISITSVERSIONCHECK),MB_YESNO)==IDYES) {
						ShellExecute(NULL, NULL, _T("http://scarangel.sourceforge.net/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
					*/
					Log(LOG_SUCCESS|LOG_STATUSBAR,GetResString(IDS_NEWMVERSIONAVL));
					ShowNotifier(GetResString(IDS_NEWMVERSIONAVLPOPUP), TBN_NEWSVERSION);
					if (AfxMessageBox(GetResString(IDS_NEWMVERSIONAVL)+GetResString(IDS_VISITMVERSIONCHECK),MB_YESNO)==IDYES) {
						ShellExecute(NULL, NULL, _T("http://mephisto.emule-web.de/"), NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
					// <== Mephisto mod [Stulle] - Mephisto
					}
				}
				else{
					thePrefs.UpdateLastSVC();
					// ==> Mephisto mod [Stulle] - Mephisto
					/*
					AddLogLine(true,GetResString(IDS_NONEWSVERVERSION));
					*/
					AddLogLine(true,GetResString(IDS_NONEWMVERVERSION));
					// <== Mephisto mod [Stulle] - Mephisto
				}
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_NEWVERSIONFAILED));
	return 0;
}
// <== ScarAngel Version Check - Stulle

// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
void CemuleDlg::SaveSettings (bool _shutdown) {
	if (_shutdown) {
		theApp.emuledlg->preferenceswnd->m_wndSecurity.DeleteDDB();
	}

	// ==> Threaded Known Files Saving [Stulle] - Stulle
	if (theApp.knownfiles->m_SaveKnownThread) // we just saved something
	{
		theApp.knownfiles->m_SaveKnownThread->EndThread();
		delete theApp.knownfiles->m_SaveKnownThread;
		theApp.knownfiles->m_SaveKnownThread = NULL;
	}
	else // we might have missed something
	// <== Threaded Known Files Saving [Stulle] - Stulle
		theApp.knownfiles->Save();
	theApp.sharedfiles->Save();
	//transferwnd->downloadlistctrl.SaveSettings();
	//transferwnd->downloadclientsctrl.SaveSettings();
	//transferwnd->uploadlistctrl.SaveSettings();
	//transferwnd->queuelistctrl.SaveSettings();
	//transferwnd->clientlistctrl.SaveSettings();
	//sharedfileswnd->sharedfilesctrl.SaveSettings();
	//chatwnd->m_FriendListCtrl.SaveSettings();
	searchwnd->SaveAllSettings();
	serverwnd->SaveAllSettings();
	kademliawnd->SaveAllSettings();
	//sharedfileswnd->historylistctrl.SaveSettings(CPreferences::tableHistory); //Xman [MoNKi: -Downloaded History-]

	//Xman new adapter selection
	if(_shutdown && theApp.pBandWidthControl->GetwasNAFCLastActive()==true)
		thePrefs.SetNAFCFullControl(true);
	//Xman end

	//Xman don't overwrite bak files if last sessions crashed
	//remark: it would be better to set the flag after all deletions, but this isn't possible, because the prefs need access to the objects when saving
	if (_shutdown) {
		thePrefs.m_this_session_aborted_in_an_unnormal_way=false;
	}
	//Xman end

	theApp.m_pPeerCache->Save();
	if (_shutdown) {
		theApp.scheduler->RestoreOriginals();
		theApp.searchlist->SaveSpamFilter();
		if (thePrefs.IsStoringSearchesEnabled())
			theApp.searchlist->StoreSearches();
	}
	thePrefs.Save();
	if (_shutdown) {
		thePerfLog.Shutdown();
	}
	theApp.scheduler->SaveToFile();
}
// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

// ==> XP Style Menu [Xanatos] - Stulle
void CemuleDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	HMENU hMenu = AfxGetThreadState()->m_hTrackingMenu;
	if(CMenu *pMenu = CMenu::FromHandle(hMenu))
		pMenu->MeasureItem(lpMeasureItemStruct);
	
	if(nIDCtl)
		CTrayDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}
// <== XP Style Menu [Xanatos] - Stulle

// ==> Invisible Mode [TPT/MoNKi] - Stulle
LRESULT CemuleDlg::OnHotKey(WPARAM wParam, LPARAM /*lParam*/)
{
	if(wParam == HOTKEY_INVISIBLEMODE_ID)
		b_HideApp = !b_HideApp;
		if(b_HideApp)
		{
			EnumWindows(AskEmulesForInvisibleMode, INVMODE_HIDEWINDOW);
		}
		else
		{
			EnumWindows(AskEmulesForInvisibleMode, INVMODE_RESTOREWINDOW);
		}
	return 0;
}

void CemuleDlg::ToggleHide()
{
	// ==> Static Tray Icon [MorphXT] - MyTh88
	m_bMaximized = IsZoomed();
	// <== Static Tray Icon [MorphXT] - MyTh88
	m_bTbhMiniMuleVis = theApp.minimule->IsWindowVisible(); // TBH: minimule - Stulle
	b_HideApp = true;
	b_TrayWasVisible = TrayHide();
	b_WindowWasVisible = IsWindowVisible();
	ShowWindow(SW_HIDE);
}

void CemuleDlg::ToggleShow()
{
	b_HideApp = false;
	if(b_TrayWasVisible)
		// ==> TBH: minimule - Stulle
		/*
		TrayShow();
		*/
		TrayShow(m_bTbhMiniMuleVis);
		// <== TBH: minimule - Stulle
	if(b_WindowWasVisible)
	// ==> Static Tray Icon [MorphXT] - MyTh88
	/*
		ShowWindow(SW_SHOW);
	*/
	{
		if(m_bMaximized)
			ShowWindow(SW_SHOWMAXIMIZED);
		else
		ShowWindow(SW_SHOW);
	}
	// <== Static Tray Icon [MorphXT] - MyTh88
}

BOOL CemuleDlg::RegisterInvisibleHotKey()
{
	if(m_hWnd && IsRunning()){
		bool res = RegisterHotKey( this->m_hWnd, HOTKEY_INVISIBLEMODE_ID ,
						   thePrefs.GetInvisibleModeHKKeyModifier(),
						   thePrefs.GetInvisibleModeHKKey())!=0;
		return res;
	} else
		return false;
}

BOOL CemuleDlg::UnRegisterInvisibleHotKey()
{
	if(m_hWnd){
		bool res = !(UnregisterHotKey(this->m_hWnd, HOTKEY_INVISIBLEMODE_ID));

		// Allows "invisible mode" on multiple instances of eMule
		// Only one app (eMule) can register the hotkey, if we unregister, we need
		// to register the hotkey in other emule.
		EnumWindows(AskEmulesForInvisibleMode, INVMODE_REGISTERHOTKEY);
		return res;
	} else
		return false;
}

// Allows "invisible mode" on multiple instances of eMule
// LOWORD(WPARAM) -> HotKey KeyModifier
// HIWORD(WPARAM) -> HotKey VirtualKey
// LPARAM		  -> int:	INVMODE_RESTOREWINDOW	-> Restores the window
//							INVMODE_REGISTERHOTKEY	-> Registers the hotkey
LRESULT CemuleDlg::OnRestoreWindowInvisibleMode(WPARAM wParam, LPARAM lParam)
{
	if (thePrefs.GetInvisibleMode() &&
		(UINT)LOWORD(wParam) == thePrefs.GetInvisibleModeHKKeyModifier() &&
		(char)HIWORD(wParam) == thePrefs.GetInvisibleModeHKKey()) {
			switch(lParam){
				case INVMODE_RESTOREWINDOW:
					ToggleShow();
					break;
				case INVMODE_REGISTERHOTKEY:
					RegisterInvisibleHotKey();
					break;
				case INVMODE_HIDEWINDOW:
					ToggleHide();
			}
			return UWM_RESTORE_WINDOW_IM;
	} else
		return false;
} 

// Allows "invisible mode" on multiple instances of eMule
BOOL CALLBACK CemuleDlg::AskEmulesForInvisibleMode(HWND hWnd, LPARAM lParam){
	DWORD dwMsgResult;
	WPARAM msgwParam;

	msgwParam=MAKEWPARAM(thePrefs.GetInvisibleModeHKKeyModifier(),
				thePrefs.GetInvisibleModeHKKey());

	LRESULT res = ::SendMessageTimeout(hWnd,UWM_RESTORE_WINDOW_IM, msgwParam, lParam,
				SMTO_BLOCK |SMTO_ABORTIFHUNG,10000,&dwMsgResult);
	
	return res; 
} 
// <== Invisible Mode [TPT/MoNKi] - Stulle

// ==> Design Settings [eWombat/Stulle] - Max
HBRUSH CemuleDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	m_hbrWndClr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetBkMode(TRANSPARENT);

	return m_hbrWndClr;
}
// <== Design Settings [eWombat/Stulle] - Max

// ==> Advanced Updates [MorphXT/Stulle] - Stulle
void CemuleDlg::DoDLPVersioncheck()
{
	if (WSAAsyncGetHostByName(m_hWnd, UM_DLPAUTOVERCHECK_RESPONSE, "dlp.dyndns.info", m_acDLPAutoBuffer, sizeof(m_acDLPAutoBuffer)) == 0){
		AddLogLine(true,GetResString(IDS_AUTODLPFAILED));
	}
}

LRESULT CemuleDlg::OnDLPAutoVerCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	//Xman Info:
	//IP-samples:
	//5.0.0.99 --> DLP version 5
	//105.0.0.99 --> DLP version 105
	//0.1.0.99 --> DLP version 256 not allowed!
	//1.1.0.99 --> DLP version 257


	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acDLPAutoBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;
				dwResult &= 0x00FFFFFF;
//				uint8 uTest1 = (uint8)(dwResult >> 0);
				if (theApp.dlp->IsDLPavailable())
				{
					if(dwResult > theApp.dlp->GetDLPVersion())
						DownloadDLP();
				}
				else
					DownloadDLP();
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_AUTODLPFAILED));
	return 0;

}

void CemuleDlg::DownloadDLP()
{
	CString sbuffer;
	CString strURL = thePrefs.GetAntiLeechURL();
	TCHAR szTempFilePath[_MAX_PATH];
	_tmakepath(szTempFilePath, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), _T("antiLeech.dll"), _T("new"));

	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_ANTILEECH_DWNFILE);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = szTempFilePath;

	if (dlgDownload.DoModal() != IDOK)
	{
		LogError(LOG_STATUSBAR, GetResString(IDS_LOG_ERRDWN), strURL);
		return;
	}
	theApp.dlp->Reload();
}

void CemuleDlg::DoIPFilterVersioncheck()
{
	if (WSAAsyncGetHostByName(m_hWnd, UM_IPFFILTERAUTOVERCHECK_RESPONSE, "ipfilter.dyndns.info", m_acIPFilterAutoBuffer, sizeof(m_acIPFilterAutoBuffer)) == 0){
		AddLogLine(true,GetResString(IDS_AUTOIPFILTERFAILED));
	}
}

LRESULT CemuleDlg::OnIPFilterAutoVerCheckResponse(WPARAM /*wParam*/, LPARAM lParam)
{
	//Info:
	//IP-samples:
	//5.0.0.99 --> IPFilter version 5
	//105.0.0.99 --> IPFilter version 105
	//0.1.0.99 --> IPFilter version 256 not allowed!
	//1.1.0.99 --> IPFilter version 257


	if (WSAGETASYNCERROR(lParam) == 0)
	{
		int iBufLen = WSAGETASYNCBUFLEN(lParam);
		if (iBufLen >= sizeof(HOSTENT))
		{
			LPHOSTENT pHost = (LPHOSTENT)m_acIPFilterAutoBuffer;
			if (pHost->h_length == 4 && pHost->h_addr_list && pHost->h_addr_list[0])
			{
				uint32 dwResult = ((LPIN_ADDR)(pHost->h_addr_list[0]))->s_addr;
				dwResult &= 0x00FFFFFF;
				if (PathFileExists(theApp.ipfilter->GetDefaultFilePath()))
				{
					if(dwResult > thePrefs.GetIPFilterVersionNum())
						theApp.ipfilter->UpdateIPFilterURL(dwResult);
				}
				else
					theApp.ipfilter->UpdateIPFilterURL(dwResult);
				return 0;
			}
		}
	}
	LogWarning(LOG_STATUSBAR,GetResString(IDS_AUTOIPFILTERFAILED));
	return 0;

}

void CemuleDlg::CheckIPFilter()
{
	if(thePrefs.IsIPFilterViaDynDNS())
		DoIPFilterVersioncheck();
	else
		theApp.ipfilter->UpdateIPFilterURL();
}
// <== Advanced Updates [MorphXT/Stulle] - Stulle

// ==> High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
void CemuleDlg::Update_TrafficGraph()
{
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	// Retrieve the current datarates
	uint32 downratekb;	uint32 eMuleInOverall;
	uint32 upratekb; uint32 eMuleOutOverall;
	uint32 notUsed;
	theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
		downratekb, eMuleInOverall,
		upratekb, eMuleOutOverall,
		notUsed, notUsed);
	//Xman end

	m_co_UpTrafficGraph.Set_TrafficValue(upratekb);
	m_co_DownTrafficGraph.Set_TrafficValue(downratekb);
}

void CemuleDlg::Reposition_TrafficGraph()
{
	if (m_co_UpTrafficGraph.m_hWnd &&
		m_co_DownTrafficGraph.m_hWnd &&
		thePrefs.GetShowSpeedMeter())
	{
		CRect		rect;
		CRect rect1,rect2;
		toolbar->GetClientRect(&rect);

		// set updateintervall of graphic rate display (in seconds)
		//rect1.top = rect.top+2;
		rect1.top = 2;
		rect1.right = rect.right-2;
		//rect1.bottom = rect.top+(rect.Height()/2)-1;
		rect1.bottom = 26;
		rect1.left = rect.right-150;

		rect2.top = rect.top+(rect.Height()/2)+1;
		rect2.top = 28;
		rect2.right = rect.right-2;
		//rect2.bottom = rect.bottom-2;
		rect2.bottom = 52;
		rect2.left = rect.right-150;

		CSize	csMaxSize;

		toolbar->GetMaxSize(&csMaxSize);
		if (rect.left + csMaxSize.cx > rect.right - 150)
		{	
			m_co_UpTrafficGraph.ShowWindow(SW_HIDE);
			m_co_DownTrafficGraph.ShowWindow(SW_HIDE);
			return;
		}

		m_co_UpTrafficGraph.SetWindowPos( NULL, rect1.left, rect1.top,
			rect1.Width(), rect1.Height(),
			SWP_NOZORDER | SWP_SHOWWINDOW );
		m_co_DownTrafficGraph.SetWindowPos( NULL, rect2.left, rect2.top,
			rect2.Width(), rect2.Height(),
			SWP_NOZORDER | SWP_SHOWWINDOW );
	}
}
// <== High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88

// ==> File Settings [sivka/Stulle] - Stulle
LRESULT CemuleDlg::OnSaveDone(WPARAM /*wParam*/,LPARAM /*lParam*/)
{
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	theApp.downloadqueue->SaveFileSettings(false);

	return 0;
}
// <== File Settings [sivka/Stulle] - Stulle

// ==> Threaded Known Files Saving [Stulle] - Stulle
LRESULT CemuleDlg::OnSaveKnownDone(WPARAM /*wParam*/,LPARAM /*lParam*/)
{
	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN)
		return FALSE;

	theApp.knownfiles->SaveKnown(false);

	return 0;
}
// <== Threaded Known Files Saving [Stulle] - Stulle

// ==> Diabolic Easteregg [Stulle] - Mephisto
void CemuleDlg::CloseeMule()
{
	theApp.m_app_state = APP_STATE_SHUTTINGDOWN;
	OnClose();
}
// <== Diabolic Easteregg [Stulle] - Mephisto