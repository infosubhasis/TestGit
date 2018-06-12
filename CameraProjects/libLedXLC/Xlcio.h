#pragma once
// ////////////////////////////////////////////////////////////////
// Common class for multiple interfaces.
// Sending and getting commands and responses are handled in the two functions:
//   SendCmd(Cstring cmd)
//   GetResponse(CString &response)
// Because of this the abstract base class is defined here. The classes for the different interfaces must be 
// inherited from this class
// 
// ////////////////////////////////////////////////////////////////

#ifdef LIBCORONADLL_EXPORTS
#define LIBCORONADLL_API __declspec(dllexport) 
#else
#define LIBCORONADLL_API __declspec(dllimport) 
#endif

#define STX	0x02
#define ETX 0x03

#define RECEIVE_BUFFER_SIZE   1024
#define RESPONSE_LEN_MAX      512
#define WRITE_BUFFER_SIZE     256


typedef enum {
	ERR_SUCCESS = (0),
	ERR_FAILEDONCREATEFILE = (-1001),
	ERR_FAILEDONSETCOMMTIMEOUT = (-1002),
	ERR_FAILEDONSETUPCOMM = (-1003),
	ERR_FAILEDONGETCOMMSTATE = (-1004),
	ERR_FAILEDONCLOSEHANDLE = (-1005),
	ERR_WRITEFILETIMEDOUT = (-1006),
	ERR_FAILEDONWRITFILE = (-1007),
	ERR_DEVICENOTOPEN = (-1008),
	ERR_READFILETIMEDOUT = (-1009),
	ERR_READFILETIMEDOUT1 = (-1010),
	ERR_FAILEDONREADFILE1 = (-1011),
	ERR_FAILEDONREADFILE2 = (-1012),
	ERR_NORESPONSEFROMXLC = (-1013),
	ERR_INVALIDRESPONSE = (-1014)
}rs232ErrNum_t;


#include "afxwin.h"
#include "afxcmn.h"
#include<functional>

class LIBCORONADLL_API CXlcio {
 private:
 public:
  	CString strName;
//
	CXlcio(void);
	virtual ~CXlcio(void)=0;
//
	virtual DWORD Close(void)=0;
	virtual int SendCmd(CString cmd) = 0;
	virtual int GetResponse(CString &response) = 0;
	virtual BOOL IsOpened( void )=0;
};
