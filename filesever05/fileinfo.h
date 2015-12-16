// passed at 2015?12?14?20:03:46
// ver02 passed at 2015 12 15 03 13
// encoding difference  unicode to utf-8
// rename to fileio maybe better

#ifndef FILEINFO_H
#define FILEINFO_H

#define PAGE_NOT_FONUND	404
#define URL_IS_A_FILE	-10
#define URL_IS_A_TXT	-11 
#define URL_BUFLEN      1024
#include <iostream>
#include <Windows.h>
#include <strsafe.h>
#include <tchar.h>
#include <vector>
#include <string>



#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

using namespace std;




class fileinfo {
public:
	TCHAR name[MAX_PATH] = {0};
	tstring fullpath;
	tstring fullHtmlUri;
	WIN32_FIND_DATA fd;
	int setFullHtmlUri() {
		// replace
		TCHAR slash = '/';
		TCHAR antislash = '\\';
		fullHtmlUri = tstring(TEXT(""));
		//fullHtmlUri ;
		for (auto it = fullpath.begin(); it != fullpath.end(); ++it) {
			
			if ((*it) == antislash) {
				fullHtmlUri.append(TEXT("/"));
			}
			else if ((*it) == ' ') {
				fullHtmlUri.append(TEXT("%20"));
			}
			else {
				fullHtmlUri.append(1, (*it));
			}
		}
		fullHtmlUri = tstring(TEXT("/files/")) + fullpath[0] + fullHtmlUri.substr(2, fullHtmlUri.length() - 2);

		return 0;
	}
};


int getfolders(TCHAR * goalpath, vector<fileinfo> & files);  //??????const tchar *

int getfolders(
	_In_	tstring goalpath,
	_Out_	vector<fileinfo> & files
	);


// curpath  searth path with out the endng slash
int genFileInfoHtmlStr(vector<fileinfo> & files, tstring & str);

int genDriveInfoHtmlStr(
	_In_	vector<fileinfo> & drives,
	_Out_	tstring & str
	);

// url starts with slash and must contains valid path
// also deals with %20
int analyzeHtmlPath(tstring url, tstring & goalpath);

int getDriveLetters(_In_ vector<fileinfo> & files);

//int sendFile(ifstream file, socket sendSocket);


#pragma region TCHAR WCHAR
// convert tstring to char *
// need to delete p in UNICODE condition 
char * tstring2pchar(	_In_	const tstring & str);

// maybe i can sum the convertors of char and tchar to a package...
// convert char * to tstring
int char2tstring(char * str, tstring & outstr);



TCHAR * tstring2ptchar(const tstring & tstr);

// replace all given strings
tstring _treplaceAll(tstring str, tstring strtofind, tstring yourstr);

// replace all uft8 %dd%dd to chinese charactors
tstring _tdecode2zhcn(tstring str);

// connect 2 hex to a char
unsigned char hex2char(TCHAR hexa, TCHAR hexb);

int getoffset(TCHAR hex);
#pragma endregion
#endif