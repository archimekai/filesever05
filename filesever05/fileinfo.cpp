#include "fileinfo.h"



int getfolders(TCHAR * goalpath, vector<fileinfo> & files) {
	
	// check the input path plus 3 is not longer than MAX_PATH
	size_t pathlen;
	StringCchLength(goalpath, MAX_PATH, &pathlen);
	if (pathlen > (MAX_PATH - 3)) {
		_tprintf(TEXT("ERROR: Directory path is too long! \n"));
		return (-1);
	}

	TCHAR path[MAX_PATH];
	WIN32_FIND_DATA ffd;
	// Prepare path for Findfile
	StringCchCopy(path, MAX_PATH, goalpath);
	StringCchCat(path, MAX_PATH, TEXT("\\*"));
	HANDLE hFind = FindFirstFile(path, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		_tprintf(TEXT("findfirstfile error!"));
		if (_tcscmp(path + _tcslen(path) - 4, TEXT(".txt")) == 0) {
			return URL_IS_A_TXT;
		}
		else {
			return URL_IS_A_FILE;
		}
	}
	
//	LARGE_INTEGER filesize;
//	int i = 0;

	//files[i] = ffd;  // ??????????????????????
	fileinfo temp;
	StringCchCopy(temp.name, MAX_PATH, ffd.cFileName);
	temp.fd = ffd;
	temp.fullpath = tstring(goalpath) + TEXT("\\") + tstring(temp.name); //foolish  use tstring at all time in next version!
	temp.setFullHtmlUri();
	files.push_back(temp);

	while (FindNextFile(hFind, &ffd) != 0) {
		StringCchCopy(temp.name, MAX_PATH, ffd.cFileName);
		temp.fd = ffd;
		temp.fullpath = tstring(goalpath) + TEXT("\\") + tstring(temp.name); //foolish  use tstring at all time in next version!
		temp.setFullHtmlUri();
		files.push_back(temp);
	}

	FindClose(hFind);

	if (files.size() == 0) {
		return (-1); // confilct with getlasterror
	}
	return GetLastError();
}


int getfolders(tstring goalpath, vector<fileinfo> & files) {
	TCHAR * temp = tstring2ptchar(goalpath);
	int itemp = getfolders(temp, files);
	delete[] temp;
	return itemp;
}


int genFileInfoHtmlStr(vector<fileinfo> & files, tstring & str) {
	if (files.size() == 0) {
		str = str + tstring(TEXT("<p>No files or folders here!</p><p><a href = ""/..""><span class = ""filename"">..</span></a></p>"));
		return 2;// no files
	}

	for (vector<fileinfo>::iterator it = files.begin(); it != files.end(); it++) {


		str = str + tstring(TEXT("<p class = ""item"">"))
			+ tstring(TEXT("<a href = """))
			+ tstring((*it).fullHtmlUri)   //URL
			+ tstring(TEXT(""">"))
			+ tstring(TEXT("<span class = ""filename"">"))  // ???????  ???? ?????????
			+ tstring((*it).name)
			+ tstring(TEXT("</span></a></p>"));
			//+ tstring(TEXT("<span>"))
			//+ tstring(_tprintf_s(""));
			//?????????? ????????????
	}
	return 0;
}


int genDriveInfoHtmlStr(vector<fileinfo> & files, tstring & str) {
	if (files.size() == 0) {
		str = str + tstring(TEXT("<p>No files or folders here!</p><p><a href = ""/..""><span class = ""filename"">..</span></a></p>"));
		return 2;// no files
	}

	for (vector<fileinfo>::iterator it = files.begin(); it != files.end(); it++) {


		str = str + tstring(TEXT("<p class = ""item"">"))
			+ tstring(TEXT("<a href = ""/files/"))
			+ tstring((*it).name)   //URL
			+ tstring(TEXT(">"))
			+ tstring(TEXT("<span class = ""filename"">"))  // ???????  ???? ?????????
			+ tstring((*it).name)
			+ tstring(TEXT("</span></a></p>"));
		//+ tstring(TEXT("<span>"))
		//+ tstring(_tprintf_s(""));
		//?????????? ????????????
	}
	return 0;

}



int analyzeHtmlPath(tstring url, tstring & goalpath) {
	// deal with /.  and /..
	//auto it = url.rbegin();
	//++it;
	if (url[url.length() - 1] == '/') {
		url.resize(url.length() - 1);
	}
	TCHAR endchar = '.';
	TCHAR slash = '/';
	TCHAR antislash = '\\';
	tstring realpath;
	size_t kpos;
	
	TCHAR keyword[] = TEXT("/files/");
	kpos = url.find(keyword, 0);
	if (kpos == tstring::npos) {
		return PAGE_NOT_FONUND; // because of wrong url , not file-nonexist
	}
	size_t startCopyOffSet = kpos + _tcslen(keyword) + 1;  // +1: skip drive letter


	if (url[url.length() - 1] == endchar) {
		//++it;
		if (url[url.length() - 2] == endchar) {   // wtf ++it  ++it   -> it = ".."!!!
			// back to superior folder
			int pos = url.rfind(slash, url.length() - 4);  //  /..  3 charactors
			realpath.append(url, startCopyOffSet, pos - startCopyOffSet);
		}
		else {
			// ????
			realpath.append(url, startCopyOffSet, url.length() - 2 - startCopyOffSet);
		}
	}
	else{
		realpath.assign(url.begin() + startCopyOffSet, url.end());  // +1: skip drive letter
	}

	// replace %20
	realpath = _treplaceAll(realpath, tstring(TEXT("%20")), tstring(TEXT(" ")));
	// replace slash 
	for (auto it = realpath.begin(); it != realpath.end(); ++it) {
		if ((*it) == slash) {
			(*it) = antislash;
		}
	}
	// replace %dd%dd to chinese charactors
	realpath = _tdecode2zhcn(realpath);

	//add drive letter 
	goalpath = url[kpos + _tcslen(keyword)] + tstring(TEXT(":")) + realpath;
	//if (goalpath.length() == 2) {
	//	// just a drive letter
	//	//goalpath = goalpath + tstring(TEXT("\\"));
	//}

	return 0;
}


int getDriveLetters(_In_ vector<fileinfo> & files) {
//	TCHAR buf[MAX_PATH];
	fileinfo temp;
	TCHAR path[10] = { 0 };
	path[1] = ':';
	path[2] = '\\';
	unsigned int type;
	for (char c = 'A'; c <= 'Z'; ++c) {
		path[0] = c;
		type = GetDriveType(path);
		if (type == DRIVE_FIXED) {
			temp.name[0] = c;
			files.push_back(temp);
		}
	}
	return 0;
}

char * tstring2pchar(
	_In_	const tstring & str
	) {
	//if (buflen > str.length() + 1) {
	//	cout << "ERROR: Not Enough buffer!";
	//	return (-1); 
	//}

#ifdef _UNICODE
	char * pBinaryText;
	size_t textlen;
	textlen = WideCharToMultiByte(CP_UTF8, // new code in UTF8
		0,
		str.c_str(),
		-1, // calcuate length automatically
		NULL,
		0,
		NULL,
		NULL
		);
	pBinaryText = new char[textlen + 1]; //  why plus 1
	memset((void *)pBinaryText, 0, sizeof(char) * (textlen + 1));
	WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		pBinaryText,
		textlen,
		NULL,
		NULL);
#else
	char * pBinaryText = str.c_str();  // is there a 0 at the end fo buffer?
#endif
	return pBinaryText;
}

int char2tstring(char * str, tstring & outstr) {
#ifdef _UNICODE
	// ref: http://blog.163.com/tianshi_17th/blog/static/4856418920085209414977/
	size_t len ;
	size_t converted = 0;
	wchar_t * wstr;
	len = MultiByteToWideChar(CP_UTF8,
		0,
		str,
		-1,
		NULL,
		NULL);

	wstr = new wchar_t[len];
	memset(wstr, 0, len);
	MultiByteToWideChar(CP_UTF8,
		0,
		str,
		-1,
		wstr,
		len);

	//mbstowcs_s(&converted, wstr, len, str, _TRUNCATE);
	outstr = tstring(wstr);
	delete[] wstr;
#else
	outstr = tstring(str);
#endif
	return 0;
}



TCHAR * tstring2ptchar(const tstring & tstr) {
	TCHAR * result = new TCHAR[tstr.length() + 1]; // length() include 0?
	memset(result, 0, sizeof(TCHAR) * (tstr.length() + 1)); // may affect effiency
	for (unsigned int i = 0; i < tstr.length(); ++i) {
		result[i] = tstr[i];
	}
	return result;
}

tstring _treplaceAll(tstring str, tstring strtofind, tstring yourstr) {
	tstring result;
	bool isSame = 0;
	auto itf = strtofind.begin();
	for (auto it = str.begin(); it != str.end(); ++it) {
		isSame = 1;
		int i = 0;
		for (itf = strtofind.begin(); itf + i != strtofind.end(); ++i) {
			if ((*(it + i))  != (*(itf + i))) {
				isSame = 0;
				break;
			}
		}
		if (isSame) { //????????
			result.append(yourstr);
			it = it + strtofind.length() - 1;
		}
		else {
			result.append(1,(*it));
		}
	}

	return result;
}

tstring _tdecode2zhcn(tstring str) {
	tstring result;
	for (auto it = str.begin(); it != str.end(); ++it) {
		unsigned char ctemp[URL_BUFLEN] = { 0 };
		int i = 0;
		while (it != str.end() && (*it) == '%' ) {
			// if always % started utf 8
			ctemp[i] = hex2char(*(it + 1), *(it + 2));
			it = it + 3;
			++i;
		}
		if (i > 0) {
			tstring wstr;
			char2tstring((char *)ctemp, wstr);
			result.append(wstr);
		}
		if (it != str.end()) {
			result.append(1, (*it));
		}
		if (it == str.end()) {  // it may become it.end()
			break;
		}
		//if ((*it) == '%' && (*(it + 3)) == '%') { // must be successive
		//	//int i = 1;
		//	unsigned char c[4] = { 0 };
		//	c[0] = hex2char(*(it + 1), *(it + 2));
		//	c[1] = hex2char(*(it + 4), *(it + 5));  // little didan or big didan
		//	tstring wstr;
		//	char2tstring((char *)c, wstr);
		//	result.append(wstr);

		//	it = it + 5;
		//}
		//else {
			
		//}
	}
	return result;
}

unsigned char hex2char(TCHAR hexa, TCHAR hexb) {
	return getoffset(hexa) * 16 + getoffset(hexb);	
}

int getoffset(TCHAR hex) {
	if (hex >= 'a' && hex <= 'z') {
		return (hex - 'a' + 10);
	}
	else if (hex >= 'A' && hex <= 'Z') {
		return (hex - 'A' + 10);
	}
	else if (hex >= '0' && hex <= '9') {
		return (hex - '0');
	}
	else {
		return -1; //error
	}
}