////
//#include"SupportOpenFolder.h"
//#include"CompressFile.h"
//#include"CompressFolder.h"
//
//int main()
//{
//
//	//EncodeFolder();
//
//	//EncodeFile("C:\\Users\\Admin\\Desktop\\CompressFolderAllFormat\\Compress_Folder_All_Format\\CompressFileText\\ec.txt");
//	//ExportFile();
//	//ExportFolder();
//
//	return 0;
//
//}
//
//

//Nana test


#include <nana/gui/filebox.hpp>
#include <iostream>
#include<nana/gui.hpp>
#include <nana\gui\widgets\label.hpp>
#include <nana/gui/widgets/menubar.hpp>
#include <nana/gui/widgets/toolbar.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana\gui\widgets\button.hpp>
#include <nana/threads/pool.hpp>

#include"SupportOpenFolder.h"
#include"CompressFile.h"
#include"CompressFolder.h"
#include <thread>
#include<direct.h>
int main()
{



#pragma region MENU
	using namespace nana;

	form fm{ API::make_center(400, 150) };
	fm.caption("OZip 1.0");
	toolbar tbar{ fm, true, true };

	label lab{ fm, "<bold black size=16>Ozip version 1.0 </>" };


	lab.format(true);
	lab.hide();
	bool isCompress = false;

	//create treebox on menu bar
	menubar mnbar(fm);
	mnbar.push_back("&Compress");
	mnbar.push_back("&Extact");


	//Add on compress file
	mnbar.at(0).append("Compress file", [fm](menu::item_proxy& ip)
	{
		nana::filebox picker{ nullptr, true };

		auto paths = picker.show();
		if (paths.empty())
		{
			std::cout << "Cancelled" << std::endl;
		}
		else
		{
			clock_t begin = clock();

			string filename;
			//Tim duong link va nén
			for (auto& p : paths)
			{
				filename = p.string();
			}

			EncodeFile(filename);
			_fcloseall();
			clock_t end = clock();
			double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

			cout << "Thoi gian chay chuong trinh : " << time_spent << endl;


			nana::msgbox box(u8"POPUP");
			box << L"COMPRESS FILE COMPLETE";
			box.show();

		}
	});


	//Compress folder
	mnbar.at(0).append("Compress folder", [fm](menu::item_proxy& ip)
	{
		nana::folderbox picker;

		auto paths = picker.show();
		if (paths.empty())
		{
			std::cout << "Cancelled" << std::endl;
		}
		else
		{
			string folderName;
			for (auto& p : paths)
				folderName = p.string();
			EncodeFolder(folderName);

			nana::msgbox box(u8"POPUP");
			box << L"COMPRESS FOLDER COMPLETE";
			box.show();
		}
	});


	//Add on Extact

	mnbar.at(1).append("Extact Here", [fm](menu::item_proxy& ip)
	{
		nana::filebox picker{ nullptr, true };

		auto paths = picker.show();
		if (paths.empty())
		{
			std::cout << "Cancelled" << std::endl;
		}
		else
		{
			string LinkPath;

			for (auto& p : paths)
				LinkPath = p.string();

			FILE* fi = fopen(ToCharArray(LinkPath), "rb");
			char ch;
			fread(&ch, 1, 1, fi);
			cout << ch << endl;
			if (ch == '1')
			{
				ExportFile(LinkPath);
			}
			else if (ch == '2')
			{
				ExportFolder(LinkPath);
			}

			_fcloseall();

			nana::msgbox box(u8"POPUP");
			box << L"EXTACT COMPLETE";
			box.show();
		}
	});








	//Render
	fm.div("vert <><<><weight=40% text><>><><weight=16<><button><>><>");
	fm["text"] << lab;



	//Show the form

	fm.collocate();
	fm.show();
	exec();
#pragma endregion


}


	





