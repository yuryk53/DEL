/*19.01.14
Создано: Yuriy Bilyk
утилита чистит сьемные устройства после работы вируса*/
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <cstring>
#include <string>
#include <iomanip>
#include "md5.h"
using namespace std;

double VERSION=4.0;
double LAST_VERSION=3.0;

/*
0 = Black
2 = Green
4 = Red
6 = Yellow
7 = White
8 = Gray
*/

class color {
private:
	CONSOLE_SCREEN_BUFFER_INFO csbi;	//save last screen buffer info (color)
public:
	color(int color=8)
	{	
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi );
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}
	~color()
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), csbi.wAttributes); //return to last color
	}
};

bool RemDir(char *dname, bool verbose=false) //removes a directory
{
	color c(4);
	if(!RemoveDirectory(dname))
	{
		if(verbose) cout<<"RemoveDirectory fail, trying change attributes...\n";
		SetFileAttributes(dname, FILE_ATTRIBUTE_NORMAL);
		if(!RemoveDirectory(dname)) 
		{
			if(verbose) {
				cout<<"Failed to remove directory \""<<dname<<"\"\n";
				if(GetLastError()==ERROR_FILE_NOT_FOUND) cout<<"ERROR_FILE_NOT_FOUND\n";
			}

			return false;
		}
	}
	return true;
}

bool verifyLNK(char *fileName, bool verbose=false) //verify .lnk file by reading hex header, it should be 4c000114200000c000000046
{												   // or 76 0 0 0 1 20 2 0 0 0 0 0 192 0 0 0 0 0 0 70 (without hex)
	ifstream in(fileName, ios::binary);
	if(!in)
	{
		color c(4);
		if(verbose) cout<<"verifyLNK: File Open Error!";
		system("pause");
		return false;
	}

	unsigned header[20] = {76,0,0,0,1,20,2,0,0,0,0,0,192,0,0,0,0,0,0,70};
	for(int i=0; i<20; i++) //read header
	{
		if(header[i] != in.get()) 
		{
			color c(3);
			if(verbose) cout<<"File "<<fileName<<" header doesn't match LNK header!\n";
			return false; //etalon header doesn't match the actual file header
		}
	}
	return true;
}

bool delLnk(bool verbose=false) //deletes file with .lnk extension
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char lnk[] = "*.lnk";
	color c(4);
	
	hFind = FindFirstFile(lnk, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
    {
       if(GetLastError()==2) cout<<"# NO LINK FILES FOUND!\n";
       return false;
    } 
    else 
    {
		if(verifyLNK(FindFileData.cFileName, verbose)) //if file matches LNK header
		{
			if(!DeleteFile(FindFileData.cFileName)) // delete file, if it's link
			{
				//function failed to delete a file -> try to reset attributes of this file and delete again
				if(verbose) cout<<"Failed to delete file "<<FindFileData.cFileName<<" -> trying to reset attributes and delete again:\n";

				SetFileAttributes(FindFileData.cFileName, FILE_ATTRIBUTE_NORMAL);
				if(DeleteFile(FindFileData.cFileName)) //trying to delete (if function failes, it returns 0
				{
					color c(2);
					if(verbose) cout<<"\tFile delete OK\n";
				}
				else
				{
					color c(4);
					if(verbose) cout<<"\tFailed to delete file!\n";
				}

			}
		}
		
    }

	//search for other *.lnk files
	while(FindNextFile(hFind, &FindFileData)) //while the search is possible
	{
		if(verifyLNK(FindFileData.cFileName, verbose)) //if file matches LNK header
		{
			if(!DeleteFile(FindFileData.cFileName)) // delete file, if it's link
			{
				//function failed to delete a file -> try to reset attributes of this file and delete again
				if(verbose) cout<<"Failed to delete file "<<FindFileData.cFileName<<" -> trying to reset attributes and delete again:\n";

				SetFileAttributes(FindFileData.cFileName, FILE_ATTRIBUTE_NORMAL);
				if(DeleteFile(FindFileData.cFileName)) //trying to delete (if function failes, it returns 0
				{
					color c(2);
					if(verbose) cout<<"\tFile delete OK\n";
				}
				else
				{
					color c(4);
					if(verbose) cout<<"\tFailed to delete file!\n";
				}

			}
		}
	}
	FindClose(hFind);
	return true;
}

bool chgAttrib(bool verbose=false)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	
	color c(4);
	hFind = FindFirstFile("*.*", &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
    {
       if(verbose) cout<<"# NO FILES DETECTED | GetLastError code:"<<GetLastError()<<'\n';
       return false;
    }

	do //while the search is possible
	{
		if((GetFileAttributes(FindFileData.cFileName) & FILE_ATTRIBUTE_HIDDEN))	//search for hidden files
		{
			if(verbose) cout << FindFileData.cFileName <<'\n';
			SetFileAttributes(FindFileData.cFileName, FILE_ATTRIBUTE_NORMAL); // define normal attributes for file/folder
		}
	} while(FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
	return true;

}


bool md(const char *dname, bool verbose=false)
{
	color c(4);
	if(strlen(dname) <= _MAX_FNAME)
	{
		if(!CreateDirectory(dname, NULL))
		{
			if(verbose) cout<<"File \""<<dname<<"\" already exists, trying to replace...\n";
			SetFileAttributes(dname, FILE_ATTRIBUTE_NORMAL);
			if(!DeleteFile(dname))	//replace file
			{
				if(verbose) cout<<"# CAN'T DELETE FILE"<<'\n';
				return false;
			}
			if(CreateDirectory(dname, NULL)==ERROR_ALREADY_EXISTS)
			{
				if(verbose) cout<<"# FILE "<<dname<<" ALREADY EXISTS"<<"\nDirectory creation failed!\n";
				return false;
			}
		}
	}
	return true;
}


void menu()
{
	setlocale(LC_ALL, "Russian");
	cout<<"Версия "<<std::fixed<<std::setprecision(1)<<VERSION<<"\n"
		<<"Утилита удаляет все ярлыки и файл autorun.inf\n"
		<<"потенциально созданный вирусным исполняемым файлом.\n"
		<<"Если используется система выше Win Xp, требуется запуск\n"
		<<"от имени Администратора.\n"
		<<"--------------------------\n"
		<<"Утилита написана 28.06.14\n"
		<<"Copyright Yura Bilyk\n"
		<<"--------------------------\n";
	cout<<"\t\t---------------NOW WORKING---------------\n\n";
}


int FindRemovableDrive(char *drv) //no more drives = false
{
	//2 - Drive removable
	char lpBuffer[100];  //buffer with terminating null characters
	int count=0;

	lpBuffer[0]='\0';
	count = GetLogicalDriveStrings(100, lpBuffer);
	if(!strlen(lpBuffer)) return 0;

	//print removable
	char *p = lpBuffer;
	int rcount = 0;
	for(int i=0; i<(count/3); i++)
	{
		if(GetDriveType(p)==DRIVE_REMOVABLE)
		{
			for(int j=0; j<4; j++, drv++)
			{
				*drv = *(p+j);
			}
			rcount++;
		}
		p+=4;
	}
	return rcount;
}

void process(bool verbose=false)
{
	if(delLnk(verbose))
	{
		cout<<"+ LINK FILES HAVE BEEN DELETED\n";
	}
	if(!chgAttrib(verbose))
	{
		color c(4);
		cout<<"# ERROR changing attributes: try \'verbose\' command\n";
	}
	else
	{
		cout<<"+ ATTRIBUTES HAVE BEEN CHANGED\n";
	}
	if(!md("autorun.inf", verbose))
	{
		color c(4);
		cout<<"# ERROR creating a directory: try \'verbose\' command\n";
	}
	else
	{
		cout<<"+ AUTORUN DIRECTORY'S BEEN CREATED\n";
	}
	RemDir("RECYCLER",verbose);
}

bool IsAtFlash()	//return true, if programm is located at flash drive
{
	char curDir[256];		//current directory
	int bufCount=0;

	if(!(bufCount = GetCurrentDirectory(256, curDir))) return false; //failed to obtain current directory
	
	char VolumePathName[4];
	if(!(bufCount = GetVolumePathName(curDir, VolumePathName, 4))) return false; //failed to obtain current volume letter

	if(GetDriveType(VolumePathName)==DRIVE_REMOVABLE) 
	{
		return true;
	}
	else
	{
		return false;
	}	
}

bool verifyOldDel(WIN32_FIND_DATA &FindFileData)
{
     //verify that file size is less than 100KB
	int FileSize = FindFileData.nFileSizeLow;
	if(FileSize > 0 && FileSize < 102400)
	{
		//further research = reading file and verifying its checksum
		ifstream in(FindFileData.cFileName, ios::binary);
		if(!in) return false;

		string fileBuf("");
		while(!in.eof())
			fileBuf += in.get();
		in.close();

		string md5DEL_3_0 = "ed0d3ad9e312be32d6665de6da1be119";
		if(md5DEL_3_0 != md5(FindFileData.cFileName)) //comparing original checksum and checksum of file found
			return false;
		else return true;
	}
	else //file size > 100kb or the size is incorrect
		return false;

	return false;
}

void update(char *currentDirectoryAndFileName)		//if program finds the old version of itself on the drive, it updates it
{
	//find the program with the name ['DEL_'+LAST_VERSION].exe
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	char versionToString[20];
	strcpy(versionToString, (to_string(VERSION)).c_str());
	for(int i=0; i<20; i++)
	{
		if(versionToString[i]=='.') 
		{
				versionToString[i+2]='\0';
				break;
		}
	}

	//set current file name
	char currFileName[12];
	for(int i=0; i<strlen(currFileName); i++) 
		currFileName[i]='\0';
	strcpy(currFileName, "DEL_");
	strcat(currFileName, versionToString);
	strcat(currFileName, ".exe");

	char fileName[]="*DEL*.exe";
	
	hFind = FindFirstFile(fileName, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) 
    {
       return;
    }
	else
	{
		do {
			//verify that the file found is exactly the old version of DEL (using md5)
			if(verifyOldDel(FindFileData))
			{

				//1.copy new version of file
				CopyFile(currentDirectoryAndFileName, currFileName, true); //if new version already exists, function failes
				//2.delete the old one
				if(GetLastError() != ERROR_FILE_NOT_FOUND) //if new version was successfully copied or file already exists
				{
					if(!DeleteFile(FindFileData.cFileName)) // delete old file
					{
						//function failed to delete a file -> try to reset attributes of this file and delete again
						SetFileAttributes(FindFileData.cFileName, FILE_ATTRIBUTE_NORMAL);
						DeleteFile(FindFileData.cFileName); 
					}
				}
			}
		} while(FindNextFile(hFind, &FindFileData));
	}
	FindClose(hFind);
}




int main(int argc, char *argv[])
{

	bool verbose = false;
	menu();
	if(argc > 1)
	{
		string tmp = argv[1];
		tmp+='\0';
		if(tmp.compare("verbose"))
		{
			verbose = true;
		}
	}
	//---------------------------------

	//process(verbose);
	if(IsAtFlash())
	{
		process(verbose);
	}
	else
	{
		char drv[30];  // up to 10 removable devices :))
		char *p=drv;
		int removableCount = FindRemovableDrive(drv);
		if(!removableCount) 
		{
			color c(4);
			cout<<"NO REMOVABLE DRIVE WAS FOUND!!!\nPLEASE, INSERT FLASH DRIVE...\n";
		}
		for(int i=0; i<removableCount; i++)
		{
			color c(6);
			cout<<"\t\t----------PROCESSING DEVICE "<<p<<" ---------\n";
			SetCurrentDirectory(p);
			process(verbose);
			update(argv[0]); //argv[0] - current file dir
			p+=4;
			cout<<"\n\n\n";
		}
	}
	Sleep(2000);  //задержаться на 2 секунды
	//system("pause");
	return 0;
}