#pragma once
#pragma once
#include <afxwin.h>
#include "..\libLedXLC\XLC4SerialRS232.h"

#include "..\libLedXLC\XlC4Ethernet.h"

class MainApp : public CWinApp
{
public:
	BOOL InitInstance();
	BOOL ExitInstance();

	void OnReceive(int Errortype, int response);
	CXlC4Ethernet * ethernetInterface;
};

