
#pragma once
#include"CompressFile.h"
#include"CompressFolder.h"
#include <stdio.h>
#include<iostream>
#include<vector>
#include<sstream>
#include<bitset>
using namespace std;



bool isLeaf(NODE* root)
{
	return !(root->pLeft) && !(root->pRight);

}

void ArrayOutput(HuffMap &map , int index , int a[], int n)
{
	int i = 0;
	for (i; i < n; ++i)
	{
		map.BitArray[index][i] = a[i];
	}
	map.BitArray[index][i] = 2;

}



//Duyệt cây lưu dữ liệu vào map
void CompressFile(HuffMap &map , NODE* root, int arr[], int top , int &index)
{
	// Assign 0 to left edge and recur 
	if (root->pLeft) 
	{

		arr[top] = 0;
		CompressFile(map ,root->pLeft, arr, top + 1,index);
	}

	// Assign 1 to right edge and recur 
	if (root->pRight) 
	{

		arr[top] = 1;
		CompressFile(map,  root->pRight, arr, top + 1, index);
	}

	
	//Index là biến đếm những node lá ứng với các kí tự
	if (isLeaf(root)) 
	{ 
		map.charater[index] = root->_text;
		map.BitArray[index] = new int[top];
		ArrayOutput(map, index , arr, top);
		index++;
	}
}


string getType(char* s)
{
	string type;
	for (int i = strlen(s) - 1; i >= 0; i--)
	{
		if (s[i] == '.')
			break;

		type += s[i];

	}
	reverse(type.begin(), type.end());
	return type;
}

char* ToCharArray(string name)
{
	char* fileName = new char[name.length() + 1];
	strcpy(fileName, name.c_str());
	return fileName;
}
//Lưu đống dữ liệu vào map

void HuffmanCompress(FILE *fileInput , FILE *fileOut , HuffMap &map , HuffData  data, int size , char*nameFile , unsigned long long freq[256])
{
	// Construct Huffman Tree 
	NODE* root = builfHuffmanTree(data, size);
	// Print Huffman codes using 
	// the Huffman tree built above 
	int arr[1000];
	long long top = 0;

	map.BitArray = new int* [size];

	map.charater = new char[size];
	
	string typeFile = getType(nameFile);
	char* type = ToCharArray(typeFile);
	type[strlen(type)] = '\0';

	int index = 0;
	CompressFile(map , root, arr, top , index);
	
	//Write type
	char soluongDuoi = char(strlen(type) + 48);
	fwrite(&soluongDuoi, sizeof(char), 1, fileOut);
	fwrite(type, sizeof(char), strlen(type), fileOut);

	WriteHeaderFile(fileOut , data , freq);


	rewind(fileInput);
	//Lưu vào file


	//Dem so luong cac bit da ma hoa
	long long countBit8 = 0;
	//Tong cac bit da ghi ra
	long long countSumBit = 0;


	long long countBit = 0;
	long long sumBitOut = 0;
	//char bit8[9];
	bool bit8[8];
	unsigned char* Memory;
	fseek(fileInput, 0, SEEK_END);
	unsigned long long len = ftell(fileInput);
	fseek(fileInput, 0, SEEK_SET);
	Memory = new unsigned char[len];
	
	fread(Memory, sizeof(char), len, fileInput);
	
	fseek(fileInput, 0, SEEK_SET);

	for (int k = 0; k < len; k++)
	{
		int j = 0;
		const char* ptr = strchr(map.charater, Memory[k]);
		if (ptr)
		{
			int index = ptr - map.charater;
			while (map.BitArray[index][j] != 2)
			{
				countSumBit++;
				bit8[countBit++] = map.BitArray[index][j];
				if (countBit == 8)
				{

					unsigned char c = 0;

					for (int z = 0; z < 8; ++z)
						if (bit8[z])
							c |= (128 >> z);

					fwrite(&c, sizeof(unsigned char), 1, fileOut);
					countBit = 0;
					countBit8 += 8;
				}

				j++;
			}
		}
			
	}
	
	int soBitLe = 0;
	for (int z = 0; z < countBit; z++)
	{
		//fprintf(fileOut, "%c", bit8[z]);
		//cout << bit8[z];
		char c;
		if (bit8[z])
			c = '1';
		else
			c = '0';
		fwrite(&c, sizeof(char), 1, fileOut);
		soBitLe++;
	}
	char bl = char(soBitLe + 48);
	fwrite(&bl, sizeof(char), 1, fileOut);
	
	delete root;
	delete[] map.BitArray;
	delete[] map.charater;
	delete type;
	delete[] Memory;


	//fprintf(fileOut, "%d", soBitLe);

	
}


void WriteHeaderFile(FILE* Output , HuffData data , unsigned long long freq[256])
{
	int size = strlen(data.s);
	fprintf(Output, "%d", size);
	fprintf(Output, "†");
	//fwrite(&(size), sizeof(int), 1, Output);
	fwrite(freq, sizeof(unsigned long long) * 256, 1, Output);

}


void ReadHeaderFile(FILE* fileIN, HuffData& data , int &sl)
{
	char ch;
	//ch = fgetc(fileIN);
	//Get size
	int size = 0;
	fread(&ch, 1, 1, fileIN);

	while (ch!= '†')
	{
		int t = int(ch - 48);
		size = size * 10 + t;
		fread(&ch, 1, 1, fileIN);
	}

	data.s = new char[size];
	data.wei = new unsigned long long[size];

	int curpos = ftell(fileIN);

	fseek(fileIN, curpos, SEEK_SET);

	unsigned long long frequency[256];
	fread(frequency, sizeof(unsigned long long)*256, 1, fileIN);
	int count = 0;
	for (int i = 0; i < 256; i++)
	{
		
		if (frequency[i] != 0 && i!=char('\0'))
		{
			data.s[count] = i;
			if (data.s[count] == '\0')
				cout << "NULL kia dm" << endl;
			data.wei[count] = frequency[i];
			count++;
		}
	}
	data.s[count] = '\0';
	sl = count;
}



//Lay so size
long long getNumberOfFileAtIndex(FILE* in, int index)
{
	fseek(in, index, SEEK_SET);
	int thisPoint = ftell(in);
	fseek(in, 0, SEEK_END);
	long long maxsize = ftell(in);
	return maxsize - thisPoint;

}



//Hàm trả về vị trí cuối bit lưu số bit lẻ trả vệ  vị trí con trỏ ở bit lẻ
long long VitriLe(FILE* fileIN)
{

	fseek(fileIN, 0, SEEK_END);
	return ftell(fileIN) - 1;

}

void ConvertToBinArray(FILE* in, int indexStart, int* a , long long &k , long long viTriLe)
{
	long long EndFile = VitriLe(in);
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

	while (count !=EndFile)
	{
	
		a[k] = int(ch - 48);
		k++;
		fread(&ch, sizeof(char), 1, in);
		count++;
	}
}



void Decode(FILE* in, FILE* out)
{
	long long curpos = ftell(in);
	//Lay Bit Le
	//______________Gán vtLe để lấy ra số bit lẻ_______________
	long long vtLe = VitriLe(in);
	fseek(in, vtLe, SEEK_SET);
	char ch[2];
	ch[1] = '\0';
	fread(ch, sizeof(char), 1, in);

	int SoLuongBitLe = int(ch[0] - 48);
	//Sau khi lưu số lượng bit lẻ vì vtLe đang nằm ở cuối file , ta trừ đi số bit lẻ => ra đc vị trí số lẻ đầu tiên.
	for(int z = 0; z < SoLuongBitLe; z++)
		vtLe -= 1;
	fseek(in, curpos, SEEK_SET);

	int sl = 0;
	HuffData data;
	ReadHeaderFile(in, data,sl);

	//____________Define max size of bit array_____________
	int StartIndex = ftell(in);
	//cout << "Start index" <<StartIndex <<  endl;
	unsigned long long sizeOfBit = getNumberOfFileAtIndex(in, StartIndex)*8;
	

	fseek(in, StartIndex, SEEK_SET);


	//int size = strlen(data.s);
	//_________Tạo ra map dữ liệu____________

	NODE* pos = builfHuffmanTree(data, sl);	
	cout << pos->_text << endl;
	NODE* ROOT = pos;

	int len = sizeOfBit / 8 - SoLuongBitLe - 1;
	unsigned char* Memory;
	Memory = new unsigned char[len];
	fread(Memory, sizeof(char), len, in);
	Memory[len] = '\0';
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





void EncodeFile(string linkFile)
{

	string DuoiFile;
	int pos = linkFile.find(".") + 1;
	DuoiFile = linkFile.substr(pos, linkFile.size());

	//Lay ra ten cua Path
	string Namefile = subFileName(linkFile);

	unsigned long long freq[256];

	FILE* file = fopen(ToCharArray(linkFile), "rb");
	FILE* fileCompresss = fopen(ToCharArray(Namefile+".Ozip"), "wb");
	fprintf(fileCompresss, "1");
	HuffData data;
	int size = 0;

		//data = ReadFileExe(file, ToCharArray(linkFile) , freq);
	data = ReadFileBin(file, freq , size);

		

	HuffMap map;

	
	HuffmanCompress(file, fileCompresss, map, data, size, ToCharArray(linkFile) , freq);

	
}


void ExportFile(string filename)
{


	FILE* header = fopen(ToCharArray(filename), "rb");


	//bo qua byte đầu tiên là byte nhận diện file hay folder
	fseek(header, 1, SEEK_SET);
	

	char NumberOfType;
	fread(&NumberOfType, sizeof(char), 1, header);
	int sz = int(NumberOfType - 48);
	char* type = new char[sz];
	fread(type, sizeof(char), sz, header);

	string nameOut;
	for (int i = 0; i < filename.length(); i++)
	{
		if (filename[i] != '.')
			nameOut += filename[i];
		else
			break;
	}
	nameOut += ".";
	//Cong them duoi vao
	for (int i = 0; i < sz; i++)
	{
		nameOut += type[i];
	}

	FILE* out = fopen(ToCharArray(nameOut), "wb");
	Decode(header, out);
}


