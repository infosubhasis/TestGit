//////////////////////////////////////////////////////////////////////////////////////////////
// XLC4Serio.cpp:	Serial interface file
/// Description:	This file will be providing a class for interfacing the XLC4 via serial interface <BR>
///
/// Author:			Sczech / CHROMASENS GmbH Konstanz <BR>
///
/// Date:			June, 03, 2014
//	Version:		1.0
//////////////////////////////////////////////////////////////////////////////////////////////


#include "Xlcio.h"
#include "XLC4SerialRS232.h"

#include <strstream>

////////////////////////////////////////////////////////////////////////////////
// Simple input/Output via COMx: 
// ----------------------------------------------------------------------------
// Sending commands to the XLC4
// Getting a response from the controller
///////////////////////////////////////////////////////////////////////////////

// Constructor of the class
CXLC4SerialRS232::CXLC4SerialRS232(void){
	strName = _T("Rs232");
	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
 	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );
	m_hXLC4ComDev = NULL;
	m_bOpened = FALSE;
}

CXLC4SerialRS232::~CXLC4SerialRS232()
{
	Close();
}

BOOL CXLC4SerialRS232::IsOpened( void ){
	return( m_bOpened );
}
///////////////////////////////////////////////////////////////////////////////////////////
// Open RS232 interface,
// Set Timeout-Values for ReadFile and WriteFile.
// Timeout is set for an immediate return after ReadFile/WriteFile .
// Wait for data by the eventsm_OverlappedRead.hEvent and m_OverlappedWrite.hEvent 
//////////////////////////////////////////////////////////////////////////////////////////////
DWORD CXLC4SerialRS232::Open( CString strComPort, int nBaud )
{
    DCB dcb;

    m_errNum  = ERR_SUCCESS;  
	if( m_bOpened )
	   return m_errNum;  // Device already open
	// Use this format to be sure that interfaces >= COM9 can be opened 
	m_strDevice.Format(_T("\\\\.\\%s"), strComPort);
    CW2T szPort(m_strDevice);
	m_hXLC4ComDev = 
	  CreateFile( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL 
								       | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL );
	if( m_hXLC4ComDev == NULL || m_hXLC4ComDev == INVALID_HANDLE_VALUE) {
       m_dwError  = GetLastError();
	   CString s  = ErrorCodeToText(m_dwError);
	   m_errNum   = ERR_FAILEDONCREATEFILE;
	   m_errMsg.Format(_T("++ Error %d: Couldn't open port %s\n%s"), m_errNum,m_strDevice,s);
	   return m_errNum;
	}

	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
 	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );

	COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 4000;

	BOOL const e_timeout = SetCommTimeouts( m_hXLC4ComDev, &CommTimeOuts );
	if( !e_timeout ) {
       m_dwError  = GetLastError();
	   CString s  = ErrorCodeToText(m_dwError);
	   m_errNum   = ERR_FAILEDONSETCOMMTIMEOUT;
	   m_errMsg.Format(_T("++ Error %d: Couldn't set timeout values for %s\n%s"), m_errNum,m_strDevice,s);
	   return m_errNum;
	}

	m_OverlappedRead.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_OverlappedWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	dcb.DCBlength = sizeof( DCB );
	BOOL const e_state = GetCommState( m_hXLC4ComDev, &dcb );
	if( !e_state ) {
       m_dwError  = GetLastError();
	   CString s  = ErrorCodeToText(m_dwError);
	   m_errNum   = ERR_FAILEDONGETCOMMSTATE;
	   m_errMsg.Format(_T("++ Error %d: Couldn't set timeout values for %s\n%s"), m_errNum,m_strDevice,s);
	   return m_errNum;
	}
	dcb.BaudRate = nBaud; //m_nBaudRate
	dcb.ByteSize = 8;
	if( !SetCommState( m_hXLC4ComDev, &dcb )      ||
		!SetupComm( m_hXLC4ComDev, 10000, 10000 ) ||
		m_OverlappedRead.hEvent == NULL           ||
		m_OverlappedWrite.hEvent == NULL ) {
           m_dwError  = GetLastError();
		   if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
		   if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
		   CloseHandle( m_hXLC4ComDev );
	       CString s  = ErrorCodeToText(m_dwError);
	       m_errNum   = ERR_FAILEDONSETUPCOMM;
	       m_errMsg.Format(_T("++ Error %d: Couldn't set baud rate or number of bits on %s\n%s"), m_errNum,m_strDevice,s);
	       return m_errNum;
	}
//   empty Receive-Buffer 
	int m_recvRdIindex = 0;
	int m_recvWrIindex = 0;
	int m_recvFill     = 0;
//
	m_bOpened = TRUE;
	return m_errNum;
}

DWORD CXLC4SerialRS232::Close( void )
{

    m_errNum = ERR_SUCCESS;  
	m_dwError = 0;  
	if( !m_bOpened || m_hXLC4ComDev == NULL )
		return m_dwError;
//
	if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
	if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
	if(!CloseHandle( m_hXLC4ComDev )) {
       m_dwError  = GetLastError();
	   CString s  = ErrorCodeToText(m_dwError);
	   m_errNum   = ERR_FAILEDONCLOSEHANDLE;
	   m_errMsg.Format(_T("++ Error %d: Couldn't close port %s properly\n%s"), m_errNum,m_strDevice,s);
	   return m_errNum;
	}
	m_bOpened = FALSE;
	m_hXLC4ComDev = NULL;
	return m_errNum;
}

//====================================================================================
// Get  the Frameware  and hardware Version Info
//
//=====================================================================================

int CXLC4SerialRS232::GetFramewareVersion(CString &versionInfo)
{
	CString sendString(_T(""));
	CString rcvString(_T(""));

	int returnval = ERR_SUCCESS;
	// The delimiters for the start and the end of the command (STX and ETX are added by the serial interface class!)
	// Request the current Firmware version and release from the controller
	sendString.Format(_T("VR"));
	// Need to enclose the send and receive command into try/catch-blocks. Otherwise the program will crash in case of an error
	// The interface class throws exceptions!
	try {
		returnval = SendCmd(sendString);
	}
	catch (CXLC4Rs232IoError rs232Err) {
		printf("Error sending command 'VR': %S", rs232Err.ioErrMsg);
		returnval = rs232Err.xclError;
	}

	try {
		GetResponse(rcvString);
	}
	catch (CXLC4Rs232IoError rs232Err) {
		printf("Error getting response 'VR': %S", rs232Err.ioErrMsg);
		returnval = rs232Err.xclError;
	}
	printf("Received response: %S\n", rcvString);
	versionInfo = rcvString;
	return returnval;

}

//////////////////////////////////////////////////////////////////////////////////
// Overwrite function of Base Class: SendCmd                                    //
// -------------------------------------------------------                      //
// Please make sure to use a catch/try mechanism when using this functio to     //
// treat the errors correctly                                                   //
// Adding of the STX and ETX is done in this function                           //
// 
//////////////////////////////////////////////////////////////////////////////////
int CXLC4SerialRS232::SendCmd( CString strCommand )
{
	if( !m_bOpened || m_hXLC4ComDev == NULL ) { 
	   m_dwError = 0;
	   m_errNum   = ERR_DEVICENOTOPEN ;
	   m_errMsg.Format(_T("++ Error %d: Cannot do a SendCmd, Device %s not opened"),m_errNum,m_strDevice);
	   CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
       throw rs232Err;  
    }
	BOOL bWriteStat, bOverlappStat;
	DWORD dwBytesWritten;
	int size;
	char writeBuf[WRITE_BUFFER_SIZE];

	PurgeComm(m_hXLC4ComDev, PURGE_TXCLEAR | PURGE_RXCLEAR);

	size = strCommand.GetLength();
    CW2A byteBuffer(strCommand);
    memset(writeBuf,0,WRITE_BUFFER_SIZE);
    writeBuf[0] = (char)STX;
	strncpy_s(&writeBuf[1],WRITE_BUFFER_SIZE-1, byteBuffer,size);
    writeBuf[size+1] = (char)ETX;
	writeBuf[size+2] = '\0';

	bWriteStat = WriteFile( m_hXLC4ComDev, writeBuf, size+2, &dwBytesWritten, &m_OverlappedWrite );
	if( !bWriteStat && ( (m_dwError=GetLastError()) == ERROR_IO_PENDING ) ){
		if( WaitForSingleObject( m_OverlappedWrite.hEvent, 1000 ) ) {
			dwBytesWritten = 0;                       // Write-Timeout
            m_dwError  = GetLastError();              // throw exception
	        CString s  = ErrorCodeToText(m_dwError);
	        m_errNum   = ERR_WRITEFILETIMEDOUT;
	        m_errMsg.Format(_T("++ Error %d: WriteFile timed out\n%s"), m_errNum,m_strDevice,s);
	        CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
            throw rs232Err;  
		}
		else
		{ // The pending operation has been completed, the system sets the event object to the signaled state.
			bOverlappStat=GetOverlappedResult( m_hXLC4ComDev, &m_OverlappedWrite, &dwBytesWritten, FALSE );
			if(!bOverlappStat || dwBytesWritten == 0) {
               m_dwError  = GetLastError();              // throw exception
	           CString s  = ErrorCodeToText(m_dwError);
	           m_errNum   = ERR_FAILEDONWRITFILE;
	           m_errMsg.Format(_T("++ Error %d: Failed on WriteFile to %s, num bytes written %d\n%s"),
				                            m_errNum,m_strDevice,dwBytesWritten,s);
	           CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
               throw rs232Err;  
			}
		}
	}
	return (int)ERR_SUCCESS;
}


#define READ_STATE_BEGIN   0
#define READ_STATE_STORE   1
#define READ_STATE_STX     2
#define READ_STATE_ETX     3
#define READ_STATE_END     4
#define READ_STATE_TIMEOUT     100
#define READ_STATE_SYNTAXERROR 101

int CXLC4SerialRS232::GetResponse(CString &response)
{
    char szResp[RESPONSE_LEN_MAX];  // Responsestring as Byte-String
    int retryCounter;
	int state;
    int i,j,len;

	Sleep(100);

	response.Empty();
    retryCounter = 50;
    i = 0;
    j = 0;
	memset(m_recvBuffer, 0, RECEIVE_BUFFER_SIZE);
    state = READ_STATE_BEGIN;

	while(state != READ_STATE_END) {
      switch( state )
	  {
	   case READ_STATE_BEGIN:
         len = ReadInputData( m_recvBuffer, RESPONSE_LEN_MAX);
         if( len > 0 ) state=READ_STATE_STX;
	     else {
           retryCounter--;
		   Sleep(100);
		   if(retryCounter <= 0) {
			   state = READ_STATE_TIMEOUT;
	           m_errNum   = ERR_NORESPONSEFROMXLC;
		   }
		 }
		 break;

	   case READ_STATE_STX:
		   if(m_recvBuffer[j] == STX) {
			   state = READ_STATE_STORE;
			   j++;  // Do not readSTX 
		   }
		   else {
		     //Try to find the start
             // found state = 2
		     // No start-Flag found
		     state= READ_STATE_SYNTAXERROR;
	       }
		   break;

	  case READ_STATE_STORE:
		 while(m_recvBuffer[j] != ETX && j < len && i < RESPONSE_LEN_MAX-1) {
           szResp[i++] = m_recvBuffer[j++];
	     }
		 if(m_recvBuffer[j] == ETX) {
           szResp[i] = '\0';
		   state = READ_STATE_ETX; 	// Command transmitted completely...
		 } else if( j == len ) {    // .. no ETX received yet
		   j = 0;                   // Try to read the rest..
           len = ReadInputData(m_recvBuffer,RESPONSE_LEN_MAX);
           if( len == 0 ) {
             retryCounter--;
		     Sleep(100);
		     if(retryCounter <= 0) {
			   state = READ_STATE_TIMEOUT; // finally TIMEOUT
	           m_errNum   = ERR_NORESPONSEFROMXLC;
		    }
		   }
	     }
		 else if(i >= RESPONSE_LEN_MAX-1) {
	       state = READ_STATE_SYNTAXERROR;  // Maximal response length reached
	     }
	     break;

	  case READ_STATE_ETX:
		  {                     // Block, sonst Error C2361
		    CA2W res(szResp);   // Return Response as CString
		    response.Append(res);
            state = READ_STATE_END;
	        break;
		  }

	  default:
         szResp[i] = '\0';
 	     CA2W res(szResp);   // Return Response as CString
		 response.Append(res);
	     m_errNum   = ERR_INVALIDRESPONSE;
		 m_dwError = 0;
	     m_errMsg.Format(_T("++ Error %d: Incomplete or no response from XLC4 on %s\nresponse=\"%s\""),m_errNum,m_strDevice,response);
	     CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
         throw rs232Err;  
	  }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// ReadInputData
// ----------------
// Read the responses from the XLC controller
// Return values > 0 -> Anzahl gelesener Bytes
// If an error occurs, an exception will be thrown
/////////////////////////////////////////////////////////////////////////////////
int CXLC4SerialRS232::ReadInputData( const char *buffer, int limit )
{
	BOOL bReadStatus, bOverlappStat;
	DWORD dwBytesRead, dwBytesToRead, dwErrorFlags;
	COMSTAT ComStat;

	if( !m_bOpened || m_hXLC4ComDev == NULL )  {
	   m_dwError = 0;
	   m_errNum   = ERR_DEVICENOTOPEN ;
	   m_errMsg.Format(_T("++ Error %d: Cannot do a ReadFile, Divice %s not opened"),m_errNum,m_strDevice);
	   CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
       throw rs232Err;  
	}

	dwBytesRead = 0;
	ClearCommError( m_hXLC4ComDev, &dwErrorFlags, &ComStat );
	if( !ComStat.cbInQue ) // No Bytes in the cbInQueue
	   return 0;           // Timeout must be handled a level higher

	dwBytesToRead = (DWORD) ComStat.cbInQue;
	if( limit < (int) dwBytesToRead ) dwBytesToRead = (DWORD) limit;

	bReadStatus = ReadFile( m_hXLC4ComDev, (LPVOID)buffer, dwBytesToRead, &dwBytesRead, &m_OverlappedRead );
	if( !bReadStatus ){
		if( (m_dwError=GetLastError()) == ERROR_IO_PENDING ){
			if( WaitForSingleObject( m_OverlappedRead.hEvent, 2000 )) {
               m_dwError  = GetLastError();              // throw exception
	           CString s  = ErrorCodeToText(m_dwError);
	           m_errNum   = ERR_READFILETIMEDOUT;
	           m_errMsg.Format(_T("++ Error %d: ReadFile timed out on %s, num bytes read %d\n%s"),
				                            m_errNum,m_strDevice,dwBytesRead,s);
	           CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
               throw rs232Err;  
			}
			else {
				bOverlappStat = GetOverlappedResult(m_hXLC4ComDev, &m_OverlappedRead, &dwBytesRead, FALSE) ; 
     			if(!bOverlappStat || dwBytesRead == 0) {
                   m_dwError  = GetLastError();              // throw exception
	               CString s  = ErrorCodeToText(m_dwError);
	               m_errNum   = ERR_FAILEDONREADFILE1;
	               m_errMsg.Format(_T("++ Error %d: Failed on ReadFile to %s, num bytes written %d\n%s"),
				                          m_errNum,m_strDevice,dwBytesRead,s);
	              CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
                  throw rs232Err;  
			    }
				// ReadFile successfully finished
			    return (int)dwBytesRead;
		    }
		}
		else 
		{   // some other error tan ERROR_IO_PENDING, throw exception
	       CString s  = ErrorCodeToText(m_dwError);
	       m_errNum   = ERR_FAILEDONREADFILE2;
	       m_errMsg.Format(_T("++ Error %d: Failed on ReadFile to %s, num bytes written %d\n%s"),
		                          m_errNum,m_strDevice,dwBytesRead,s);
	       CXLC4Rs232IoError rs232Err(m_errNum, m_dwError, m_errMsg);
           throw rs232Err;  
		}
	}
	if(dwBytesRead==0) {
		m_dwError=GetLastError();
	}

	return (int)dwBytesRead;
}

////////////////////////////////////////////////////////////////////////////////
// Get the text related to a Communication-Errorcode 
////////////////////////////////////////////////////////////////////////////////
CString CXLC4SerialRS232::ErrorCodeToText(DWORD err)
{
   CString ErrorText;
   LPTSTR s;
   if(::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            err,
            0,
            (LPTSTR)&s,
            0,
            NULL) == 0)
   { // Too bad! FormatMessage failed also... 
      CString t;
	  DWORD secondErr = GetLastError();
	  t.Format(_T("No translation found for error code %d, Formatmessage: %d"), LOWORD(err),secondErr);
      ErrorText = t;
      ::LocalFree(s); 
   }
    else
   { // mapping for error code found
      LPTSTR p = _tcschr(s, _T('\r'));
	  if(p != NULL) {*p = _T('\0');}  // get rid of (CRLF}
      ErrorText = s;
      ::LocalFree(s);
   }
    return ErrorText;
}

////////////////////////////////////////////////////////////////////
// Constructor of the error class 
////////////////////////////////////////////////////////////////////
CXLC4Rs232IoError::CXLC4Rs232IoError(rs232ErrNum_t xlcErr, DWORD dwCommErr, CString msg)
{
    xclError = xlcErr;      // Error number of programm situation ERR_...
	dwCommErr = dwCommErr;  // Windows Socket API errors
	ioErrMsg = msg;
}
