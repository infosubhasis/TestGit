
#include<stdio.h>
#include <iostream>
#include "..\libLEDCorona\XLC4SerialRS232.h"
#include "..\libLEDCorona\XlC4Ethernet.h"

int main()
{
	//char a = 2;
	//std::cout << " : " << char(67) <<" "<< char(3)<< std::endl; //Print <STX> CTRL+B and <ETX> CTRL+C
	
	CXlC4Ethernet ethernetInterface;

	ethernetInterface.Open("192.168.87.234");

	/*CXLC4SerialRS232 serialIOClass;	

	if (ERR_SUCCESS == serialIOClass.Open("COM1"))
	{
		CString FWversion;
		serialIOClass.GetFramewareVersion(FWversion);
		serialIOClass.SendCmd(_T("LC 1"));
		Sleep(3000);
		serialIOClass.SendCmd(_T("LC 0"));
	}




	serialIOClass.Close();*/
	system("pause");
}
