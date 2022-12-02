#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>
#pragma warning(disable: 4996)
time_t t;

time_t* GetTime(time_t& t) {
	time(&t);
	return &t;
}
void Who(int arg_size, std::string& res) {
	if (arg_size != 0)
	{
		res = "<Who> can`t have any argument.";

	}
	else
	{
		res = "Oleksandr Starzhynskyi, k-28, V6";
	}
}
void Show(std::filesystem::path& path, std::string argStr, std::string& res) {
	if (!std::filesystem::is_empty(path))
	{
		if (argStr != "")
		{
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (entry.path().filename().string().find(argStr) != std::string::npos)
				{
					res += entry.path().filename().string() + '\n';
				}
			}
			if (res == "")
			{
				res = "Elements is not exist.";
			}
		}
		else {
			for (const auto& entry : std::filesystem::directory_iterator(path))
				res += entry.path().filename().string() + '\n';
		}
	}
	else {
		res = "This directory is empty.";
	}
}
void Goto(std::filesystem::path& path, std::string argStr, std::string& res) {
	if (argStr == "../")
	{
		if (path.has_parent_path())
		{
			path = path.parent_path();
			res = path.string() + '\n';
			Show(path, "", res);
		}
		else {
			res = "The folder is not exist.";
		}

	}
	else {
		bool isExist = false;
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (entry.path().filename().string() == argStr && std::filesystem::is_directory(entry.path()))
			{
				isExist = true;
			}
		}
		if (isExist)
		{
			path /= argStr;
			res = path.string() + '\n';
			Show(path, "", res);
		}
		else {
			res = "The folder is not exist.";
		}
	}
}
void Default(std::string& res) {
	res = "Invalid command.";
}
void ClientHandler(SOCKET CurrentConnection, std::filesystem::path path, std::ofstream& log) {
	int command_size;
	int arg_size;
	int iResult;

	while (true) {
		iResult = recv(CurrentConnection, (char*)&command_size, sizeof(int), NULL);
		if (iResult > 0)
		{
			char* command = new char[command_size + 1];
			command[command_size] = '\0';
			bool cStatus = true;
			recv(CurrentConnection, command, command_size, NULL);
			std::string commandStr(command);
			recv(CurrentConnection, (char*)&arg_size, sizeof(int), NULL);
			char* arg = new char[arg_size + 1];
			arg[arg_size] = '\0';
			if (arg_size != 0)
			{
				recv(CurrentConnection, arg, arg_size, NULL);
			}
			std::string argStr(arg);
			std::string res = "";
			std::cout << "Server received: " << commandStr << ' ' << argStr << std::endl;
			log << "Server received: " << commandStr << ' ' << argStr << " |" << ctime(GetTime(t)) << std::endl;
			if (commandStr == "Who")
			{
				Who(arg_size, res);
			}else if (commandStr == "Goto")
			{
				Goto(path, argStr, res);
			}else if (commandStr == "Show") {
				Show(path, argStr, res);
			}else if (commandStr == "Close") {
				if (arg_size != 0)
				{
					res = "<Close> can`t have any argument.";

				}
				else
				{
					iResult = shutdown(CurrentConnection, SD_SEND);
					std::cout << "Server send: " << res << std::endl;
					log << "Server send: " << res << " |" << " |" << ctime(GetTime(t)) << std::endl;
					delete[] command;
					delete[] arg;
					break;
				}
			}else {
				Default(res);
			}
			int res_size = res.size();
			send(CurrentConnection, (char*)&res_size, sizeof(int), NULL);
			send(CurrentConnection, res.c_str(), res_size, NULL);
			std::cout << "Server send: " << res << std::endl;
			log << "Server send: " << res << " |" << ctime(GetTime(t)) << std::endl;
			delete[] command;
			delete[] arg;
			arg_size = 0;
			command_size = 0;
		}
		else {
			closesocket(CurrentConnection);
			WSACleanup();
			break;
		}
	}

}

int main(int argc, char* argv[]) {

	std::ofstream log;
	log.open("serverLog.txt", std::ios_base::app);

	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		log << "Error" << " |" << ctime(GetTime(t)) << std::endl;
		exit(1);
	}

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	if (sListen == INVALID_SOCKET) {
		std::cout << "Error\n";
		log << "Error" << " |" << ctime(GetTime(t)) << std::endl;
		WSACleanup();
		return 1;
	}
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1031);
	addr.sin_family = AF_INET;

	if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cout << "Bind failed.\n";
		log << "Bind failed." << " |" << " |" << ctime(GetTime(t)) << std::endl;
		closesocket(sListen);
		return 1;
	}
	if (listen(sListen, 1) == SOCKET_ERROR) {
		std::cout << "Error listening on socket.\n";
		log << "Error listening on socket." << " |" << ctime(GetTime(t)) << std::endl;
	}


	SOCKET newConnection = INVALID_SOCKET;

	if (newConnection == INVALID_SOCKET)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
	}

	if (newConnection == INVALID_SOCKET) {
		std::cout << "Error #2\n";
		log << "Error #2" << " |" << ctime(GetTime(t)) << std::endl;
		closesocket(sListen);
		WSACleanup();
	}
	else {
		std::filesystem::path path = std::filesystem::current_path();
		std::cout << "Client Connected!\n";
		log << "Client Connected!" << " |" << ctime(GetTime(t)) << std::endl;
		ClientHandler(newConnection, path, log);
		closesocket(sListen);
		WSACleanup();
		std::cout << "Connection closed\n";
		log << "Connection closed" << " |" << ctime(GetTime(t)) << std::endl;
	}

	log.close();
	system("pause");
	return 0;
}