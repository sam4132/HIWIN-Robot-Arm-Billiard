#include "pch.h"
#include "stdafx.h"
#include "iostream"
#include <fstream>
#include "HRSDK.h"
#include <vector>
#include <string>
#include <Winsock2.h> 
#include "Mitsubishi.h"
#include "thread"
#include <conio.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)
#define M_PI 3.14159265358979323846
#include <cmath>
#ifdef x64
#pragma comment(lib, "HRSDK.lib")
#else
#pragma comment(lib, "HRSDK.lib")
#endif


using namespace System;
using namespace System::IO::Ports;

void recon_to_home(HROBOT device_id, double mom[6], double pocket2[6]);
void hit_retrun(SOCKET sockClient);
bool readMom(double* mom, SOCKET sockClient, double pocket2[6]);
void offset(HROBOT device_id, double pocket2[6]);
void hit_ball(HROBOT device_id, double* mom, double pocket2[6]);
void callibration(HROBOT device_id, double pocket2[6]);
void __stdcall callBack(uint16_t, uint16_t, uint16_t*, int) {

}
void JogByKeyDown(HROBOT, char, int);
void JogByKeyDown_wrist(HROBOT id, char keyNum, int type);
void WaitKeUp(HROBOT);
double mom[6] = { 0, 0, 0, 0, 0, 0 };
double home[6] = { 0,0,0,0,-90 ,0};
double recon[6] = { 0, 312.846, 501.48, -180, -15.397, 90 };  //old { 0 ,343.173, 474.121, 180, -16.202, 90}   8/19nice 0 312.846 501.48 -180 -15.397 90
int ball_dis = 45;
const char* baseCh = { "1" };
double pocket2c[6];
double ang[6] = { 0,0,0,0,0,0 };
int main(array<System::String^>^ args)
{
	char sdk_ver[50];
	char hrss_ver[50];
	double mom_pos[6] = { 0,0,0,0,0,0 };
	get_hrsdk_version(sdk_ver);
	std::cout << "SDK version: " << sdk_ver << std::endl;
	HROBOT device_id = open_connection("169.254.176.54", 1, callBack);
	clear_alarm(device_id);
	int confirm = 0;
	bool run = true;
	set_override_ratio(device_id, 100);
	double* mom_pointer = mom;
	double pocket2[6] = { 4.138, 531.653, 60, -180, 0, 90 };//10
	callibration(device_id, pocket2);
	pocket2[2] = pocket2[2] - 8;
	for (int i = 0; i < 6; i++) {
		
		pocket2c[i] = pocket2[i];
		std::cout << "pocket2c[i]" << pocket2c[i] << std::endl;
	}
	std::cout << "connecting port" << std::endl;
	///////////////////serial////////////////////////////////
	SerialPort port("COM5", 9600);
	port.Open();
	port.Write("1");
	std::cout << "connect port succes" << std::endl;

	/////////////////socket////////////////////////////////
	std::cout << "connecting socket " << std::endl;
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(8888);
	connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	std::cout << "connect socket succes" << std::endl;
	//////////////////socket////////////////////////////////
	String^ receivedData;
	String^ truedata = "1";
	String^ receivedData2;
	String^ truedata2 = "2";

	if (device_id >= 0) {
		get_hrss_version(device_id, hrss_ver);
		std::cout << "HRSS version: " << hrss_ver << std::endl;
		std::cout << "connect successful." << std::endl;
		if (get_motor_state(device_id) == 0) {
			set_motor_state(device_id, 1);   // Servo on
		}
		std::cout << "wait for switch" << std::endl;
		recon_to_home(device_id, mom,pocket2);
		while (1)
		{
			for (int i = 0; i < 6; i++)
			{
				pocket2[i] = pocket2c[i];
			}
			receivedData = port.ReadExisting();
			Sleep(100);
			std::cout << "WAIT" << std::endl;
			if (truedata == receivedData)
			{
				lin_pos(device_id, 0, 0, recon);
				Sleep(500);
				while (get_motion_state(device_id) == 5)
				{
					Sleep(200);
					printf("moving\n");;
				}
				std::cout << "BUTTON opened" << std::endl;
				send(sockClient, baseCh, strlen(baseCh) + 1, 0);
				readMom(mom_pointer, sockClient, pocket2);
				hit_ball(device_id, mom, pocket2);
				clear_alarm(device_id);
				port.Write("A");
				std::cout << "ARDUINO WRITEED" << std::endl;
				while (1)
				{
					receivedData2 = port.ReadExisting();
					Console::WriteLine(receivedData2);
					if (truedata2 == receivedData2)
						break;
				}
				port.Write("1");
				recon_to_home(device_id, mom, pocket2);
			}
		}
		std::cout << "\n Press \"Enter\" key to quit the program." << std::endl;
		std::cin.get();
		disconnect(device_id);
	}
	else {
		std::cout << "connect failure." << std::endl;
	}
	closesocket(sockClient);
	WSACleanup();
	return 0;
}

void callibration(HROBOT device_id, double pocket2[6]) {
	ptp_axis(device_id,0, home);
	Sleep(500);
	while (get_motion_state(device_id) == 5)
	{
		Sleep(200);
		printf("moving %d\n", get_motion_state(device_id));;
	}
	pocket2[5] = pocket2[5] - 90;
	double tempc[6] = { 38, 0, 0, 0, 0, 0 };
	double tempcx[6] = { 0, -38, 0, 0, 0, -180 };
	lin_pos(device_id, 0,0, pocket2);
	Sleep(500);
	while (get_motion_state(device_id) == 5)
	{
		Sleep(200);
		printf("moving\n");;
	}
	lin_rel_pos(device_id, 0, 0, tempc);
	Sleep(1000);
	set_override_ratio(device_id, 10);
	while (true) {
		clear_alarm(device_id);
		char keyNum = _getch();
		if (keyNum == 27) {
			break;
		}
		double pos[6];
		std::cout << std::endl;
		JogByKeyDown(device_id, keyNum, 0);
		get_current_position(device_id, pos);
		pos[0] = pos[0] - 38;
		for (int i = 0; i < 6; i++) {
			std::cout << pos[i] << " ";
			pocket2[i] = pos[i];
		}
		WaitKeUp(device_id);
	}
	pocket2[5] = pocket2[5] + 90;
	for (int i = 0; i < 6; i++) {
		std::cout << "pocket2[i]" << pocket2[i] << " ";

	}
	set_override_ratio(device_id, 100);
	lin_pos(device_id, 0, 0, pocket2);
	Sleep(500);
	while (get_motion_state(device_id) == 5)
	{
		Sleep(200);
		printf("moving\n");;
	}
	lin_rel_pos(device_id, 0, 0, tempcx);
	Sleep(500);
	while (get_motion_state(device_id) == 5)
	{
		Sleep(200);
		printf("moving\n");;
	}
	set_override_ratio(device_id, 10);
	while (true) {
		clear_alarm(device_id);
		char keyNum = _getch();
		if (keyNum == 27) {
			break;
		}
		double pos[6];
		std::cout << std::endl;
		JogByKeyDown(device_id, keyNum, 0);
		get_current_position(device_id, pos);
		pos[1] = pos[1] + 38;
		pos[5] = pos[5] + 180;
		for (int i = 0; i < 6; i++) {
			std::cout << pos[i] << " ";
			pocket2[i] = pos[i];
		}
		WaitKeUp(device_id);
	}
	for (int i = 0; i < 6; i++) {
		std::cout << "pocket2[i]" << pocket2[i] << " ";

	}
	set_override_ratio(device_id, 100);
	ptp_axis(device_id,  0, home);
	Sleep(500);
	while (get_motion_state(device_id) == 5)
	{
		Sleep(200);
		printf("moving\n");;
	}
	lin_pos(device_id, 0, 0, recon);
	Sleep(500);
	while (get_motion_state(device_id) == 5)
	{	
		Sleep(200);
		printf("moving\n");
	}
	set_override_ratio(device_id, 10);
	while (true) {
		clear_alarm(device_id);
		char keyNum = _getch();
		if (keyNum == 27) {
			break;
		}
		double pos[6];
		std::cout << std::endl;
		JogByKeyDown_wrist(device_id, keyNum, 1);
		get_current_position(device_id, pos);
		for (int i = 0; i < 6; i++) {
			std::cout << pos[i] << " ";
			recon[i] = pos[i];
		}
		WaitKeUp(device_id);
	}
	set_override_ratio(device_id, 100);
}
void JogByKeyDown(HROBOT id, char keyNum, int type) {

	switch (keyNum) {
	case 'A':
	case 'a':
		jog(id, type, 0, 1);
		break;
	case 'D':
	case 'd':
		jog(id, type, 0, -1);
		break;
	case 'W':
	case 'w':
		jog(id, type, 1, 1);
		break;
	case 'S':
	case 's':
		jog(id, type, 1, -1);
		break;
	case 'R':
	case 'r':
		jog(id, type, 2, 1);
		break;
	case 'F':
	case 'f':
		jog(id, type, 2, -1);
		break;
	}
}

void JogByKeyDown_wrist(HROBOT id, char keyNum, int type) {

	switch (keyNum) {
	case 'A':
	case 'a':
		jog(id, type, 1, 1);
		break;
	case 'D':
	case 'd':
		jog(id, type, 1, -1);
		break;
	case 'W':
	case 'w':
		jog(id, type, 2, 1);
		break;
	case 'S':
	case 's':
		jog(id, type, 2, -1);
		break;
	case 'R':
	case 'r':
		jog(id, type, 4, 1);
		break;
	case 'F':
	case 'f':
		jog(id, type, 4, -1);
		break;
	}
}

void WaitKeUp(HROBOT id) {

	while (GetAsyncKeyState(81) < 0 || \
		GetAsyncKeyState(87) < 0 || \
		GetAsyncKeyState(65) < 0 || \
		GetAsyncKeyState(83) < 0 || \
		GetAsyncKeyState(90) < 0 || \
		GetAsyncKeyState(88) < 0 || \
		GetAsyncKeyState(69) < 0 || \
		GetAsyncKeyState(82) < 0 || \
		GetAsyncKeyState(68) < 0 || \
		GetAsyncKeyState(70) < 0 || \
		GetAsyncKeyState(67) < 0 || \
		GetAsyncKeyState(86) < 0) {
		Sleep(5);
	}
	jog_stop(id);
}

void hit_ball(HROBOT device_id, double* mom, double pocket2[6]) {
	bool is_reachable = true;
	/*ptp_axis(device_id, 0, home);
	Sleep(1000);
	while (get_motion_state(device_id) == 5)
	{
		Sleep(200);
		printf("moving\n");
	}*/
	pocket2[0] = pocket2[0] - 347;
	for (int i = 0; i < 6; i++) {
		pocket2[i] = pocket2[i] + mom[i];
		std::cout << "ball position" << pocket2[i];
	}
	std::cout << std::endl;
	offset(device_id, pocket2);
retry:
	motion_reachable(device_id, pocket2, is_reachable);

	if (is_reachable) {
		lin_pos(device_id, 0, 0, pocket2);
		Sleep(1000);
		printf("yes its reachable\n");
		
		

		while (get_motion_state(device_id) == 5)
		{
			Sleep(200);
			printf("moving\n");;
		}
		get_current_joint(device_id, ang);
	}
	else if (pocket2[1] < 400 && pocket2[0] > 19 && pocket2[0] < 64) {
		pocket2[1] = pocket2[1] + 2.5;
		pocket2[2] = pocket2[2] - 0.1;
		printf("cant go there pocket2[1] < 400\n");
		goto retry;
	}
	else if (pocket2[1] > 400 && pocket2[0] > 19 && pocket2[0] < 64) {
		pocket2[1] = pocket2[1] - 2.5;
		pocket2[2] = pocket2[2] - 0.1;
		printf("cant go there pocket2[1] < 400\n");
		goto retry;
	}

	else {
		if (pocket2[1] < 400 && pocket2[0] > 19)
		{
			printf("pocket2[1] < 400 && pocket2[0] > 19 右下角底袋\n");
			for (int i = 0; i < 20; i++)
			{
				pocket2[0] = pocket2[0] - 2.5;
				pocket2[1] = pocket2[1] + 2.5;
				motion_reachable(device_id, pocket2, is_reachable);
				if (is_reachable)
				{
					goto retry;
				}
			}
		}
		else if (pocket2[1] < 400 && pocket2[0] < 19)
		{
			printf("pocket2[1] < 400 && pocket2[0] < 19 左下角底袋\n");
			for (int i = 0; i < 20; i++)
			{
				pocket2[0] = pocket2[0] + 2.5;
				pocket2[1] = pocket2[1] + 2;
				motion_reachable(device_id, pocket2, is_reachable);
				if (is_reachable)
				{
					goto retry;
				}
			}
		}
		else if (pocket2[1] > 400 && pocket2[0] < 19)
		{
			printf("pocket2[1] > 400 && pocket2[0] < 19 左上角底袋\n");
			for (int i = 0; i < 20; i++)
			{
				pocket2[0] = pocket2[0] + 2.5;
				pocket2[1] = pocket2[1] - 2.5;
				motion_reachable(device_id, pocket2, is_reachable);
				if (is_reachable)
				{
					goto retry;
				}
			}
		}
		else if (pocket2[1] > 400 && pocket2[0] > 19)
		{
			printf("pocket2[1] > 400 && pocket2[0] > 19 右上角底袋\n");
			for (int i = 0; i < 20; i++)
			{
				pocket2[0] = pocket2[0] - 2.5;
				pocket2[1] = pocket2[1] - 2.5;
				motion_reachable(device_id, pocket2, is_reachable);
				if (is_reachable)
				{
					goto retry;
				}
			}
		}
		else
		{
			mom[5] = mom[5] + 1;
			printf("cant go there mom[5] = %2f\n", mom[5]);
			for (int i = 0; i < 6; i++)
			{
				pocket2[i] = pocket2c[i];
			}
			pocket2[0] = pocket2[0] - 346;
			for (int i = 0; i < 6; i++) {
				pocket2[i] = pocket2[i] + mom[i];
				std::cout << pocket2[i] << std::endl;
			}
			offset(device_id, pocket2);
			goto retry;
		}
	}

}

void offset(HROBOT device_id, double pocket2[6]) {
	double radian = mom[5] * (M_PI / 180); // 将角度转换为弧度
	double cos_value = cos(radian); // 计算余弦值
	double sin_value = sin(radian); // 计算正弦值
	double offset[6] = { 0,0,0,0,0,0 };

	offset[0] = (ball_dis * cos(radian)) - (43 * sin(radian));
	offset[1] = (ball_dis * sin(radian)) + (43 * cos(radian));
	pocket2[0] = offset[0] + pocket2[0];
	pocket2[1] = offset[1] + pocket2[1];

}

void recon_to_home(HROBOT device_id, double mom[6] , double pocket2[6])
{
	double wait[6] = { 90 , 0,0,0,-90,0 };
	wait[5] = ang[5];
	ptp_axis(device_id, 0, wait);
	Sleep(500);
	while (get_motion_state(device_id) == 5)
	{
		Sleep(200);
		printf("moving\n");;
	}
	double waitt[6] = { 0,0,0,0,0,0 };
	waitt[5] = -ang[5];
	ptp_rel_axis(device_id, 0, waitt);
}


bool readMom(double* mom, SOCKET sockClient,double pocket2[6])
{
retry:
	char recvBuf[80];
	recv(sockClient, recvBuf, 80, 0);
	if (strcmp(recvBuf, "a") == 0) {
		return false;  // 服务器发送关闭命令，跳出循环
	}
	if (!recvBuf) {
		goto retry;
	}
	std::cout << recvBuf << std::endl;
	float num1, num2, num3;
	int kubian = 0;
	int calli = 0;
	int calliy = 0;
	sscanf(recvBuf, "%f %f %f %d %d %d", &num1, &num2, &num3, &kubian, &calli, &calliy);

	
	// 输出转换后的结果
	std::cout << "num1: " << num1 << "num2: " << num2 << "num3: " << num3 << " kubian: " << kubian << "calli"<< calli << std::endl;

	double num4, num5, num6;
	num4 = num1;
	num5 = num2;
	num6 = num3;
	mom[0] = num4 * 10;
	mom[1] = num5 * -10;
	mom[5] = num6 * -1;
	


	
	ball_dis = 55;
	if (kubian)
	{
		ball_dis = 35;
		pocket2[2] = pocket2[2] + 5;
	}
	if (calli == 1)
	{
		std::cout << "callix 反過來" << std::endl;
		pocket2[0] = pocket2[0] + 6;
	}
	if (calli == 2)
	{
		std::cout << "callix x正得" << std::endl;
		//pocket2[0] = pocket2[0] + 3.5;
	}
	if (calliy == 1)
	{
		std::cout << "calliy 3" << std::endl;
		pocket2[1] = pocket2[1] + 5;
	}
	if (calliy == 2)
	{
		std::cout << "calliy 1.5" << std::endl;
		//pocket2[1] = pocket2[1] - 2.5;
	}
	std::cout << "mom[5]" << mom[5] << std::endl;
}



