// ClientSocket.cpp : implementation file
//

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "ClientSocket.h"
#include "XlC4Ethernet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket(CXlC4Ethernet * pSockethandler)
{
	m_pSockethandler = pSockethandler;
	data = 121;
}

CClientSocket::~CClientSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CClientSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions

void CClientSocket::OnClose(int nErrorCode) 
{
//	AfxMessageBox("Connection Closed",MB_OK);
	
	CAsyncSocket::OnClose(nErrorCode);	
}

void CClientSocket::OnConnect(int nErrorCode) 
{
	data = data + 5;
	m_pSockethandler->LogResponseCode(nErrorCode);
	m_pSockethandler->OnConnected(nErrorCode);
	CAsyncSocket::OnConnect(nErrorCode);	
}

void CClientSocket::OnOutOfBandData(int nErrorCode) 
{
	ASSERT(FALSE); //Telnet should not have OOB data
	CAsyncSocket::OnOutOfBandData(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode) 
{

	/*cView->ProcessMessage(this);*/ //This callback gets caled when there is resposnse recieved from the destination 
	m_pSockethandler->LogResponseCode(nErrorCode);
	m_pSockethandler->JobOnReceive(nErrorCode);
	CAsyncSocket::OnReceive(nErrorCode);
}

void CClientSocket::OnSend(int nErrorCode) 
{
	CAsyncSocket::OnSend(nErrorCode);
}

void CClientSocket::OnAccept(int nErrorCode)
{	
	CAsyncSocket::OnSend(nErrorCode);
}