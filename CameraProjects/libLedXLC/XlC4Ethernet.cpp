
#include "XlC4Ethernet.h"
#include "ClientSocket.h"


CXlC4Ethernet::CXlC4Ethernet()
{

	m_pLogger = NULL;
	errno_t err;
	if (m_pLogger == NULL)
		err = fopen_s(&m_pLogger, "logfile_LEDCorona.log", "w");

	Output2FILE::Stream() = m_pLogger;

	fSocketInitSuccessful = false;
	fSocketConnected = false;
	fDeviceAuthenticated = false;

}

CXlC4Ethernet::~CXlC4Ethernet(void)
{
	if (m_telnetSocket)
		delete m_telnetSocket;

}
int CXlC4Ethernet::Open(CString cHostName)
{
	CStringA test(cHostName);
	FILE_LOG(logDEBUG) << "connecting to Socket IP : " << test.GetBuffer() << std::endl;
	if (!fSocketInitSuccessful)
	{
		if (!AfxSocketInit())
		{
			AfxMessageBox(_T("SOCKETS INIT FAILED"));
			return ERR_SOCKETINITFAILED;
		}
		fSocketInitSuccessful = true;
		FILE_LOG(logDEBUG) << "SOCKET INIT DONE" << std::endl;
	}

	if (fSocketInitSuccessful)
	{
		BOOL bOK;
		m_telnetSocket = new CClientSocket(this);
		if (m_telnetSocket != NULL)
		{			
			bOK = m_telnetSocket->Create();
			if (bOK == TRUE)
			{
				m_telnetSocket->AsyncSelect(FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT | FD_OOB);
				BOOL ret = m_telnetSocket->Connect(cHostName, 23);
				Sleep(90);
			}
			else
			{
				ASSERT(FALSE);  //Did you remember to call AfxSocketInit()?
				delete m_telnetSocket;
				m_telnetSocket = NULL;
				return ERR_FAILEDONCONNECT;
			}
		}
	}

	return ERR_SUCCESS;

}

DWORD CXlC4Ethernet::Close(void)
{
	if (fSocketConnected)
		m_telnetSocket->Close();
	if (m_pLogger)
		fclose(m_pLogger);
	return ERR_SUCCESS;
}
int CXlC4Ethernet::SendCmd(CString strCommand)
{

	if (!fDeviceAuthenticated)
		return ERR_DEVICE_AUTHENTICATION;
	
	char writeBuf[WRITE_BUFFER_SIZE];
	int size = strCommand.GetLength();
	CW2A byteBuffer(strCommand);
	memset(writeBuf, 0, WRITE_BUFFER_SIZE);
	writeBuf[0] = (char)STX;
	strncpy_s(&writeBuf[1], WRITE_BUFFER_SIZE - 1, byteBuffer, size);
	writeBuf[size + 1] = (char)ETX;
	writeBuf[size + 2] = '\r';
	writeBuf[size + 3] = '\n';

	m_telnetSocket->Send(writeBuf, size + 4);

	return ERR_SUCCESS;
}

int CXlC4Ethernet::SendData(CString strData)
{
	char writeBuf[WRITE_BUFFER_SIZE];
	int size = strData.GetLength();
	CW2A byteBuffer(strData);
	memset(writeBuf, 0, WRITE_BUFFER_SIZE);
	strncpy_s(&writeBuf[0], WRITE_BUFFER_SIZE - 1, byteBuffer, size);	
	writeBuf[size] = '\r';
	writeBuf[size + 1] = '\n';

	m_telnetSocket->Send(writeBuf, size+2);

	return ERR_SUCCESS;
}


void CXlC4Ethernet::LogResponseCode(int nErrorCode)
{
	FILE_LOG(logDEBUG) << "ERROR CODE : " << nErrorCode << std::endl;
	
}

void CXlC4Ethernet::OnConnected(int nErrorCode)
{	
	m_fnCallBack(MSG_DEV_CONNECT, nErrorCode);
	if (nErrorCode == ERR_SUCCESS)
	{
		fSocketConnected = true;
		FILE_LOG(logDEBUG) << "SOCKET CONNECTED" << std::endl;
	}
}
void CXlC4Ethernet::JobOnReceive(int nErrorCode)
{	
	unsigned char m_bBuf[1024];	
	memset(m_bBuf, 0, 1024);
	
	if (nErrorCode != 0)
	{
		m_fnCallBack(MSG_DEV_RECIEVE, ERR_SOCKET_RECIEVE);
	}

	int nBytes = m_telnetSocket->Receive(m_bBuf, 1024);
	if (nBytes != SOCKET_ERROR)
	{
		FILE_LOG(logDEBUG) << "MSG RCVD: " << m_bBuf << std::endl;
		int ndx = 0;
		while (GetLine(m_bBuf, nBytes, ndx) != TRUE);
		ProcessOptions();
		if (m_strNormalText.Find("Login:") != -1)
		{
			SendData(_T("admin"));
		}
		else if (m_strNormalText.Find("Password:") != -1)
		{
			SendData(_T("chromasens"));
		}
		else if (m_strNormalText.Find("Logged in successfully") != -1)
		{
			fDeviceAuthenticated = true;
			m_fnCallBack(MSG_DEV_AUTHENTICATION, ERR_SUCCESS);			
			//SendCmd(_T("VR")); //Version Info
		}
		else if ((m_strNormalText.Find("Access denied") != -1))
		{
			m_fnCallBack(MSG_DEV_AUTHENTICATION, ERR_DEVICE_AUTHENTICATION);
			fDeviceAuthenticated = false;
		}	
		else if ((m_strNormalText.Find("er 100") != -1))
		{
			m_fnCallBack(MSG_DEV_RECIEVE, ERR_UNKNOWN_COMMAND);
		}
		else
		{
			m_fnCallBack(MSG_DEV_RECIEVE, nErrorCode);
		}
	}
	m_strLine.Empty();
	m_strResp.Empty();


}
void CXlC4Ethernet::JobOnClose()
{

}

void CXlC4Ethernet::ProcessOptions()
{
	CStringA m_strTemp;
	CString m_strOption;
	unsigned char ch;
	int ndx;
	int ldx;
	BOOL bScanDone = FALSE;

	m_strTemp = m_strLine;

	while (!m_strTemp.IsEmpty() && bScanDone != TRUE)
	{
		ndx = m_strTemp.Find(IAC);
		if (ndx != -1)
		{
			m_strNormalText += m_strTemp.Left(ndx);
			ch = m_strTemp.GetAt(ndx + 1);
			switch (ch)
			{
			case DO:
			case DONT:
			case WILL:
			case WONT:
				m_strOption = m_strTemp.Mid(ndx, 3);
				m_strTemp = m_strTemp.Mid(ndx + 3);
				m_strNormalText = m_strTemp.Left(ndx);
				m_ListOptions.AddTail(m_strOption);
				break;
			case IAC:
				m_strNormalText = m_strTemp.Left(ndx);
				m_strTemp = m_strTemp.Mid(ndx + 1);
				break;
			case SB:
				m_strNormalText = m_strTemp.Left(ndx);
				ldx = m_strTemp.Find(SE);
				m_strOption = m_strTemp.Mid(ndx, ldx);
				m_ListOptions.AddTail(m_strOption);
				m_strTemp = m_strTemp.Mid(ldx);
				AfxMessageBox(m_strOption, MB_OK);
				break;
			}
		}

		else
		{
			m_strNormalText = m_strTemp;
			bScanDone = TRUE;
		}
	}

	RespondToOptions();
}
BOOL CXlC4Ethernet::GetLine(unsigned char * bytes, int nBytes, int& ndx)
{
	BOOL bLine = FALSE;
	while (bLine == FALSE && ndx < nBytes)
	{
		unsigned char ch = bytes[ndx];

		switch (ch)
		{
		case '\r': // ignore
			m_strLine += "\r\n"; //"CR";
			break;
		case '\n': // end-of-line
				   //			m_strLine += '\n'; //"LF";
				   //			bLine = TRUE;
			break;
		default:   // other....
			m_strLine += ch;
			break;
		}

		ndx++;

		if (ndx == nBytes)
		{
			bLine = TRUE;
		}
	}
	return bLine;
}
void CXlC4Ethernet::RespondToOptions()
{
	CString strOption;

	while (!m_ListOptions.IsEmpty())
	{
		strOption = m_ListOptions.RemoveHead();

		ArrangeReply(strOption);
	}

	DispatchMessage(CStringA(m_strResp));
	m_strResp.Empty();
}
void CXlC4Ethernet::ArrangeReply(CString strOption)
{
	unsigned char Verb;
	unsigned char Option;
	unsigned char Modifier;
	unsigned char ch;
	BOOL bDefined = FALSE;

	if (strOption.GetLength() < 3) return;

	Verb = strOption.GetAt(1);
	Option = strOption.GetAt(2);

	switch (Option)
	{
	case 1:	// Echo
	case 3: // Suppress Go-Ahead
		bDefined = TRUE;
		break;
	}

	m_strResp += IAC;

	if (bDefined == TRUE)
	{
		switch (Verb)
		{
		case DO:
			ch = WILL;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case DONT:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WILL:
			ch = DO;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WONT:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case SB:
			Modifier = strOption.GetAt(3);
			if (Modifier == SEND)
			{
				ch = SB;
				m_strResp += ch;
				m_strResp += Option;
				m_strResp += IS;
				m_strResp += IAC;
				m_strResp += SE;
			}
			break;
		}
	}

	else
	{
		switch (Verb)
		{
		case DO:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case DONT:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WILL:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WONT:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		}
	}
}
void CXlC4Ethernet::DispatchMessage(CStringA strText)
{
	try {
		char * test = strText.GetBuffer();
		int length = strText.GetLength();
		//m_telnetSocket->Send(test, length);
		
	}
	catch (...)
	{
		//Failed sending Character
	}
}



