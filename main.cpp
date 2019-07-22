/*
///////////////////////////负样本增强/////////////////////////
*/
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>    
#include <opencv2/imgproc/imgproc.hpp>   
#include <iostream>  
#include <algorithm>
#include <io.h>
#include <string>
#include "tinyxml2.h"
#define N4Str2Char 200
#define SIZE_width 640
#define SIZE_height 480

using namespace std;
using namespace cv;
using namespace tinyxml2;



struct myrect{
	int xmin;
	int ymin;
	int xmax;
	int ymax;
	int longside;
	int shortside;
};


bool CmpSide(myrect a, myrect b){
	return a.shortside > b.shortside;
}


void Str2Char(string input, char* p){
	input.copy(p, input.size(), 0);
	*(p + input.size()) = '\0';
}



//////////////////////////////////确保负样本中只有一个框，简化操作////////////////////////
bool CheckOnly(char* xmlPath){
	XMLDocument doc;
	doc.LoadFile(xmlPath);
	XMLElement *annotations = doc.RootElement();
	XMLElement *object = annotations->FirstChildElement("object");
	int num = 0;
	while (object != NULL){
		num++;
		object = object->NextSiblingElement();
	}
	if (num > 1)
		return false;
	else
		return true;
}


//////////////////////////////////框赋值操作////////////////////////
void SetValue4Rect(myrect& rect, int xmin, int ymin, int xmax, int ymax){
	rect.xmin = xmin;
	rect.ymin = ymin;
	rect.xmax = xmax;
	rect.ymax = ymax;
	if (rect.xmax - rect.xmin >= rect.ymax - rect.ymin){
		rect.longside = rect.xmax - rect.xmin;
		rect.shortside = rect.ymax - rect.ymin;
	}
	else{
		rect.longside = rect.ymax - rect.ymin;
		rect.shortside = rect.xmax - rect.xmin;
	}
}


//////////////////////////////////从xml文件中提取框信息////////////////////////
void GetXmlRect(char* xmlPath,myrect& rect){
	XMLDocument doc;
	doc.LoadFile(xmlPath);
	XMLElement *annotations = doc.RootElement();
	XMLElement *object = annotations->FirstChildElement("object");
	XMLElement *bndbox = object->FirstChildElement("bndbox");
	int xmin = bndbox->FirstChildElement("xmin")->IntText();
	int ymin = bndbox->FirstChildElement("ymin")->IntText();
	int xmax = bndbox->FirstChildElement("xmax")->IntText();
	int ymax = bndbox->FirstChildElement("ymax")->IntText();
	SetValue4Rect(rect, xmin, ymin, xmax, ymax);
}


//////////////////////////////////找到负样本中可以PS的空位置////////////////////////
void GetPSpace(myrect& negative_rect, myrect& space_rect){
	vector<myrect> rectlist;
	rectlist.clear();
	myrect rect = negative_rect;
	myrect rect1, rect2, rect3, rect4, rect5, rect6, rect7, rect8;
	SetValue4Rect(rect1, 0, 0, rect.xmin, rect.ymin);
	rectlist.push_back(rect1);
	SetValue4Rect(rect2, rect.xmin, 0, rect.xmax, rect.ymin);
	rectlist.push_back(rect2);
	SetValue4Rect(rect3, rect.xmax, 0, SIZE_width, rect.ymin);
	rectlist.push_back(rect3);
	SetValue4Rect(rect4, rect.xmax, rect.ymin, SIZE_width, rect.ymax);
	rectlist.push_back(rect4);
	SetValue4Rect(rect5, rect.xmax, rect.ymax, SIZE_width, SIZE_height);
	rectlist.push_back(rect5);
	SetValue4Rect(rect6, rect.xmin, rect.ymax, rect.xmax, SIZE_height);
	rectlist.push_back(rect6);
	SetValue4Rect(rect7, 0, rect.ymax, rect.xmin, SIZE_height);
	rectlist.push_back(rect7);
	SetValue4Rect(rect8, 0, rect.ymin, rect.xmin, rect.ymax);
	rectlist.push_back(rect8);
	sort(rectlist.begin(), rectlist.end(), CmpSide);
	space_rect = rectlist[1];
}


int createXML(char* xmlPath, char* pic_filename, string x_min, string y_min, string x_max, string y_max)
{
	XMLDocument doc;

	//添加申明可以使用如下两行
	XMLDeclaration* declaration = doc.NewDeclaration();
	doc.InsertFirstChild(declaration);

	XMLElement* root = doc.NewElement("annotation");
	doc.InsertEndChild(root);

	XMLElement* folder = doc.NewElement("folder");
	XMLText* foldertext = doc.NewText("NegaSample");
	folder->InsertFirstChild(foldertext);
	root->InsertEndChild(folder);

	XMLElement* filename = doc.NewElement("filename");
	XMLText* filenametext = doc.NewText(pic_filename);
	filename->InsertFirstChild(filenametext);
	root->InsertEndChild(filename);

	XMLElement* size = doc.NewElement("size");
	root->InsertEndChild(size);


	XMLElement* width = doc.NewElement("width");
	XMLText* widthtext = doc.NewText("640");
	width->InsertFirstChild(widthtext);
	size->InsertEndChild(width);

	XMLElement* height = doc.NewElement("height");
	XMLText* heighttext = doc.NewText("480");
	height->InsertFirstChild(heighttext);
	size->InsertEndChild(height);

	XMLElement* depth = doc.NewElement("depth");
	XMLText* depthtext = doc.NewText("3");
	depth->InsertFirstChild(depthtext);
	size->InsertEndChild(depth);

	XMLElement* object = doc.NewElement("object");
	root->InsertEndChild(object);

	XMLElement* name = doc.NewElement("name");
	XMLText* nametext = doc.NewText("Head");
	name->InsertFirstChild(nametext);
	object->InsertEndChild(name);

	XMLElement* bndbox = doc.NewElement("bndbox");
	object->InsertEndChild(bndbox);

	char temp[N4Str2Char];
	XMLElement* xmin = doc.NewElement("xmin");
	Str2Char(x_min, temp);
	char* xmin_p = temp;
	XMLText* xmintext = doc.NewText(xmin_p);
	xmin->InsertFirstChild(xmintext);
	bndbox->InsertEndChild(xmin);

	XMLElement* ymin = doc.NewElement("ymin");
	Str2Char(y_min, temp);
	char* ymin_p = temp;
	XMLText* ymintext = doc.NewText(ymin_p);
	ymin->InsertFirstChild(ymintext);
	bndbox->InsertEndChild(ymin);

	XMLElement* xmax = doc.NewElement("xmax");
	Str2Char(x_max, temp);
	char* xmax_p = temp;
	XMLText* xmaxtext = doc.NewText(xmax_p);
	xmax->InsertFirstChild(xmaxtext);
	bndbox->InsertEndChild(xmax);

	XMLElement* ymax = doc.NewElement("ymax");
	Str2Char(y_max, temp);
	char* ymax_p = temp;
	XMLText* ymaxtext = doc.NewText(ymax_p);
	ymax->InsertFirstChild(ymaxtext);
	bndbox->InsertEndChild(ymax);

	return doc.SaveFile(xmlPath);
}


void GetFilesName(string path, vector<string>& files)
{
	//文件句柄
	intptr_t   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					GetFilesName(p.assign(path).append("/").append(fileinfo.name), files);
			}
			else
			{
				files.push_back((fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}


int main()
{
	vector<string> filesname;
	size_t pos;
	string neag_xml_dir = "NegaSample/Annotations";
	string nega_pic_dir = "NegaSample/JPEGImages";
	string nega_strength_save_xml_dir = "NegaSampleStrength/Annotations";
	string nega_strength_save_pic_dir = "NegaSampleStrength/JPEGImages";
	char* pos_xml_file_path = "pos1.xml";
	char* pos_pic_file_path = "pos1.jpg";
	string nega_xml_file_path;
	string nega_pic_file_path;  
	char  nega_xml[N4Str2Char];
	char  nega_pic[N4Str2Char];
	char*  nega_xml_p;
	char*  nega_pic_p;


	GetFilesName(neag_xml_dir, filesname);
	for (int i = 0; i < filesname.size(); i++){
		pos = filesname[i].find(".");
		filesname[i] = filesname[i].substr(0, pos);
	}

	Mat nega_img;
	Mat pos_img;
	Mat pos_imgROI;
	myrect negative_rect;
	myrect positive_rect;
	myrect space_rect; 

	pos_img = imread(pos_pic_file_path);
	GetXmlRect(pos_xml_file_path, positive_rect);
	pos_imgROI = pos_img(Rect(positive_rect.xmin - 10, positive_rect.ymin - 10, positive_rect.xmax - positive_rect.xmin + 20, positive_rect.ymax - positive_rect.ymin + 20));

	for (int i = 0; i < filesname.size(); i++){
		/////////////////////////读取xml文件/////////////////////////////////
		nega_xml_file_path = neag_xml_dir + '/' + filesname[i] + ".xml";
		nega_xml_file_path.copy(nega_xml, nega_xml_file_path.size(), 0);
		*(nega_xml + nega_xml_file_path.size()) = '\0';
		nega_xml_p = nega_xml;
		/////////////////////////读取图片/////////////////////////////////
		nega_pic_file_path = nega_pic_dir + '/' + filesname[i] + ".jpg";
		if (CheckOnly(nega_xml_p) == true){
			nega_img = imread(nega_pic_file_path);
			GetXmlRect(nega_xml_p, negative_rect);
			GetPSpace(negative_rect, space_rect);
			if (space_rect.xmin + pos_imgROI.cols > SIZE_width || space_rect.ymin + pos_imgROI.rows > SIZE_height){
				cout << filesname[i] << "  ERROR:" << "out of array,skip this negative sample" << endl;
			}
			else{
				/////////////////////////PS并存输出图片/////////////////////////////////
				pos_imgROI.copyTo(nega_img(Rect(space_rect.xmin, space_rect.ymin, pos_imgROI.cols, pos_imgROI.rows)));
				nega_pic_file_path = nega_strength_save_pic_dir + '/' + filesname[i] + ".jpg";
				imwrite(nega_pic_file_path, nega_img);

				/////////////////////////提取xml完整名，写入xml需要/////////////////////////////////
				nega_xml_file_path = nega_strength_save_xml_dir + '/' + filesname[i] + ".xml";
				nega_xml_file_path.copy(nega_xml, nega_xml_file_path.size(), 0);
				*(nega_xml + nega_xml_file_path.size()) = '\0';
				nega_xml_p = nega_xml;

				/////////////////////////提取图片完整名，写入xml需要//////////////////////////////
				nega_pic_file_path = filesname[i] + ".jpg";
				nega_pic_file_path.copy(nega_pic, nega_pic_file_path.size(), 0);
				*(nega_pic + nega_pic_file_path.size()) = '\0';
				nega_pic_p = nega_pic;


				//////////////////////////将坐标从int类型转化为string类型，写入xml需要///////////////////////////////
				stringstream ss1,ss2,ss3,ss4;
				ss1 << (space_rect.xmin + 10);
				string x_min = ss1.str();
				ss2 << (space_rect.ymin + 10);
				string y_min = ss2.str();
				ss3 << (space_rect.xmin + pos_imgROI.cols - 10);
				string x_max = ss3.str();
				ss4 << (space_rect.ymin + pos_imgROI.rows - 10);
				string y_max = ss4.str();

				//////////////////////////写入xml///////////////////////////////
				createXML(nega_xml_p, nega_pic_p, x_min, y_min, x_max, y_max);
				cout << filesname[i] << "has been finished" << endl;
			}
		}
		else
		{
			cout << "has two objects,skip this negative sample" << endl;;
		}
	}
	
	system("pause");
	return 0;
}