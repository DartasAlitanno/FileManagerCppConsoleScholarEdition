/*
������ �������� �� ������ ��������� ���������, �.�. ��� ����� ���������� ������ ��� ��������� �������� ������� � ������� ����������,
��� �� ��������� ���������� ��� �������� � ���������, ����� ����� ���� �� �������� ����������� ������ �� ����� ������,
�� ��������� ������, ��, �� ����� ��������� ���������, ������ ���������� � ��������, ��� ��������� ����� ����������������

��� ������ ��������� �������� ����� C++ 17
*/
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <stack>
#include <map>
#include <windows.h>
#include <direct.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <io.h>
#define PATHBACK "..."
#define FRAME_LENGHTSIZE 58
#define FRAME_LENGHTSTR string(FRAME_LENGHTSIZE, ' ')
namespace f = std::filesystem;//��������� ���� ������������ ���� ��� ������ � �������� ��������, ����� ������� ������� �� ��������
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

//����������� �������
const HANDLE myConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
CONSOLE_SCREEN_BUFFER_INFO consoleBufferInfo;
enum ConsoleColor {
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Magenta = 5,
	Brown = 6,
	LightGray = 7,
	DarkGray = 8,
	LightBlue = 9,
	LightGreen = 10,
	LightCyan = 11,
	LightRed = 12,
	LightMagenta = 13,
	Yellow = 14,
	White = 15
};
struct structForCursorPosition { unsigned int X = 0; unsigned int Y = 0; };
structForCursorPosition getCursorPosition() {
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	structForCursorPosition cursorPosition;
	if (GetConsoleScreenBufferInfo(myConsoleHandle, &consoleInfo)) {
		cursorPosition.X = consoleInfo.dwCursorPosition.X;
		cursorPosition.Y = consoleInfo.dwCursorPosition.Y;
	}
	else {
		cursorPosition.X = 0;
		cursorPosition.Y = 0;
	}
	return cursorPosition;
}
void setCursor(short x, short y) { SetConsoleCursorPosition(myConsoleHandle, { x, y }); }
void setColor(int color, int backcolor) { SetConsoleTextAttribute(myConsoleHandle, color + backcolor * 16); }
template <class ColorItem>//������� ���-�� ������
void setStrColor(ColorItem text, int textColor, int backColor) {
	setColor(textColor, backColor);
	cout << text;
	setColor(White, Black);
}
void setStrColor(TCHAR* text, int textColor, int backColor) {
	setColor(textColor, backColor);
	std::wcout << text;
	setColor(White, Black);
}

//������� ���������
void consoleStartSetup() {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	SetConsoleTitleA("Console File Manager");
	system("mode con cols=120 lines=30");
	system("color 0F");
}

//�������� ����� � ������
class StorageItem {
protected:
	string name;
	string path;
	long long unsigned int size = 0;
	bool hide;
	bool readOnly;
	string getNameFromPath() {
		string name;
		for (int i = path.size(); i > 0; i--) {
			if (path[i] == '/') {
				int ij = i + 1;
				for (int j = 0; j < ((path.size() - i) - 1); j++, ij++) name.push_back(path[ij]);
				break;
			}
		}
		return name;
	}
public:
	tm timeCreate;
	StorageItem(string path) : path(path) {
		name = getNameFromPath();
	}
	void printTime(tm t) {
		if (t.tm_mday < 10) cout << '0';
		cout << t.tm_mday << ".";
		if (t.tm_mon < 10) cout << '0';
		cout << t.tm_mon << "." << 1900 + t.tm_year << " ";
		if (t.tm_hour < 10) cout << '0';
		cout << t.tm_hour << ":";
		if (t.tm_min < 10) cout << '0';
		cout << t.tm_min << ":";
		if (t.tm_sec < 10) cout << '0';
		cout << t.tm_sec;
	}
	void printSize() {
		string strSize = std::to_string(size);
		if (strSize.size() > 3) {
			for (int i = strSize.size() - 3; i > 0; i -= 3)
				strSize.insert(i, " ");
		}
		cout << strSize << " ����";
	}
	void printAttribute() {
		cout << "���� �����: ";
		(hide) ? setStrColor("��", LightGreen, Black) : setStrColor("���", LightRed, Black);
		cout << "; ������ ��� ������: ";
		(readOnly) ? setStrColor("��", LightGreen, Black) : setStrColor("���", LightRed, Black);
	}
	string getPath() { return path; }
	string getName() { return name; }
	void setSize(int size) { this->size = size; }
	void setAttribute(bool hide, bool readOnly) {
		this->hide = hide;
		this->readOnly = readOnly;
	}
};
//����� �����
class File : public StorageItem {
private:
public:
	tm timeAccess;
	tm timeWrite;
	File(string path) : StorageItem(path) { }
};
//����� ��������
class Directory : public StorageItem {
private:
	unsigned int fileCount;
	unsigned int folderCount;
public:
	Directory(string path) : StorageItem(path) {
		fileCount = 0;
		folderCount = 0;
	}
	void setCounter(unsigned int fileCount, unsigned int folderCount) {
		this->fileCount = fileCount;
		this->folderCount = folderCount;
	}
	void printContent() {
		cout << "������: " << fileCount << "; �����: " << folderCount;
	}
};

//������ ����������
class Panel {
private:
	string path;
	struct folderInfo {
		long long unsigned int size = 0;
		unsigned int fileCount = 0;
		unsigned int folderCount = 0;
		folderInfo operator+=(const folderInfo& sideFi) {
			this->size += sideFi.size;
			this->fileCount += sideFi.fileCount;
			this->folderCount += sideFi.folderCount;
			return *this;
		}
	};
	//��������� ������� ���������
	folderInfo getFolderSize(string path) {
		_finddata_t fda;
		intptr_t search;
		folderInfo fi;
		if ((search = _findfirst((path + "/*.*").c_str(), &fda)) != -1) {
			fi += getFolderSize(fda, path);
			while (!_findnext(search, &fda)) fi += getFolderSize(fda, path);
			_findclose(search);
		}
		return fi;
	}
	folderInfo getFolderSize(_finddata_t& structPtr, string path) {
		folderInfo fi;
		if (strcmp(structPtr.name, ".") && strcmp(structPtr.name, "..")) {
			if (structPtr.attrib & 0b10000) {
				fi.folderCount++;
				fi += getFolderSize(path + "/" + structPtr.name);
			}
			else {
				fi.fileCount++;
				fi.size += structPtr.size;
			}
		}
		return fi;
	}
	//��� ������
	string findFolderItem(string path, string name) {
		_finddata_t fda;
		intptr_t search;
		if ((search = _findfirst((path + "/*.*").c_str(), &fda)) != -1) {
			string res = findFolderItem(fda, path, name);
			if (res != "0") {
				_findclose(search);
				return res;
			}
			while (!_findnext(search, &fda)) {
				res = findFolderItem(fda, path, name);
				if (res != "0") {
					_findclose(search);
					return res;
				}
			}
		}
		_findclose(search);
		return "0";
	}
	string findFolderItem(_finddata_t& structPtr, string path, string name) {
		if (strcmp(structPtr.name, ".") && strcmp(structPtr.name, "..")) {
			if (!strcmp(structPtr.name, name.c_str())) return path + "/" + name;
			if (structPtr.attrib & 0b10000) {
				string res = findFolderItem(path + "/" + structPtr.name, name);
				if (res != "0") return res;
			}
		}
		return "0";
	}
	//���� ����������� ����� � ������
	void addElementToList(string path) {
		driveList.clear();
		_finddata_t fda;
		intptr_t search;
		if ((search = _findfirst((path + "/*.*").c_str(), &fda)) != -1) {
			addElementToList(fda, path);
			while (!_findnext(search, &fda)) addElementToList(fda, path);
			_findclose(search);
		}
	}
	void addElementToList(_finddata_t& structPtr, string path) {
		if (strcmp(structPtr.name, ".") && strcmp(structPtr.name, "..")) {
			if (structPtr.attrib & 0b10000) {
				Directory dir(path + "/" + structPtr.name);
				localtime_s(&dir.timeCreate, &structPtr.time_create);
				dir.setCounter(getFolderSize(path + "/" + structPtr.name).fileCount, getFolderSize(path + "/" + structPtr.name).folderCount);
				dir.setSize(getFolderSize(path + "/" + structPtr.name).size);
				dir.setAttribute(structPtr.attrib & 0b000010, structPtr.attrib & 0b000001);
				folderList.push_back(dir);
			}
			else {
				File file(path + "/" + structPtr.name);
				localtime_s(&file.timeCreate, &structPtr.time_create);
				localtime_s(&file.timeAccess, &structPtr.time_access);
				localtime_s(&file.timeWrite, &structPtr.time_write);
				file.setSize(structPtr.size);
				file.setAttribute(structPtr.attrib & 0b000010, structPtr.attrib & 0b000001);
				fileList.push_back(file);
			}
		}
	}
	//������������� ��������
	int askToDelete(string name) {
		interactFrameOpen();//��������
		string path = this->path + "/" + name;
		setCursor(42, 13);
		if (path.size() > 34) {
			if (name.size() > 31) {
				name.resize(31);
				name += "...";
			}
			setStrColor("..." + name, Black, Cyan);
		}
		else setStrColor(path, Black, Cyan);
		return askAccept("�� �������, ��� ������ ������� �������?", "��", "���");
	}
	//��� ������
	void getDriveList() {
		char dev[250];
		int size = GetLogicalDriveStringsA(250, dev);
		for (size_t i = 0; i < size; i++) if (dev[i] == '\\') dev[i] = '/';
		string drive;
		for (size_t i = 0; i < size; i++) {
			if (dev[i] == '\0') {
				driveList.push_back(drive);
				drive.clear();
				continue;
			}
			drive.push_back(dev[i]);
		}
	}
public:
	bool selectDrive;
	vector<File> fileList;
	vector<Directory> folderList;
	vector<string> driveList;
	Panel() {
		selectDrive = true;
		getDriveList();
	}
	Panel(string path) : path(path) { addElementToList(path); }
	//����������
	void updateList(string path) {
		this->path = path;
		folderList.clear();
		fileList.clear();
		addElementToList(path);
	}
	//���� ��������������
	static void interactFrameOpen() {//60 15 �����
		SetConsoleOutputCP(866);//������ ��������� ��� ��������
		for (int i = 10; i < 20; i++) {
			setCursor(40, i);//������������ �����
			cout << char(179);
			setCursor(80, i);//������������ ������
			cout << char(179);
		}
		for (int i = 40; i <= 80; i++) {
			setCursor(i, 10);//�������������� ������
			if (i == 40) cout << char(218);
			else if (i == 80) cout << char(191);
			else cout << char(196);
			setCursor(i, 20);//�������������� �����
			if (i == 40) cout << char(192);
			else if (i == 80) cout << char(217);
			else cout << char(196);
		}
		for (int i = 41; i < 80; i++) {
			for (int j = 11; j < 20; j++) {
				setCursor(i, j);
				cout << " ";
			}
		}
		SetConsoleOutputCP(1251);
	}
	static void interactFrameClose() {
		SetConsoleOutputCP(866);
		string space(41, ' ');
		for (int i = 10; i <= 20; i++) {
			setCursor(40, i);
			cout << space;
			setCursor(60, i);
			cout << char(179);
			setCursor(59, i);
			cout << char(179);
		}
		SetConsoleOutputCP(1251);
	}
	//�������� ���-��
	static int askAccept(string question, string strTrue, string strFalse, int shift = 0) {
		if (question.size() > 40) return false;
		int newX = 60 - (question.size() / 2);
		setCursor(newX, 11 + shift);
		cout << question;
		int arrow = 0;
		while (true) {
			//�����������
			if (arrow == 3) arrow = 1;
			if (arrow == 0) arrow = 2;
			//��������
			setCursor(52, 17);
			if (arrow == 1) setStrColor(strTrue, Black, LightGray);
			else cout << strTrue;
			setCursor(65, 17);
			if (arrow == 2) setStrColor(strFalse, Black, LightGray);
			else cout << strFalse;
			setCursor(80, 20);
			switch (_getch()) {
			case 224://�����������
				switch (_getch()) {
				case 75:
					arrow--;
					break;
				case 77:
					arrow++;
					break;
				}
				break;
			case 13://����
				return arrow;
			case 27://�����
				return 0;
				break;
			}
		}
		return 0;
	}
	//����� � ����� �� ����
	void enter(string name) {
		fileList.clear();
		folderList.clear();
		if (name == PATHBACK && path.size() <= 3) {
			getDriveList();
			return;
		}
		if (name == PATHBACK) {
			int i = path.size() - 1;
			for (i; path[i] != '/'; i--);
			path.erase(i, path.size());
			addElementToList(path + "/.");
		}
		else {
			if (selectDrive) {
				selectDrive = false;
				if(path[path.size() - 1] == '/') path.pop_back();
			}
			path += "/" + name;
			addElementToList(path);
		}
	}
	//������ ���������� ������
	int getSize() { return fileList.size() + folderList.size(); }
	string getPath() { return path; }
	//��������
	void deleteItem(string pathName) {
		if (selectDrive) return;
		if (pathName != "...") {
			if (askToDelete(pathName) == 1) {
				pathName = this->path + "/" + pathName;//� ���� ���������� ���
				//������� ������ ������
				char* cPath = new char[pathName.length() + 1];
				strcpy_s(cPath, pathName.length() + 1, pathName.c_str());
				//���������� � ����� ���� �� ������ ������ ��� ����
				cPath[strlen(cPath)] = 0;
				cPath[strlen(cPath) + 1] = 0;
				//������� � ����������� ��������� ���� �������� �������� �������
				SHFILEOPSTRUCTA strOper = { 0 };
				strOper.hwnd = NULL;
				strOper.wFunc = FO_DELETE;//��������
				strOper.pFrom = cPath;
				strOper.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
				//��������� �������� ��� ���� ���������
				SHFileOperationA(&strOper);
			}
			interactFrameClose();
		}
	}
	//��������
	void createItem() {
		if (selectDrive) return;
		interactFrameOpen();
		int result = askAccept("������� ����� ��� ����?", "����", "�����");
		if (result == 0) return;
		setCursor(52, 12);
		setStrColor("������� ��������:", LightMagenta, Black);
		setCursor(41, 13);
		string name;
		getline(cin, name);
		if (result == 1) {
			f::path newFile = this->path;//����� �������� ������� � �������� �������
			newFile += "/" + name;
			if (!exists(newFile)) {
				std::ofstream ofs(newFile);
				ofs.close();
			}
			else {
				setCursor(47, 14);
				setStrColor("����� ��� ��� ����������,", LightRed, Black);
				setCursor(50, 15);
				setStrColor("������ � ��������.", LightRed, Black);
				for (int i = 0; exists(newFile); i++) {
					name.resize(name.size() - 4);
					if (i > 0) name.resize(name.size() - 3);
					name += "(" + std::to_string(i + 1) + ")";
					name += newFile.extension().string();
					newFile = this->path + "/" + name;
				}
				std::ofstream ofs(newFile);
				ofs.close();
				Sleep(2500);
			}
		}
		else if (result == 2) {
			f::path newFolder = path + "/" + name;
			if (!exists(newFolder)) f::create_directories(newFolder);//������� ����������, ���� �� ���
			else {
				setCursor(47, 14);
				setStrColor("����� ��� ��� ����������,", LightRed, Black);
				setCursor(50, 15);
				setStrColor("������ � ��������.", LightRed, Black);
				for (int i = 0; exists(newFolder); i++) {
					//����������� � �������, ��� �������� ������������� ��������� ��� �����
					if (i > 0) name.resize(name.size() - 4);
					name += " (" + std::to_string(i + 1) + ")";
					newFolder = this->path + "/" + name;
				}
				f::create_directories(newFolder);
				Sleep(2500);
			}
		}
		interactFrameClose();
	}
	//�����������
	static void copyItem(string pathFrom, string pathTo) {
		if (pathFrom != "..." && pathTo != "...") {
			f::path from = pathFrom;
			f::path where = pathTo;
			if (exists(from)) {//���� ������� ��� ����������� ����������
				if (from.extension().string().size() > 0) {//���� ����
					if (equivalent(from, where)) {//���� ����/����� ����������
						for (int i = 0; exists(where); i++) {
							//����������� � �������, ��� �������� ������������� ��������� ��� �����
							pathTo.resize(pathTo.size() - 4);
							if (i > 0) pathTo.resize(pathTo.size() - 3);
							pathTo += "(" + std::to_string(i + 1) + ")";
							pathTo += from.extension().string();
							where = pathTo;
						}
					}
					f::copy_file(from, where);
				}
				else {//����� �������
					if (!exists(where)) f::create_directories(where);//������� ����������, ���� �� ���
					if (equivalent(from, where)) {//���� ����/����� ����������
						for (int i = 0; exists(where); i++) {
							//����������� � �������, ��� �������� ������������� ��������� ��� �����
							if (i > 0) pathTo.resize(pathTo.size() - 4);
							pathTo += " (" + std::to_string(i + 1) + ")";
							where = pathTo;
						}
					}
					f::copy(from, where, f::copy_options::recursive);
				}
			}
		}
	}
	//�����������
	static void replaceItem(string pathFrom, string pathTo) {
		copyItem(pathFrom, pathTo);
		f::path forDelete = pathFrom;
		f::remove_all(forDelete);
	}
	//�������������
	void renameItem(string name) {
		if (selectDrive) return;
		interactFrameOpen();
		setCursor(52, 12);
		setStrColor("������� ����� ��������: ", LightMagenta, Black);
		f::path oldAddress = this->path + "/" + name;
		f::path newAddress = oldAddress;
		while (exists(newAddress)) {
			setCursor(41, 13);
			string newName;
			getline(cin, newName);
			newAddress = this->path + "/" + newName;
			if (exists(newAddress)) {
				setCursor(47, 14);
				setStrColor("����� ��� ��� ����������.", LightRed, Black);
				cin.clear();
			}
			else {
				f::rename(oldAddress, newAddress);
				break;
			}
		}
		interactFrameClose();
	}
	//�����
	void findItem() {
		if (selectDrive) return;
		interactFrameOpen();
		setCursor(46, 11);
		setStrColor("������� �������� ��� ������:", LightCyan, Black);
		setCursor(41, 12);
		string name;
		getline(cin, name);
		string res = findFolderItem(this->path, name);
		if (res != "0") {
			setCursor(53, 13);
			setStrColor("������� �� ������", LightGreen, Black);
			setCursor(41, 14);
			if (res.size() > 36) {
				string shortName = res;
				shortName.resize(36);
				shortName += "...";
				setStrColor(shortName, LightGreen, Black);
			}
			else setStrColor(res, LightGreen, Black);
			int result = askAccept("������� � �������� ������?", "��", "���", 4);
			if (result == 1) {
				fileList.clear();
				folderList.clear();
				for (int i = res.size(); i > 0; i--) {
					if (res[i] == '/') {
						res.resize(i);
						break;
					}
				}
				updateList(res);
				interactFrameClose();
			}
		}
		else {
			setCursor(55, 13);
			setStrColor("�� �������", LightRed, Black);
			setCursor(59, 17);
			setStrColor("��", Black, LightGray);
			_getch();
		}
		interactFrameClose();
	}
};

//������������
class Interface {
private:
	Panel* leftPanel;
	Panel* rightPanel;
	//��������������
	int currentFrame;
	int leftArrow, rightArrow;
	bool selectSecondPath = false;
	string pathForFirsPath;
	string secondName;
	//��������� ����
	struct Frame {
		int line = 1;
		int size = 20;
		int begin = 0, end = size;
	};
	Frame leftFrame;
	Frame rightFrame;
	//��� ����������� ���� ����� � ������ ������
	vector<string> leftNamesView;
	vector<string> rightNamesView;
	//��������� ��� ��� ����� ������� �������
	void clearEmptySpace() {
		int x = getCursorPosition().X;
		if (x > 0 && x < 120) {
			if (x <= 0 || x == 59 || x == 60 || x >= 119) return;//���� ��������� ����� � ����� ��� �� �� ���������
			x--;//��������� ������� ����� �����
			if (x > FRAME_LENGHTSIZE) x -= 60;//���� ��������� �� ��������� ������� ����, �������� ������ ������ �������� ������ ������ � �������
			string space((FRAME_LENGHTSIZE - x), ' ');
			cout << space;
		}
	}
	//��� ���������� ������ �����������
	void fillListNames(vector<string>& viewList, const Panel* panel) {
		viewList.clear();
		if (!panel->selectDrive) viewList.push_back(PATHBACK);
		for (Directory d : panel->folderList) viewList.push_back(d.getName());
		for (File f : panel->fileList) viewList.push_back(f.getName());
		for (string s : panel->driveList) viewList.push_back(s);
	}
	//������ ������
	static void interfaceFrame() {
		SetConsoleOutputCP(866);
		for (int i = 2; i < 30; i++) {
			setCursor(0, i);//������������ �����
			cout << char(186);
			setCursor(59, i);//������������ �� ������
			cout << char(179);
			setCursor(60, i);
			cout << char(179);
			setCursor(119, i);//������������ ������
			cout << char(186);
		}
		for (int i = 0; i < 120; i++) {
			setCursor(i, 1);//�������������� ������
			if (i == 0) cout << char(201);
			else if (i == 60 || i == 59) cout << char(209);
			else if (i == 119) cout << char(187);
			else cout << char(205);
			setCursor(i, 23);//�������������� �����
			if (i == 0) cout << char(204);
			else if (i == 60 || i == 59) cout << char(216);
			else if (i == 119) cout << char(185);
			else cout << char(205);
			setCursor(i, 29);//�������������� ���
			if (i == 0) cout << char(200);
			else if (i == 60 || i == 59) cout << char(207);
			else if (i == 119) cout << char(188);
			else cout << char(205);
		}
		SetConsoleOutputCP(1251);
	}
	//����� �������
	void drawPanel(int selectedArrow, vector<string>& viewList, Frame& frame, const int place, Panel* panel) {
		frame.line = 3;
		for (int i = frame.begin; i < frame.end; i++) {
			if (i >= 0 && i < viewList.size()) {
				setCursor(place, frame.line);
				if (viewList[i].size() > 54) {
					string shortName = viewList[i];
					shortName.resize(51);
					shortName += "...";
					if (i == selectedArrow) setStrColor(shortName, Black, DarkGray);
					else cout << shortName;
				}
				else if (i == selectedArrow) setStrColor(viewList[i], Black, DarkGray);
				else cout << viewList[i];
				clearEmptySpace();
				frame.line++;
			}
			else {
				setCursor(place, frame.line);
				cout << FRAME_LENGHTSTR;
				frame.line++;
			}
		}
		//��������� � ���������
		frame.line = 24;
		if (selectedArrow <= panel->folderList.size() && selectedArrow > 0) {
			selectedArrow -= 1;//���������� �� ���������� ������ �������
			while (true) {
				if (frame.line > 27) break;
				setCursor(place, frame.line);
				switch (frame.line) {
				case 24:
					cout << "������: ";
					panel->folderList[selectedArrow].printTime(panel->folderList[selectedArrow].timeCreate);
					break;
				case 25:
					cout << "������: ";
					panel->folderList[selectedArrow].printSize();
					break;
				case 26:
					cout << "��������: ";
					panel->folderList[selectedArrow].printContent();
					break;
				case 27:
					cout << "��������: ";
					panel->folderList[selectedArrow].printAttribute();
					break;
				}
				frame.line++;
				clearEmptySpace();
			}
			setCursor(place, frame.line);
			cout << FRAME_LENGHTSTR;
		}
		else if (selectedArrow > 0 && selectedArrow > panel->folderList.size() && selectedArrow <= panel->fileList.size()) {
			selectedArrow -= (panel->folderList.size() + 1);//���������� �� ���������� ������ �������
			while (true) {
				if (frame.line > 28) break;
				setCursor(place, frame.line);
				switch (frame.line) {
				case 24:
					cout << "������: ";
					panel->fileList[selectedArrow].printTime(panel->fileList[selectedArrow].timeCreate);
					break;
				case 25:
					cout << "�������: ";
					panel->fileList[selectedArrow].printTime(panel->fileList[selectedArrow].timeWrite);
					break;
				case 26:
					cout << "������: ";
					panel->fileList[selectedArrow].printTime(panel->fileList[selectedArrow].timeAccess);
					break;
				case 27:
					cout << "������: ";
					panel->fileList[selectedArrow].printSize();
					break;
				case 28:
					cout << "��������: ";
					panel->fileList[selectedArrow].printAttribute();
					break;
				}
				frame.line++;
				clearEmptySpace();
			}
		}
		else if (viewList[selectedArrow] == "...") {
			int i = frame.line;
			for (i; i < 29; i++) {
				setCursor(place, i);
				clearEmptySpace();
			}
		}
	}
	//����� ������ �������
	void printPanelPath(Panel* panel, int place) {
		setCursor(place, 2);
		if (!panel->selectDrive) {
			if (panel->getPath().size() < FRAME_LENGHTSIZE) setStrColor(panel->getPath(), Black, Yellow);
			else {
				string path = panel->getPath();
				path.resize(55);
				path += "...";
				setStrColor(path, Black, Yellow);
			}
		}
		else setStrColor("����� �����", Black, Yellow);
		clearEmptySpace();
	}
	//�� �������
	void enterItem(int& selectedArrow, Panel* panel, vector<string>& viewList) {
		if (selectedArrow <= panel->folderList.size() && !panel->selectDrive) {//���� ������� �����
			if (panel->getPath().size() <= 3) panel->selectDrive = true;
			panel->enter(viewList[selectedArrow]);
			fillListNames(viewList, panel);
			selectedArrow = 0;
			printPanelPath(leftPanel, 1);
			printPanelPath(rightPanel, 61);
		}
		else if (selectedArrow > panel->folderList.size() && (selectedArrow - panel->folderList.size()) <= panel->fileList.size() && !panel->selectDrive) {//���� �� �����
			system(("\"" + panel->getPath() + "/" + viewList[selectedArrow] + "\"").c_str());
		}
		else if (selectedArrow <= panel->driveList.size() && panel->selectDrive) {
			panel->selectDrive = false;
			panel->updateList(viewList[selectedArrow]);
			fillListNames(viewList, panel);
		}
	}
	//����� ��������� ����
	void frameArrowLogic(int& selectedArrow, Frame& frame) {
		if (selectedArrow < frame.begin) {
			frame.begin = selectedArrow;
			frame.end = frame.begin + frame.size;
		}
		if (selectedArrow > frame.end - 1) {
			frame.end = selectedArrow + 1;
			frame.begin = frame.end - frame.size;
		}
	}
	//������� �����/����
	void arrowUp(int& selectedArrow, int size) { if (selectedArrow < size) selectedArrow++; }
	void arrowDown(int& selectedArrow) { if (selectedArrow > 0) selectedArrow--; }
	//��� ������������� �������� ����
	void checkAndPrintCurrentFrameID() {
		int one = 20;
		int two = 80;
		if (currentFrame == 2) {
			one = 80;
			two = 20;
		}
		setCursor(one, 23);
		setStrColor("<-current frame->", Yellow, Black);
		setCursor(two, 23);
		SetConsoleOutputCP(866);
		cout << string(17, char(205));
		SetConsoleOutputCP(1251);
	}
public:
	Interface() : leftArrow(0), rightArrow(0), currentFrame(1) {
		leftPanel = new Panel();
		rightPanel = new Panel();
		fillListNames(leftNamesView, leftPanel);
		fillListNames(rightNamesView, rightPanel);
	}
	//���������� �������
	static void constantGraphic() {
		interfaceFrame();
		setCursor(0, 0);
		setStrColor("F1 �������; F2 �������������; F3 �����������; F4 ����������; F5 �������; F6 �����; ESC �����; TAB ������������ ����.    ", White, Blue);
		setCursor(0, 0);
	}
	//������������� �������
	void cycleGraphic() {
		//���� ����� ������
		printPanelPath(leftPanel, 1);
		printPanelPath(rightPanel, 61);
		//������������� ����
		checkAndPrintCurrentFrameID();
		//�����
		/*drawDrive(leftArrow, leftFrame, 1, leftPanel);
		drawDrive(rightArrow, rightFrame, 61, rightPanel);*/
		//����� �������
		drawPanel(leftArrow, leftNamesView, leftFrame, 1, leftPanel);
		drawPanel(rightArrow, rightNamesView, rightFrame, 61, rightPanel);
		setCursor(119, 29);//����� ������ ����� ��������� ����� �������� ��� � ������ ������ ����
	}
	//�������������� � �������������
	void interact() {
		constantGraphic();
		while (true) {
			//�������� ���� ���������
			frameArrowLogic(leftArrow, leftFrame);
			frameArrowLogic(rightArrow, rightFrame);
			cycleGraphic();
			//������
			switch (_getch()) {
			case 224://�����������
				switch (_getch()) {
				case 72:
					if (currentFrame == 1) arrowDown(leftArrow);
					if (currentFrame == 2) arrowDown(rightArrow);
					break;
				case 80:
					if (currentFrame == 1) arrowUp(leftArrow, leftNamesView.size() - 1);
					if (currentFrame == 2) arrowUp(rightArrow, rightNamesView.size() - 1);
					break;
				}
				break;
			case 13://����
				if (currentFrame == 1) enterItem(leftArrow, leftPanel, leftNamesView);
				if (currentFrame == 2) enterItem(rightArrow, rightPanel, rightNamesView);
				break;
			case 0://������ F
				switch (_getch()) {
				case 59://1 �������
					if (currentFrame == 1) leftPanel->createItem();
					if (currentFrame == 2) rightPanel->createItem();
					break;
				case 60://2 �������������
					if (currentFrame == 1) leftPanel->renameItem(leftNamesView[leftArrow]);
					if (currentFrame == 2) rightPanel->renameItem(rightNamesView[rightArrow]);
					break;
				case 61://3 �����������
					if (selectSecondPath) {
						if (currentFrame == 1 && leftPanel->selectDrive) break;
						if (currentFrame == 2 && rightPanel->selectDrive) break;
						string secondPath;
						if (currentFrame == 1) secondPath = leftPanel->getPath() + "/";
						if (currentFrame == 2) secondPath = rightPanel->getPath() + "/";
						Panel::replaceItem(pathForFirsPath, secondPath + secondName);
						selectSecondPath = false;
						setCursor(30, 0);
						setStrColor("F3 �����������;", White, Blue);
						setCursor(119, 29);
					}
					else if (!selectSecondPath) {
						if (currentFrame == 1 && leftPanel->selectDrive) break;
						if (currentFrame == 2 && rightPanel->selectDrive) break;
						//�������������� � �������������
						Panel::interactFrameOpen();
						setCursor(48, 12);
						cout << "����� � ������ �������,";
						setCursor(49, 13);
						cout << "� ������� F3 ��� ���,";
						setCursor(51, 14);
						cout << "��� �����������.";
						int res = Panel::askAccept("�������� ������������ ��������,", "��", "������");
						Panel::interactFrameClose();
						if (res != 1) break;
						setCursor(30, 0);
						setStrColor("F3 �����������;", Black, LightGreen);
						setCursor(119, 29);
						if (currentFrame == 1) {
							secondName = leftNamesView[leftArrow];
							pathForFirsPath = leftPanel->getPath() + "/" + leftNamesView[leftArrow];

						}
						if (currentFrame == 2) {
							secondName = rightNamesView[rightArrow];
							pathForFirsPath = rightPanel->getPath() + "/" + rightNamesView[rightArrow];
						}
						selectSecondPath = true;
					}
					break;
				case 62://4 ����������
					//������ ���� ������������� ������ ���� filesystem ��� �����������
					if (selectSecondPath) {
						if (currentFrame == 1 && leftPanel->selectDrive) break;
						if (currentFrame == 2 && rightPanel->selectDrive) break;
						string secondPath;
						if (currentFrame == 1) secondPath = leftPanel->getPath() + "/";
						if (currentFrame == 2) secondPath = rightPanel->getPath() + "/";
						Panel::copyItem(pathForFirsPath, secondPath + secondName);
						selectSecondPath = false;
						setCursor(46, 0);
						setStrColor("F4 ����������;", White, Blue);
						setCursor(119, 29);
					}
					else if (!selectSecondPath) {
						if (currentFrame == 1 && leftPanel->selectDrive) break;
						if (currentFrame == 2 && rightPanel->selectDrive) break;
						//�������������� � �������������
						Panel::interactFrameOpen();
						setCursor(48, 12);
						cout << "����� � ������ �������,";
						setCursor(49, 13);
						cout << "� ������� F4 ��� ���,";
						setCursor(51, 14);
						cout << "��� �����������.";
						int res = Panel::askAccept("�������� ������������ ��������,", "��", "������");
						Panel::interactFrameClose();
						if (res != 1) break;
						setCursor(46, 0);
						setStrColor("F4 ����������;", Black, LightGreen);
						setCursor(119, 29);
						if (currentFrame == 1) {
							secondName = leftNamesView[leftArrow];
							pathForFirsPath = leftPanel->getPath() + "/" + leftNamesView[leftArrow];

						}
						if (currentFrame == 2) {
							secondName = rightNamesView[rightArrow];
							pathForFirsPath = rightPanel->getPath() + "/" + rightNamesView[rightArrow];
						}
						selectSecondPath = true;
					}
					break;
				case 63://5 �������
					//� �������������� WinApi
					if (currentFrame == 1) leftPanel->deleteItem(leftNamesView[leftArrow]);
					if (currentFrame == 2) rightPanel->deleteItem(rightNamesView[rightArrow]);
					break;
				case 64://6 �����
					if (currentFrame == 1) leftPanel->findItem();
					if (currentFrame == 2) rightPanel->findItem();
					break;
				}
				//������������� ����
				if ((leftPanel->getPath() == rightPanel->getPath() || pathForFirsPath == leftPanel->getPath() ||
					pathForFirsPath == rightPanel->getPath()) &&
					!leftPanel->selectDrive && !rightPanel->selectDrive) {
					if (leftArrow > 0) leftArrow--;
					if (rightArrow > 0) rightArrow--;
					leftPanel->updateList(leftPanel->getPath());
					rightPanel->updateList(rightPanel->getPath());
					fillListNames(leftNamesView, leftPanel);
					fillListNames(rightNamesView, rightPanel);
				}
				else if (currentFrame == 1 && !leftPanel->selectDrive) {
					if (leftArrow > 0) leftArrow--;
					leftPanel->updateList(leftPanel->getPath());
					fillListNames(leftNamesView, leftPanel);
				}
				else if (currentFrame == 2 && !rightPanel->selectDrive) {
					if (rightArrow > 0) rightArrow--;
					rightPanel->updateList(rightPanel->getPath());
					fillListNames(rightNamesView, rightPanel);
				}
				break;
			case 9://TAB
				currentFrame ^= 0b00000011;
				break;
			case 27://�����
				return;
				break;
			}
		}
	}
};

//����� ��� �������������� ������� �������
void checkAndSetConsoleWindowSize() {
	while (true) {
		GetConsoleScreenBufferInfo(myConsoleHandle, &consoleBufferInfo);
		if (consoleBufferInfo.dwSize.X > 120 || consoleBufferInfo.srWindow.Bottom > 30) {
			system("mode con cols=120 lines=30");
			Interface::constantGraphic();
		}
		Sleep(500);//���� �� ������� �� �� 30%+
	}
}

int main() {
	consoleStartSetup();
	std::thread flowForCheckConsoleSize(checkAndSetConsoleWindowSize);
	flowForCheckConsoleSize.detach();
	Interface program;
	program.interact();
	system("cls");
}