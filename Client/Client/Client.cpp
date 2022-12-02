#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#pragma warning(disable: 4996)

SOCKET Connection;
time_t t;

time_t* GetTime(time_t &t) {
	time(&t);
	return &t;
}
// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	rtrim(s);
	ltrim(s);
}

void GetCommand(std::string command[], int command_size) {
	std::string msg = "";
	std::string word = "";
	int word_count = 0;
	std::getline(std::cin, msg);
	trim(msg);
	msg += ' ';
	for (int i = 0; msg[i] != '\0'; i++)
	{
		if (msg == " ")
			break;
		if (msg[i] == '\\' && msg[i + 1] == ' ')
		{
			word += msg[i + 1];
			i = i + 1;
		}
		else if (msg[i] == ' ') {
			if (word_count < command_size)
			{
				command[word_count] = word;
			}
			word = "";
			word_count++;
		}
		else {
			word += msg[i];
		}
	}
	
}

bool ClientHandler(std::ofstream &log) {
	int msg_size;
	int iResult = WSAECONNRESET;
	iResult = recv(Connection, (char*)&msg_size, sizeof(int), NULL);
	if (iResult == 0)
	{
		return false;
	}
	char* msg = new char[msg_size + 1];
	msg[msg_size] = '\0';
	iResult = recv(Connection, msg, msg_size, NULL);
	log << "Client received: " << std::endl << msg << " |" << ctime(GetTime(t)) << std::endl;
	if (iResult == 0)
	{
		delete[] msg;
		return false;
	}
	else {
		std::cout << msg << std::endl;
		delete[] msg;
		return true;
	}
}

int main(int argc, char* argv[]) {
	std::ofstream log;
	log.open("clientLog.txt", std::ios_base::app);

	time(&t);
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		log << "Error" << " |" << ctime(GetTime(t)) << std::endl;
		exit(1);
	}


	Connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Connection == INVALID_SOCKET) {
		std::cout << "Error at socket\n";
		log << "Error at socket" << " |" << ctime(GetTime(t)) << std::endl;
		return 1;
	}

	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1031);
	addr.sin_family = AF_INET;


	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cout << "Error: failed connect to server.\n";
		log << "Error: failed connect to server." << " |" << ctime(GetTime(t)) << std::endl;
		WSACleanup();
		return 1;
	}
	std::cout << "Connected!\n";
	log << "Connected!" << ctime(GetTime(t)) << std::endl;

	std::string command[2] = { "", "" };
	bool cStatus = true;
	while (cStatus) {
		GetCommand(command, 2);

		int command_size = command[0].size();
		int arg_size = command[1].size();
		if (send(Connection, (char*)&command_size, sizeof(int), NULL) < 0 || send(Connection, command[0].c_str(), command_size, NULL) < 0 ||
			send(Connection, (char*)&arg_size, sizeof(int), NULL) < 0 || send(Connection, command[1].c_str(), arg_size, NULL) < 0) 
		{
			log << "Client send: " << command[0] << ' ' << command[1] << " |" << ctime(GetTime(t)) << std::endl;
			log << "Error send: " << command[0] << ' ' << command[1] << " |" << ctime(GetTime(t)) << std::endl;
			log << "Connection closed" << " | " << ctime(GetTime(t)) << std::endl;
			closesocket(Connection);
			WSACleanup();
			std::cout << "Error. Connection closed\n";
			break;
		}
		else {
			log << "Client send: " << command[0] << ' ' << command[1] << " |" << ctime(GetTime(t)) << std::endl;
			cStatus = ClientHandler(log);
		}
		command[0] = "";
		command[1] = "";
	}
	closesocket(Connection);
	WSACleanup();
	std::cout << "Connection closed\n";
	log << "Connection closed" << " | " << ctime(GetTime(t)) << std::endl;
	log.close();
	system("pause");
	return 0;
}