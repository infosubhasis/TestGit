//
// XLC4Serio.h : Headerfile
//
#pragma once

#include "afxwin.h"
#include "afxcmn.h"
#include "Xlcio.h"

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13




class LIBCORONADLL_API CXLC4SerialRS232 : public CXlcio {
public:
	CXLC4SerialRS232(void);
	~CXLC4SerialRS232();

	DWORD Open( CString strComPort, int nBaud = 115200);
	DWORD Close( void );

    int SendCmd( CString strCommand );
	int GetResponse(CString &response);
	BOOL IsOpened( void );
	CString ErrorCodeToText(DWORD err);
                          //   Error information
    CString m_strDevice;
	CString m_errMsg;     //   
	rs232ErrNum_t  m_errNum;
	DWORD m_dwError;

	int GetFramewareVersion(CString &versionInfo);

protected:
	char m_recvBuffer[RECEIVE_BUFFER_SIZE];
	int m_recvRdIindex;
	int m_recvWrIindex;
	int m_recvFill;
	HANDLE m_hXLC4ComDev;
	OVERLAPPED m_OverlappedRead, m_OverlappedWrite;
	BOOL m_bOpened;
//
	int ReadInputData( const char *, int );
	//int ReadDataWaiting( void );
	//BOOL Write1Byte( unsigned char );



};


////////////////////////////////////////////////////////////////////
//
//  Error-class for the RS232-functions
////////////////////////////////////////////////////////////////////
class LIBCORONADLL_API CXLC4Rs232IoError{
public:
	CXLC4Rs232IoError(rs232ErrNum_t xlcErr, DWORD dwCommErr, CString msg);

	rs232ErrNum_t xclError;        // Error number of programm situation ERR_...
	int dwCommErr;       // Error code on communication device
	CString ioErrMsg;
};