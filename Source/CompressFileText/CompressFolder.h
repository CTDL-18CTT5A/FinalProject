#include"SupportOpenFolder.h"
#include<vector>
#include<iostream>
using namespace std;

string subFileName(string str);

string subFolderName(string folderName);

void ConvertToBinArrayOfFolder(FILE* in, int indexStart, int* a, long long& k, long long viTriLe, long long endpos);


void EncodeFolder(string filename);

void DecodeFolder(FILE* in, FILE* out, int vtLE);

void EncodeMultiFile(char* filename , const char *FolderName);



vector<int> posStop(char* filename);


//Export file ta chỉ cần đọc file cho tới khi gặp điểm quy ước thì dừng và xuất file đó ra , cứ như thế cho đến khi gặp điểm quy ước cuối cùng
void ExportFolder(string filename);