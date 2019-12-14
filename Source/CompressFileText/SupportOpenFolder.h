#pragma once
#pragma once
#include"CompressFile.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <direct.h>

using namespace std;

vector<string> get_all_files_names_within_folder(string folder);

vector<string> GetAllNameFILE(char*& FolderName , string filename);

void ReadeFolderList(vector<string>& listFolder, string folderPath);

void ReadeFileList(vector<string>& fileList, string folderPath);