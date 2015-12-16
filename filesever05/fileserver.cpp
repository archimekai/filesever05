#define _UNICODE 1


#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include "fileinfo.h"
// ??????????????

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"
#define DEFAULT_BUFLEN 4096
#define SEND_FILE_BUFFER_LEN 1024

#define debug(...) printf(__VA_ARGS__)

int handleRequest(SOCKET);



int main(int argc, char **argv)
{
	int iResult;

	// Initialize Winsock
	WSADATA wsaData;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	// Resolve the local address and port to be used by the server
	struct addrinfo *addr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addr);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;

	// Create a SOCKET for the server to listen for client connections
	// (1) YOUR CODE HERE:
	// ListenSocket =
	ListenSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	// Bind the TCP listening socket to the address
	// (2) YOUR CODE HERE:
	// iResult = 
	iResult = bind(ListenSocket, addr->ai_addr, (int)addr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}

	// addr is no longer needed
	freeaddrinfo(addr);

	// Listen on the socket
	// (3) YOUR CODE HERE:
	// iResult = 
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Forever loop for any incoming request
	while (true) {
		SOCKET ClientSocket = INVALID_SOCKET;

		// Accept a client socket
		// (4) YOUR CODE HERE:
		// ClientSocket = 
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}
		iResult = handleRequest(ClientSocket);
		if (iResult) {
			closesocket(ListenSocket);
			WSACleanup();
			return iResult;
		}
	}
}

int recvUntil(SOCKET sock, char *buf, int bufsize, const char str[])
{
	char c;
	int i = 0, iResult, str_len = strlen(str), str_i = 0;

	while (true) {
		iResult = recv(sock, &c, 1, 0);
		if (iResult < 0)
			return iResult;
		if (iResult == 0)
			return i;
		if (i < bufsize) {
			buf[i] = c;
			i++;
		}
		if (c == str[str_i]) {
			str_i++;
			if (str_i == str_len)
				return i;
		}
		else
			str_i = 0;
	}
}

int generateResponse(char *method, char *path, char *httpVer, char *headers, char *resbuf, int bufsize)
{
	_snprintf(resbuf, bufsize, "HTTP/1.0 200 OK\r\n"
		"Content-Type: text/html charset=UTF-8\r\n\r\n"
		"<h2>Hello!</h2> You are visiting %s using %s with %s. Your headers are <pre>%s</pre> <p>Available Drives(Folders):</p>", path, method, httpVer, headers);
	if (strlen(path) == 1) {
		vector<fileinfo> drives;
		getDriveLetters(drives);
		tstring drivesstr;
		genDriveInfoHtmlStr(drives, drivesstr);
		char * pdrivesstr = tstring2pchar(drivesstr);
		memcpy(resbuf + strlen(resbuf), pdrivesstr, strlen(pdrivesstr) + 1);
		return 0;
	}
	tstring goalpath;
	tstring url;
	char2tstring(path, url);// may convert to wchar directly
	if (analyzeHtmlPath(url, goalpath) != 404) {
		vector<fileinfo> files;
		int isfile = getfolders(goalpath, files);
		if (isfile == URL_IS_A_FILE) {
			_snprintf(resbuf, bufsize, "HTTP/1.0 200 OK\r\n"
				"Content-Type: application/octet-stream\r\n\r\n");
			return URL_IS_A_FILE;  //deal send outside this function 
		}
		else if (isfile == URL_IS_A_TXT) {
			_snprintf(resbuf, bufsize, "HTTP/1.0 200 OK\r\n"
				"Content-Type: text/plain \r\n\r\n");
			return URL_IS_A_FILE;  //deal send outside this function 
		}
		tstring filestr;
		genFileInfoHtmlStr(files, filestr);
		char * pfilestr = tstring2pchar(filestr);
		memcpy(resbuf + strlen(resbuf), pfilestr, strlen(pfilestr) + 1);
		return 0;
	}
	else {
		debug("404!");
		return PAGE_NOT_FONUND;
	}

	
}

int handleRequest(SOCKET ClientSocket)
{
	char recvbuf[DEFAULT_BUFLEN], method[10], path[DEFAULT_BUFLEN], httpVer[10], headers[DEFAULT_BUFLEN], response[DEFAULT_BUFLEN * 2];
	int iResult, iSendResult;

	debug("+++ Accept connection\n");

	// Receive the first line
	iResult = recvUntil(ClientSocket, recvbuf, sizeof(recvbuf) - 1, "\r\n");
	if (iResult < 0) {
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		return iResult;
	}
	// We may receive an empty request
	if (iResult > 2) {
		recvbuf[iResult - 2] = 0;
		printf(">>> %s\n", recvbuf);
		// Extract method, path and httpVersion
		// WARNING: sscanf may cause overflow!
		sscanf(recvbuf, "%s %s %s", method, path, httpVer);
		// Read headers
		// Hint: using recvUntil
		// (5) YOUR CODE HERE:
		// iResult = 
		iResult = recvUntil(ClientSocket, headers, sizeof(headers) - 1, "\r\n\r\n");
		if (iResult < 0) {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			return iResult;
		}
		headers[iResult - 4] = 0;
		int genRespResult = generateResponse(method, path, httpVer, headers, response, sizeof(response));

		// Send response
		// (6) YOUR CODE HERE:
		// iSendResult = 
		//if (genRespResult == 0) { // normal condition
			iSendResult = send(ClientSocket, response, strlen(response), 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				return iSendResult;
			}
			debug("<<< %s", response);
			debug("<<< Bytes sent: %d\n", iSendResult);
	//		}
	//	}
		if(genRespResult == URL_IS_A_FILE || genRespResult == URL_IS_A_TXT) {
			tstring goalpath;
			tstring url;
			char2tstring(path, url);
			analyzeHtmlPath(url, goalpath);
			TCHAR * pc = tstring2ptchar(goalpath);
			fstream filein;
			filein.open(pc, ios::binary | ios::in);
			//delete[] pc; ERROR why?
			char filebuffer[SEND_FILE_BUFFER_LEN];
			do {
				filein.read(filebuffer, SEND_FILE_BUFFER_LEN);

				iSendResult = send(ClientSocket, filebuffer, filein.gcount(), 0);
				if (iSendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					return iSendResult;
				}
			} while (filein.gcount() == SEND_FILE_BUFFER_LEN);

		}
	}

	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		return iSendResult;
	}

	closesocket(ClientSocket);
	debug("--- Connection closed\n\n");
	return 0;
}
