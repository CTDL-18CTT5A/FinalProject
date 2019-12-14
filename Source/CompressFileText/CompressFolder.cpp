#include"SupportOpenFolder.h"
#include"CompressFolder.h"
#include<vector>
#include<iostream>
using namespace std;

string subFileName(string str)
{
	string word = "";
	for (auto x : str)
	{
		if (x == '\\')
		{
			word = "";
		}
		else
		{
			word = word + x;
		}
	}

	//Bo di duoi type
	string newWord;
	for (int i = 0; i < word.size(); i++)
	{
		if (word[i] == '.')
		{
			break;
		}
		newWord += word[i];
	}
	return newWord;
}

string subFolderName(string folderName)
{
	string word = "";
	for (auto x : folderName)
	{
		if (x == '\\')
		{
			word = "";
		}
		else
		{
			word = word + x;
		}
	}
	string newWord; //Muc tieu la bo di cai link tap tin cuoi cung (/....)
	return word;
}

void ConvertToBinArrayOfFolder(FILE* in, int indexStart, int* a, long long& k, long long viTriLe, long long endpos)
{
	long long EndFile = endpos;
	fseek(in, indexStart, SEEK_SET);
	int count = ftell(in);
	/////////////DANH DAU
	char ch;
	fread(&ch, sizeof(char), 1, in);
	while (count != viTriLe)
	{
		for (int i = 7; i >= 0; --i)
		{
			char s = ((ch & (1 << i)) ? '1' : '0');
			s -= 48;
			a[k] = s;
			k++;
		}
		fread(&ch, sizeof(char), 1, in);

		count++;
	}

	while (count != EndFile)
	{

		a[k] = int(ch - 48);
		k++;
		fread(&ch, sizeof(char), 1, in);
		count++;
	}

	cout << "Ftell : " << ftell(in) << endl;
}

//Lay size byte
long long getNumberOfFileAtIndex(FILE* in, int index, int posEnd)
{
	fseek(in, index, SEEK_SET);
	int thisPoint = ftell(in);
	return posEnd - thisPoint;

}


string ConvertToString(char* s)
{
	string str;
	for (int i = 0; i < strlen(s); i++)
	{
		str += s[i];
	}
	return str;
}

void EncodeFolder(string filename)
{
	char* FolderName;
	cout << filename << endl;
	//vector<string> allName = GetAllNameFILE(FolderName , filename);
	vector<string> allName;
	vector<string> listFolder;

	ReadeFileList(allName, filename);
	ReadeFolderList(listFolder, filename);
	
	if (allName.empty())
	{
		cout << "Folder is empty , can't compressed " << endl;
		return;
	}

	FolderName = ToCharArray(subFolderName(filename));

	string FName = ConvertToString(FolderName) + ".OZIP";

	cout << "Folder name : " << FName << endl;
	FILE* fileCompresss = fopen(FName.c_str() ,  "ab+");
	fprintf(fileCompresss, "2");
	//Ghi ra ten folder can nén , lưu lại dùng cho giải nén
	fprintf(fileCompresss, "%s†", FolderName);
	//Ghi ra so luong file tồn tại trong folder nén
	fprintf(fileCompresss, "%d", allName.size());

	fprintf(fileCompresss, "†");

	fclose(fileCompresss);

	int l = listFolder.size();
	for (int k = -1; k < l; k++)
	{
		if (k != -1)
		{
			allName.clear();
			ReadeFileList(allName, listFolder[k]);

			FILE* fileCompresss = fopen(FName.c_str(), "ab+");
			//ghi raTên folder nén
			fprintf(fileCompresss, "%s†", ToCharArray(subFileName(filename) + "\\" + subFileName(listFolder[k])));
			//Ghi ra so luong file tồn tại trong folder nén
			fprintf(fileCompresss, "%d", allName.size());

			fprintf(fileCompresss, "†");

			fclose(fileCompresss);

		}
		for (int i = 0; i < allName.size(); i++)
		{
			//Write name filete
			FILE* writeNametoFile = fopen(FName.c_str(), "ab+");
			//Lưu lại tên file để nén ra thì trả lại
			char* filename = ToCharArray(subFileName(allName[i]));
			cout << subFolderName(allName[i]) << endl;
			fwrite(filename, sizeof(char), strlen(filename), writeNametoFile);
			fwrite("†", 1, 1, writeNametoFile);
			fclose(writeNametoFile);

			//Sau khi ghi lại đầy đủ header thì bắt đầu mã hoá code
			char* s = ToCharArray(allName[i]);
			EncodeMultiFile(s, FName.c_str());

			FILE* fileCompresss = fopen(FName.c_str(), "ab+");
			fwrite("|||", 3, 1, fileCompresss);
			fclose(fileCompresss);
		}
		FILE* fin = fopen(FName.c_str(), "ab+");
		fwrite("///", 3, 1, fin);
		fclose(fin);
	}
	_fcloseall();

}

void DecodeFolder(FILE* in, FILE* out, int vtLE)
{
	int curPos = ftell(in);

	//______________Gán vtLe để lấy ra số bit lẻ_______________
	long long vtLe = vtLE;
	fseek(in, vtLe, SEEK_SET);
	char ch[2];
	ch[1] = '\0';
	fread(ch, sizeof(char), 1, in);
	int SoLuongBitLe = int(ch[0] - 48);



	//Sau khi lưu số lượng bit lẻ vì vtLe đang nằm ở cuối file , ta trừ đi số bit lẻ => ra đc vị trí số lẻ đầu tiên.
	for (int z = 0; z < SoLuongBitLe; z++)
		vtLe -= 1;

	fseek(in, curPos, SEEK_SET);

	int sl = 0;
	HuffData data;
	ReadHeaderFile(in, data, sl);



	//____________Define max size of bit array_____________
	int StartIndex = ftell(in);
	long long sizeOfBit = getNumberOfFileAtIndex(in, StartIndex, vtLE) * 8;


	int* bit = new int[sizeOfBit];
	long long n = 0;

	//fseek(in, curPos, SEEK_SET);
	fseek(in, StartIndex, SEEK_SET);


	//TU DAY

	NODE* pos = builfHuffmanTree(data, sl);
	NODE* ROOT = pos;

	int len = sizeOfBit / 8 - SoLuongBitLe;
	unsigned char* Memory;
	Memory = new unsigned char[len];
	fread(Memory, sizeof(char), len, in);
	cout << "len = " << len << endl;
	for (int i = 0; i < len; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			//Chuyen ve bit 0 hoac 1
			if ((Memory[i] >> (7 - j)) & 1)
				pos = pos->pRight;
			else
				pos = pos->pLeft;


			if (isLeaf(pos))
			{
				fwrite(&pos->_text, sizeof(char), 1, out);
				pos = ROOT;
			}
		}
	}
	fseek(in, vtLe, SEEK_SET);

	char* c = new char[SoLuongBitLe];
	fread(c, sizeof(char), SoLuongBitLe, in);
	c[SoLuongBitLe] = '\0';
	for (int i = 0; i < SoLuongBitLe; i++)
	{
		if (c[i] == '1')
			pos = pos->pRight;
		else if (c[i] == '0')
			pos = pos->pLeft;

		if (isLeaf(pos))
		{
			fwrite(&pos->_text, sizeof(char), 1, out);
			pos = ROOT;
		}

	}



}

void EncodeMultiFile(char* filename , const char * folderName)
{
	string namefile;

	for (int i = 0; i < strlen(filename); i++)
	{
		namefile += filename[i];
	}

	//Lấy ra type 
	string DuoiFile;
	int pos = namefile.find(".") + 1;
	DuoiFile = namefile.substr(pos, namefile.size());

	unsigned long long freq[256];


	//Mở file ra đọc
	FILE* file = fopen(filename, "rb+");
	FILE* fileCompresss = fopen(folderName, "ab+");
	HuffData data;

	int size = 0;

	data = ReadFileBin(file , freq , size);
	
	HuffMap map;

	HuffmanCompress(file, fileCompresss, map, data, size, filename , freq);
	_fcloseall();

	return;

}


vector<int> posStop(char* filename)
{
	char buff[4];
	vector<int> pos;
	FILE* fs = fopen(filename, "rb");
	if (fs != NULL) {
		if (3 == fread(buff, 1, 3, fs)) {
			do {
				if (_strnicmp(buff, "|||", 3) == 0)
					pos.push_back(ftell(fs) - 4);
				memmove(buff, buff + 1, 3);
			} while (1 == fread(buff + 2, 1, 1, fs));
		}
	}
	fclose(fs);

	return pos;
}


vector<int> posNewFile(char* filename)
{
	char buff[4];
	vector<int> pos;
	FILE* fs = fopen(filename, "rb");
	if (fs != NULL) {
		if (3 == fread(buff, 1, 3, fs))
		{
			do
			{
				if (_strnicmp(buff, "///", 3) == 0)
					pos.push_back(ftell(fs) - 2);
				memmove(buff, buff + 1, 3);
			} while (1 == fread(buff + 2, 1, 1, fs));
		}
	}
	fclose(fs);

	return pos;
}

//Export file ta chỉ cần đọc file cho tới khi gặp điểm quy ước thì dừng và xuất file đó ra , cứ như thế cho đến khi gặp điểm quy ước cuối cùng
void ExportFolder(string namefolder)
{
	vector<int> posSTOPFILE = posNewFile(ToCharArray(namefolder));
	vector<int> posEnd = posStop(ToCharArray(namefolder));
	FILE* header = fopen(ToCharArray(namefolder), "rb");
	rewind(header);
	fseek(header, 1, SEEK_SET);

	int posCTN = 0;
	for (int h = 0; h < posSTOPFILE.size(); h++)
	{

#pragma region Lấy ra tên file và tìm những vị trí mà file kết thúc để dừng đọc đúng lúc.



#pragma endregion
		//Neu chi co duy nhat mot file tuc la size = 0;
		if (posEnd.empty())
		{
			ExportFile(namefolder);
			return;
		}


#pragma region lấy ra tên folder cũ để tạo và ghi vào
		string folderName;
		char tempChar;
		fread(&tempChar, 1, 1, header);
		while (tempChar != '†')
		{
			folderName += tempChar;
			fread(&tempChar, 1, 1, header);
		}
		_mkdir(ToCharArray(folderName));
#pragma endregion
		


#pragma region Lấy ra số lượng file đã nén trong folder cũ
		long long numberOfFile = 0;
		char ch;
		fread(&ch, 1, 1, header);
		while (ch != '†')
		{
			numberOfFile = numberOfFile * 10 + int(ch - 48);
			fread(&ch, 1, 1, header);
		}
#pragma endregion





		int curpos = ftell(header);


		fseek(header, curpos, SEEK_SET);

		//Lưu lại vị trí cũ để thư mục cuối cùng có thể mang ra dùng vì posEnd bị giới hạn

		for (int i = posCTN; i < posEnd.size(); i++)
		{
			if (posEnd[i] > posSTOPFILE[h])
			{
				posCTN = i;
				break;
			}
			cout << "Pos END  = " << posEnd[i] << endl;
			cout << "POS STOP : " << posSTOPFILE[h] << endl;
			//Lấy từng tên file trong folder cũ từ header
			string nameOut;

			//Doc lay ten
			char ch;
			fread(&ch, 1, 1, header);
			while (ch != '†')
			{
				nameOut += ch;
				fread(&ch, 1, 1, header);
			}

			//Get type
			char NumberOfType;
			fread(&NumberOfType, sizeof(char), 1, header);
			int sz = int(NumberOfType - 48);
			char* type = new char[sz];
			fread(type, sizeof(char), sz, header);

			nameOut += ".";
			//Cong them duoi vao
			for (int i = 0; i < sz; i++)
			{
				nameOut += type[i];
			}
			cout << folderName+"\\"+nameOut << endl;

			FILE* out = fopen(ToCharArray(folderName+"\\"+nameOut), "wb");


			DecodeFolder(header, out, posEnd[i]);

			fclose(out);


			//Bo qua 3 byte ngan cach:

			char temp[5];
			fread(temp, sizeof(char), 4, header);

		}

		char temp[4];
		fread(temp, sizeof(char), 3, header);
	}





}