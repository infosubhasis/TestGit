
#include "MainApp.h"
#include "MainWin.h"
#include<stdio.h>
#include <iostream>
#include <afxsock.h>


BOOL MainApp::InitInstance()
{
	this->m_pMainWnd = new MainWin();
	this->m_pMainWnd->ShowWindow(SW_NORMAL);


	CXLC4SerialRS232 serialIOClass;

	/*if (ERR_SUCCESS == serialIOClass.Open("COM1"))
	{
	CString FWversion;
	serialIOClass.GetFramewareVersion(FWversion);
	serialIOClass.SendCmd(_T("LC 1"));
	Sleep(300);
	serialIOClass.SendCmd(_T("LC 0"));
	}
	
	serialIOClass.Close();*/


	//if (!::AfxSocketInit())
	//{
	//	//FILE_LOG(logDEBUG) << "SOCKETS INIT FAILED" << std::endl;
	//	//return ERR_SUCCESS;
	//}


	ethernetInterface = new CXlC4Ethernet;



	ethernetInterface->SetCallback([this](int Errortype, int response) {return this->OnReceive(Errortype, response); });

	//ethernetInterface->SendCmd(_T("VR"));

	ethernetInterface->Open(_T("192.168.87.234"));

	//ethernetInterface->SendCmd(_T("LC 1"));

	Sleep(300);


	return TRUE;
}

BOOL MainApp::ExitInstance()
{
	if (ethernetInterface)
		delete ethernetInterface;

	return TRUE;

}

void MainApp::OnReceive(int Errortype,int response)
{
	CString s;
	CString err;

	switch (Errortype)
	{
	case MSG_DEV_CONNECT:
		err = _T("Device Connection");
		break;
	case MSG_DEV_AUTHENTICATION:
		err = _T("Device Authentication");
		if(response == ERR_SUCCESS)
			ethernetInterface->SendCmd(_T("LC 1"));
		Sleep(4000);
		    ethernetInterface->SendCmd(_T("LC 0"));
		break;
	case MSG_DEV_RECIEVE:
		err = _T("Device Recieve");
		break;
	}

	s.Format(_T("Error type : %s Error Code : %i"),err, response);
	//AfxMessageBox(s);
	//if()

}


MainApp theApp;

