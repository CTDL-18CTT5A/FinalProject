#include"SupportOpenFolder.h"
#include "dirent.h"

vector<string> get_all_files_names_within_folder(string folder)
{
	vector<string> names;
	string search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

vector<string> GetAllNameFILE(char*& FolderName , string namefile)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	string LINK = namefile;

	vector<string> allName;

	hFind = FindFirstFile(namefile.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Khong tim thay folder\n", GetLastError());
		exit(0);
	}
	else
	{
		/*printf(TEXT("The first file found is %s\n"),
			FindFileData.cFileName);*/
		vector<string> s = get_all_files_names_within_folder(FindFileData.cFileName);
		//cout << s.size() << endl;
		for (int i = 0; i < s.size(); i++)
		{

			string name = LINK + "/" + s[i];
			allName.push_back(name);
		}
		FindClose(hFind);
	}

	return allName;
}


void ReadeFileList(vector<string>& fileList, string folderPath)
{
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(folderPath.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			fstream f(folderPath + "\\" + ent->d_name, ios::in);

			if (f.is_open())
			{
				fileList.push_back(folderPath + "\\" + ent->d_name);
				f.close();

			}


		}
		closedir(dir);
	}
}
void ReadeFolderList(vector<string>&listFolder , string folderPath)
{
	DIR* dir;
	struct dirent* ent;
	if ((dir = opendir(folderPath.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			fstream f(folderPath + "\\" + ent->d_name, ios::in);

			if (!f.is_open())
			{
				string temp(ent->d_name);
				if (temp.compare(".") != 0 && temp.compare("..") != 0)
				{
					listFolder.push_back(folderPath + "\\" + ent->d_name);
				}
			}


		}
		closedir(dir);
	}
}