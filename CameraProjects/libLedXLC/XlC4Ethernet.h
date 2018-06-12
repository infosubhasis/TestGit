#pragma once
// ////////////////////////////////////////////////////////////////
// class for multiple Ethernet Interfaces.
// 
//  
//   
// 
// ////////////////////////////////////////////////////////////////



#include <afxwin.h>
#include <afxcmn.h>
#include <vector>
#include "Xlcio.h"
#include "log.h"


typedef std::function<void(int,int)> CallbackFunction;

typedef enum {
	ERR_UNKNOWN = (-1),	
	ERR_FAILEDONCONNECT = (-1050),
	ERR_CONNECTIONTIMEOUT = (-1051),
	ERR_SOCKETINITFAILED = (-1052),
	ERR_DEVICE_AUTHENTICATION = (-1053),
	ERR_ACCESSDENIED = (-1054),
	ERR_UNKNOWN_COMMAND = (-1055),
	ERR_SOCKET_RECIEVE = (-1056)
	
}ethernetErrNum_t;

const	unsigned char IAC = 255;
const	unsigned char DO = 253;
const	unsigned char DONT = 254;
const	unsigned char WILL = 251;
const	unsigned char WONT = 252;
const	unsigned char SB = 250;
const	unsigned char SE = 240;
const	unsigned char IS = '0';
const	unsigned char SEND = '1';
const	unsigned char INFO = '2';
const	unsigned char VAR = '0';
const	unsigned char VALUE = '1';
const	unsigned char ESC = '2';
const	unsigned char USERVAR = '3';

typedef enum {
	MSG_DEV_CONNECT = (0),
	MSG_DEV_AUTHENTICATION = (1),
	MSG_DEV_RECIEVE = (2),	
}eMsgType;

class CClientSocket;

class LIBCORONADLL_API CXlC4Ethernet {
	
 public:
  	CString strName;
//
	CXlC4Ethernet();
	virtual ~CXlC4Ethernet(void);
//
	
	int Open(CString cHostName);

	virtual DWORD Close(void);
	virtual int SendCmd(CString cmd);

	int SendData(CString strData);
	//virtual int GetResponse(CString &response) = 0;
	virtual BOOL IsOpened(void) { return fSocketConnected; }
	
	void LogResponseCode(int nErrorCode);

	void OnConnected(int nErrorCode);
	void JobOnReceive(int nErrorCode);
	void JobOnClose();

	// Clients can connect their callback with this.
	void SetCallback(CallbackFunction fnCallback) {
		m_fnCallBack = fnCallback;
	}

protected:
	void ProcessOptions();
	BOOL CXlC4Ethernet::GetLine(unsigned char * bytes, int nBytes, int& ndx);
	void RespondToOptions();
	void ArrangeReply(CString strOption);
	void DispatchMessage(CStringA strText);
		
private:
	CClientSocket * m_telnetSocket;
	FILE* m_pLogger;
	std::vector<int> lstErrorcodes;	
	int iLastError;
	bool fSocketInitSuccessful, fSocketConnected, fDeviceAuthenticated;

	CString        m_recieveddata;
	CString        m_status;


	CString m_strResp;
	CString m_strLine;
	CStringList m_ListOptions;
	CStringA m_strNormalText;

	CallbackFunction m_fnCallBack;
};
