#if !defined(AFX_CLIENTSOCKET_H__42C5C9C7_3102_11D2_9A30_00C04FB78B23__INCLUDED_)
#define AFX_CLIENTSOCKET_H__42C5C9C7_3102_11D2_9A30_00C04FB78B23__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ClientSocket.h : header file
//

#include <afxsock.h>
#include <functional>
/////////////////////////////////////////////////////////////////////////////
// CClientSocket command target



class CXlC4Ethernet;

class CClientSocket : public CAsyncSocket
{
// Attributes
public:

// Operations
public:	
	CClientSocket(CXlC4Ethernet * pSockethandler);
	virtual ~CClientSocket();

// Overrides
public:
	CXlC4Ethernet * m_pSockethandler;
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientSocket)
	public:
		virtual void OnReceive(int nErrorCode);        //FD_READ 
		virtual void OnSend(int nErrorCode);        //FD_WRITE 
		virtual void OnOutOfBandData(int nErrorCode);        //FD_OOB 
		virtual void OnAccept(int nErrorCode);        //FD_ACCEPT 
		virtual void OnConnect(int nErrorCode);        //FD_CONNECT 
		virtual void OnClose(int nErrorCode);        //FD_CLOSE 

	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CClientSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG


	


// Implementation
protected:
	int data;
	
};





/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTSOCKET_H__42C5C9C7_3102_11D2_9A30_00C04FB78B23__INCLUDED_)
