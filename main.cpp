///////////////////////////��������ǿ�ű�/////////////////////////
//v1.0����˵����
//1.��ȡָ������������Pͼ����������
//2.ֻ֧��ֻ��һ����ĸ�������Pͼ��
//3.������Pͼʱ��ѡȡ��ͼ����һСȦ�ķ�Χ�����Ƕ�Ӧ���겻�䡣
//
//v1.1����˵����
//1.�����˶��ֱ߽��жϡ�
//2.�����˶�����������������ȡ��
//
//v1.2����˵����
//1.����������ϵ������Сϵ����
//2.Pͼλ�ø���Ϊ��ѡ����ռ�����м䡣

#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>    
#include <opencv2/imgproc/imgproc.hpp>   
#include <iostream>  
#include <algorithm>
#include <io.h>
#include <string>
#include <cstdio>
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



//////////////////////////////////ȷ����������ֻ��һ���򣬼򻯲���////////////////////////
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


//////////////////////////////////��ֵ����////////////////////////
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


//////////////////////////////////��xml�ļ�����ȡ����Ϣ////////////////////////
void GetXmlRect(char* xmlPath, myrect& rect){
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


//////////////////////////////////�ҵ��������п���PS�Ŀ�λ��////////////////////////
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

	//�����������ʹ����������
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
	//�ļ����
	intptr_t   hFile = 0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼,����֮
			//�������,�����б�
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
	vector<string> nega_filename;
	vector<string> pos_filename;
	size_t pos;
	size_t posi_pos = 0;

	double expand_ratio = 0.2;
	double reduce_ratio = 0.2;
	int expand_width;
	int expand_height;
	bool is_reduced;
	int center_width;
	int center_height;

	Mat nega_img;
	Mat pos_img;
	Mat pos_imgROI;
	myrect negative_rect;
	myrect positive_rect;
	myrect space_rect;

	string nega_strength_save_xml_dir = "20190723-add-neg-jpg-test/20190524-shenlong-003-student-bus-out312-negpos/Annotations";
	string nega_strength_save_pic_dir = "20190723-add-neg-jpg-test/20190524-shenlong-003-student-bus-out312-negpos/JPEGImages";
	string neag_xml_dir = "20190723-add-neg-jpg-test/20190524-shenlong-003-student-bus-out312-neg/Annotations";
	string nega_pic_dir = "20190723-add-neg-jpg-test/20190524-shenlong-003-student-bus-out312-neg/JPEGImages";
	string pos_xml_dir = "20190723-add-neg-jpg-test/20190524-shenlong-003-student-bus-out312-pos/Annotations";
	string pos_pic_dir = "20190723-add-neg-jpg-test/20190524-shenlong-003-student-bus-out312-pos/JPEGImages";

	string nega_xml_file_path;
	string nega_pic_file_path;
	string pos_xml_file_path;
	string pos_pic_file_path;

	char nega_xml[N4Str2Char];
	char nega_pic[N4Str2Char];
	char pos_xml[N4Str2Char];
	char pos_pic[N4Str2Char];

	char* nega_xml_p;
	char* nega_pic_p;
	char* pos_xml_p;
	char* pos_pic_p;

	//////ÿ��ȷ����Ӧ����Ŀ¼��ͼƬ��xml�ļ��ж�����յģ����δ������Ƚ�������////////////////
	cout << "��Ҫ�����Ӧ����Ŀ¼֮ǰ������" << endl;
	system("pause");
	GetFilesName(nega_strength_save_xml_dir, pos_filename);
	for (int i = 0; i < pos_filename.size(); i++){
		pos_xml_file_path = nega_strength_save_xml_dir + '/' + pos_filename[i];
		pos_xml_file_path.copy(pos_xml, pos_xml_file_path.size(), 0);
		*(pos_xml + pos_xml_file_path.size()) = '\0';
		pos_xml_p = pos_xml;
		remove(pos_xml_p);
	}
	pos_filename.clear();

	GetFilesName(nega_strength_save_pic_dir, pos_filename);
	for (int i = 0; i < pos_filename.size(); i++){
		pos_xml_file_path = nega_strength_save_pic_dir + '/' + pos_filename[i];
		pos_xml_file_path.copy(pos_xml, pos_xml_file_path.size(), 0);
		*(pos_xml + pos_xml_file_path.size()) = '\0';
		pos_xml_p = pos_xml;
		remove(pos_xml_p);
	}
	pos_filename.clear();

	cout << "ALL CLEAR!" << endl;

	///////////////////////////������ϣ���ʼ��ȡҪ�õ��������븺��������////////////////////////////
	GetFilesName(neag_xml_dir, nega_filename);
	for (int i = 0; i < nega_filename.size(); i++){
		pos = nega_filename[i].find(".");
		nega_filename[i] = nega_filename[i].substr(0, pos);
	}

	GetFilesName(pos_xml_dir, pos_filename);
	for (int i = 0; i < pos_filename.size(); i++){
		pos = pos_filename[i].find(".");
		pos_filename[i] = pos_filename[i].substr(0, pos);
	}

	for (int i = 0; i < nega_filename.size(); i++){

		if (posi_pos > pos_filename.size() - 1){
			posi_pos = 0;
		}

		is_reduced = false;

		////////////////////////////��ȡ������xml�ļ�������ȡ��Ӧbox//////////////////////
		pos_xml_file_path = pos_xml_dir + '/' + pos_filename[posi_pos] + ".xml";
		pos_xml_file_path.copy(pos_xml, pos_xml_file_path.size(), 0);
		*(pos_xml + pos_xml_file_path.size()) = '\0';
		pos_xml_p = pos_xml;

		GetXmlRect(pos_xml_p, positive_rect);
		/////////////////////////////��������󳤺Ϳ������ֵ///////////////////////////////
		expand_width = int((positive_rect.xmax - positive_rect.xmin)*expand_ratio / 2);
		expand_height = int((positive_rect.ymax - positive_rect.ymin)*expand_ratio / 2);

		/////////////////////////////��ȡ������ͼƬ///////////////////////////////////////
		pos_pic_file_path = pos_pic_dir + '/' + pos_filename[posi_pos] + ".jpg";
		pos_pic_file_path.copy(pos_pic, pos_pic_file_path.size(), 0);
		*(pos_pic + pos_pic_file_path.size()) = '\0';
		pos_pic_p = pos_pic;
		pos_img = imread(pos_pic_p);
		if (pos_img.empty()){
			cout << " ERROR: read image error, check the path" << endl;
			continue;
		}

		posi_pos++;

		if (positive_rect.xmin - expand_width - 2 < 0 || positive_rect.ymin - expand_height - 2 < 0 ||
			positive_rect.xmax + expand_width + 2 > SIZE_width || positive_rect.ymax + expand_height + 2 > SIZE_height){
			cout << pos_filename[posi_pos] << " ERROR: " << "out of array,skip this pos sample" << endl;
			continue;
		}
		else{
			pos_imgROI = pos_img(Rect(positive_rect.xmin - expand_width, positive_rect.ymin - expand_height,
				positive_rect.xmax - positive_rect.xmin + expand_width * 2, positive_rect.ymax - positive_rect.ymin + expand_height * 2));
		}

		/////////////////////////��ȡ������xml�ļ�/////////////////////////////////
		nega_xml_file_path = neag_xml_dir + '/' + nega_filename[i] + ".xml";
		nega_xml_file_path.copy(nega_xml, nega_xml_file_path.size(), 0);
		*(nega_xml + nega_xml_file_path.size()) = '\0';
		nega_xml_p = nega_xml;
		/////////////////////////��ȡ������ͼƬ/////////////////////////////////
		nega_pic_file_path = nega_pic_dir + '/' + nega_filename[i] + ".jpg";
		if (CheckOnly(nega_xml_p) == true){
			nega_img = imread(nega_pic_file_path);
			GetXmlRect(nega_xml_p, negative_rect);
			GetPSpace(negative_rect, space_rect);
			if (space_rect.xmin < 0 || space_rect.ymin < 0 || space_rect.xmin + pos_imgROI.cols > SIZE_width || space_rect.ymin + pos_imgROI.rows > SIZE_height){
				cout << nega_filename[i] << " ERROR: " << "out of array,skip this negative sample" << endl;
				continue;
			}
			else{
				if (pos_imgROI.cols > (space_rect.xmax - space_rect.xmin) || pos_imgROI.rows > (space_rect.ymax - space_rect.ymin)){
					reduce_ratio = max(1.0 * pos_imgROI.cols / (space_rect.xmax - space_rect.xmin), 1.0*pos_imgROI.rows / (space_rect.ymax - space_rect.ymin));
					Size ResImgSiz = Size(pos_imgROI.cols / reduce_ratio, pos_imgROI.rows / reduce_ratio);
					resize(pos_imgROI, pos_imgROI, ResImgSiz, CV_INTER_CUBIC);
					is_reduced = true;
				}

				/////////////////////////PS�������ͼƬ����ѡ����������м�/////////////////////////////////
				center_width = (space_rect.xmin + space_rect.xmax) / 2 - (space_rect.xmin + space_rect.xmin + pos_imgROI.cols) / 2;
				center_height = (space_rect.ymin + space_rect.ymax) / 2 - (space_rect.ymin + space_rect.ymin + pos_imgROI.rows) / 2;
				pos_imgROI.copyTo(nega_img(Rect(space_rect.xmin + center_width, space_rect.ymin + center_height, pos_imgROI.cols, pos_imgROI.rows)));
				nega_pic_file_path = nega_strength_save_pic_dir + '/' + nega_filename[i] + "-negpos" + ".jpg";
				imwrite(nega_pic_file_path, nega_img);

				/////////////////////////��ȡxml��������д��xml��Ҫ/////////////////////////////////
				nega_xml_file_path = nega_strength_save_xml_dir + '/' + nega_filename[i] + "-negpos" + ".xml";
				nega_xml_file_path.copy(nega_xml, nega_xml_file_path.size(), 0);
				*(nega_xml + nega_xml_file_path.size()) = '\0';
				nega_xml_p = nega_xml;

				/////////////////////////��ȡͼƬ��������д��xml��Ҫ//////////////////////////////
				nega_pic_file_path = nega_filename[i] + "-negpos" + ".jpg";
				nega_pic_file_path.copy(nega_pic, nega_pic_file_path.size(), 0);
				*(nega_pic + nega_pic_file_path.size()) = '\0';
				nega_pic_p = nega_pic;


				//////////////////////////�������int����ת��Ϊstring���ͣ�д��xml��Ҫ///////////////////////////////
				stringstream ss1, ss2, ss3, ss4;
				string x_min;
				string y_min;
				string x_max;
				string y_max;
				if (is_reduced){
					ss1 << (space_rect.xmin + center_width + expand_width / reduce_ratio);
					x_min = ss1.str();
					ss2 << (space_rect.ymin + center_height + expand_height / reduce_ratio);
					y_min = ss2.str();
					ss3 << (space_rect.xmin + center_width + pos_imgROI.cols - expand_width / reduce_ratio);
					x_max = ss3.str();
					ss4 << (space_rect.ymin + center_height + pos_imgROI.rows - expand_height / reduce_ratio);
					y_max = ss4.str();
				}
				else{
					ss1 << (space_rect.xmin + center_width + expand_width);
					x_min = ss1.str();
					ss2 << (space_rect.ymin + center_height + expand_height);
					y_min = ss2.str();
					ss3 << (space_rect.xmin + center_width + pos_imgROI.cols - expand_width);
					x_max = ss3.str();
					ss4 << (space_rect.ymin + center_height + pos_imgROI.rows - expand_height);
					y_max = ss4.str();
				}


				//////////////////////////д��xml///////////////////////////////
				createXML(nega_xml_p, nega_pic_p, x_min, y_min, x_max, y_max);
				cout << nega_filename[i] << " has been finished" << endl;

			}
		}
		else
		{
			cout << " has two objects,skip this negative sample" << endl;;
		}

	}


	system("pause");
	return 0;
}