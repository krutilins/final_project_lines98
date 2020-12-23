#include "windows.h"
#include "resource.h"

#include "random.h"

#include <list>
#include <queue>
#include <stack>
#include <strstream>
#include <tchar.h>

#define MENU_HEIGHT 45

#define TOP_HEIGHT 46	// ������ ����� (��� ������������ ����) � ��������
#define CELL_SIZE 45	//������ ������� ������ � �������� (��� ��� ����������)

#define N1 4	//���������� �������� ��� ��������� ������
#define N2 12	//���������� �������� ��� ������ ������
#define N3 9	//���������� �������� ��� �������� ������

#define MAX_MAP_X 20	//������������ ������ ���� �� x
#define MAX_MAP_Y 12    //������������ ������ ���� �� y

struct {
	//��������� �������� ��� ���� ������
	int y;
	//���� ������ � ���� ������
	int color;
	//���� ������-��������� � ���� ������
	//(����� ������� ����� �������� ����� ��� ������, ����� color != pre_color)
	int pre_color;
	//����� ��������, ��������� � ������� ������, ��� ���������, ��������, ������ ������
	int num_pic;
}

// ������� ����
map[MAX_MAP_X][MAX_MAP_Y];

// ���� �������� ������
int ball_color;


struct info
{
	int score;
	int time;
	char name[30];
}
// ������ ����������
leaders[3];

// ����������� ���������, ����� ������� � �������
const info null_leader = { 10,3600,"noname" };

// ��������� �������� ���������� �����
int y_lines;

// ����� ����
int gametime;
// ����
int gamescore;
// ��� ����: 0-easy, 1-normal, 2-hard, 3-custom
int gametype;

// ������� ������ ����
int max_x; // �� x
int max_y; // �� y
// ���������� ������������ �����
int app_balls;
//���������� ��������� �����
int del_balls;


TCHAR MY_SOUND0[] = _T("res//move.wav");
TCHAR MY_SOUND1[] = _T("res//destroy.wav");
TCHAR MY_SOUND2[] = _T("res//fetus.wav");
TCHAR MY_SOUND3[] = _T("res//cantmove.wav");

HANDLE bmp_0, bmp_prestand, bmp_stand, bmp_jump[N2], bmp_explode[N3], bmp_appear[N1], bmp_numbers, bmp_points;
HANDLE* bmp[6];

HDC hDC;
HDC hCompatibleDC;
HWND hWnd;
HINSTANCE hInst;
TCHAR szTitle[] = "Lines";
TCHAR szWindowClass[] = "LINES";

RECT clRect;

// ����� "������"
class cell
{
public:
	int posx; // ������� ������ �� ���� (����� ������� �� 0)
	int posy; // ������� ������ �� ���� (������ ���� �� 0)
	bool operator ==(const cell& b) const {
		return (b.posx == posx && b.posy == posy);
	}
	bool operator !=(const cell& b) const {
		return (!(*this == b));
	}
	int& State() const {
		return map[posx][posy].y;
	}
	int& Color() const {
		return map[posx][posy].color;
	}
	int& PreColor() const {
		return map[posx][posy].pre_color;
	}
	int& NumPic() const {
		return map[posx][posy].num_pic;
	}

	// ������� "������"
	void ACell(int e) const
	{
		int& y = State();

		switch (y)
		{
			// ���������� ����
		case 0:
			if (e == 1) {
				z0();
				y = 3;
			}
			if (e == 2 && x0()) {
				z4();
				y = 1;
			}
			else if (e == 2) {
				z1();
				y = 1;
			}
			break;
			// ���������
		case 1:
			if (e == 1) {
				z2();
				y = 3;
			}
			if (e == 3) y = 2;
			break;
			// ��������� ����
		case 2:
			if (e == 3 && x1()) {
				z6();
				y = 3;
			}
			else if (e == 3) {
				z5();
			}
			break;
			// ��� �����
		case 3:
			if (e == 0 && x0()) {
				z4();
				y = 1;
			}
			else if (e == 0) {
				z3();
				y = 0;
			}
			if (e == 4) y = 5;
			if (e == 5) y = 4;
			break;
			// ��� �������
		case 4:
			if (e == 0) {
				z6();
				z3();
				y = 0;
			}
			if (e == 6) {
				z6();
				y = 3;
			}
			if (e == 5 && x2()) {
				z6();
			}
			else if (e == 5) {
				z5();
			}
			break;
			// �������� ����
		case 5:
			if (e == 4 && x3() && x0()) {
				z6();
				z4();
				y = 1;
			}
			else if (e == 4 && x3()) {
				z6();
				z3();
				y = 0;
			}
			else if (e == 4) {
				z5();
			}
			break;
		}

		DrawState();
	}

	void DrawState() const
	{
		SelectObject(hCompatibleDC, bmp[State()][NumPic()]);
		BitBlt(
			hDC,
			posx * CELL_SIZE,
			TOP_HEIGHT + posy * CELL_SIZE,
			CELL_SIZE, CELL_SIZE,
			hCompatibleDC,
			Color() * CELL_SIZE,
			0,
			SRCCOPY);
	}

private:
	//�������� ����������

	//��������� �������������� ���������
	bool x0() const
	{
		return PreColor() != -1;
	}
	//��������� ������ ��������� ����
	bool x1() const
	{
		return (NumPic() == N1 - 1);
	}
	//��������� ������ ������ ��� ����
	bool x2() const
	{
		return (NumPic() == N2 - 1);
	}
	//��������� ������ �������� ����
	bool x3() const
	{
		return (NumPic() == N3 - 1);
	}

	//��������� �����������

	//���������� � ������ ����������� ���
	void z0() const
	{
		Color() = ball_color;
	}
	//������������ ���������
	void z1() const
	{
		Color() = random(7);
	}
	//�������� ������
	void z2() const
	{
		PreColor() = Color(); Color() = ball_color;
	}
	//���������� � ������ � ���������� ����������� ���
	void z3() const
	{
		Color() = 0;
	}
	//������������ ���������
	void z4() const
	{
		Color() = PreColor(); PreColor() = -1;
	}
	//������� �� ����� ��������� �������� ��� �������� ���������
	void z5() const
	{
		++NumPic();
	}
	//������� �� ����� ������ �������� ��� �������� ���������
	void z6() const
	{
		NumPic() = 0;
	}

};

//������� ���
cell ball;
//��������� ��� (������ ������)
cell click_ball;

//������ ������������ �����
std::list<cell> appear_list;
//������ ����������� �����
std::list<cell> explode_list;
//���� �� �������� �������� ����������� ���
std::stack<cell> path;

std::list<cell>::iterator itr;


// ��������� �������, ����������� ����� � ���������
void ALines(int);
void z0();
void z1_1();
void z1_2();
void z1_3();
void z2_1();
void z2_2();
void z2_3();
void z2_4();
void z3_1();
void z3_2();
void z4_1();
void z4_2();

bool xk0();
bool xk1();
bool xk2();
bool x0();
bool x1();
bool x2();
bool x3();
bool x4();
bool x5();

bool FindEmptyCell(cell&);
void GenerateAppearList();
void CheckAppearList();
bool FindPath(const cell&, const cell&);
bool CheckLines(const cell&);
bool Valid(const cell&);

void GameOver();
void NewGame();
void DrawTime();
void DrawScore();
void DrawTop();

void CheckCustomParameters();
void GetInfo();
void WriteInfo();


ATOM				MyRegisterClass(HINSTANCE);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	Custom(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	BestResults(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	GetName(HWND, UINT, WPARAM, LPARAM);


//������� ������� ����
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	randomize();

	hInst = hInstance;
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_LINES);

	//������� ���� ��������� Windows
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}
//����������� ������ ����
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_LINES);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = (LPCSTR)IDC_LINES;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
	return RegisterClassEx(&wcex);
}
// �������� � ����������� ����
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_CAPTION | WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, 0, 1600, 600, NULL, NULL, hInstance, NULL);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}
// ������� ������� ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT ps;
	static RECT Rect;
	static HMENU hMenu;
	static HKEY hKey;
	static cell l;
	static int temp;
	static	FILE* f;

	switch (message) {
		// ��������� ��������� ��� �������� ����
	case WM_CREATE:
		randomize();

		bmp[0] = &bmp_0;
		bmp[1] = &bmp_prestand;
		bmp[2] = &bmp_appear[0];
		bmp[3] = &bmp_stand;
		bmp[4] = &bmp_jump[0];
		bmp[5] = &bmp_explode[0];

		bmp_0 = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_0));
		bmp_prestand = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PRESTAND));

		bmp_appear[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_APPEAR_1));
		bmp_appear[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_APPEAR_2));
		bmp_appear[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_APPEAR_3));
		bmp_appear[3] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_APPEAR_4));

		bmp_stand = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_STAND));

		bmp_jump[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_3));
		bmp_jump[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_2));
		bmp_jump[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_1));
		bmp_jump[3] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_2));
		bmp_jump[4] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_3));
		bmp_jump[5] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_STAND));
		bmp_jump[6] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_4));
		bmp_jump[7] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_5));
		bmp_jump[8] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_6));
		bmp_jump[9] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_5));
		bmp_jump[10] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_JUMP_4));
		bmp_jump[11] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_STAND));

		bmp_explode[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_1));
		bmp_explode[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_2));
		bmp_explode[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_3));
		bmp_explode[3] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_4));
		bmp_explode[4] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_5));
		bmp_explode[5] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_6));
		bmp_explode[6] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_7));
		bmp_explode[7] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_8));
		bmp_explode[8] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXPLODE_9));

		bmp_numbers = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_NUMBERS));
		bmp_points = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_POINTS));

		hDC = GetDC(hWnd);
		hCompatibleDC = CreateCompatibleDC(hDC);

		GetInfo();
		hMenu = GetSubMenu(GetMenu(hWnd), 0);
		CheckMenuItem(hMenu, gametype + IDM_EASY, MF_CHECKED);
		GetWindowRect(hWnd, &Rect);
		MoveWindow(hWnd, Rect.left, Rect.top, (CELL_SIZE + 1.666) * max_x, TOP_HEIGHT + (CELL_SIZE + 1.666) * max_y + MENU_HEIGHT, TRUE);
		NewGame();

		SetTimer(hWnd, 0, 1000, NULL);
		break;

		// ��������� ��������� �� ������� ����� ������ ����
	case WM_LBUTTONDOWN:
		click_ball.posx = LOWORD(lParam) / CELL_SIZE;
		click_ball.posy = (HIWORD(lParam) - TOP_HEIGHT) / 45;
		ALines(0);
		break;
		// ��������� ��������� �� �������
	case WM_TIMER:
		switch (LOWORD(wParam)) {
		case 0:
			gametime++;
			DrawTime();
			break;
		case 1:
			ALines(1);
			break;
		}
		break;
		// ��������� ��������� WM_COMMAND
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDM_EASY:
			max_x = 9;
			max_y = 6;
			app_balls = 2;
			del_balls = 4;
			break;
		case IDM_NORMAL:
			max_x = 9;
			max_y = 9;
			app_balls = 3;
			del_balls = 5;
			break;
		case IDM_HARD:
			max_x = 20;
			max_y = 12;
			app_balls = 10;
			del_balls = 4;
			break;
		case IDM_CUSTOM:
			DialogBox(hInst, (LPCTSTR)IDD_CUSTOMBOX, hWnd, (DLGPROC)Custom);
			CheckCustomParameters();
			break;
		}
		switch (LOWORD(wParam)) {
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_EASY:
		case IDM_NORMAL:
		case IDM_HARD:
		case IDM_CUSTOM:
			CheckMenuItem(hMenu, gametype + IDM_EASY, MF_UNCHECKED);
			CheckMenuItem(hMenu, LOWORD(wParam), MF_CHECKED);
			gametype = LOWORD(wParam) - IDM_EASY;
			GetWindowRect(hWnd, &Rect);
			MoveWindow(
				hWnd,
				Rect.left,
				Rect.top,
				(CELL_SIZE + 1.666) * max_x,
				TOP_HEIGHT + (CELL_SIZE + 1.666) * max_y + MENU_HEIGHT,
				TRUE);
		case IDM_NEW:
			NewGame();
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case IDM_BESTRESULTS:
			DialogBox(hInst, (LPCTSTR)IDD_BESTRESULTSBOX, hWnd, (DLGPROC)BestResults);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
		// ��������� ��������� ��� ��������� ����
	case WM_PAINT:
		BeginPaint(hWnd, &ps);

		DrawTop();
		for (l.posx = 0; l.posx < max_x; l.posx++) {
			for (l.posy = 0; l.posy < max_y; l.posy++) {
				l.DrawState();
			}
		}
		EndPaint(hWnd, &ps);
		break;
		// ��������� ��������� ��� ����������� ����
	case WM_DESTROY:
		DeleteObject(bmp_0);
		DeleteObject(bmp_prestand);
		DeleteObject(bmp_appear[0]);
		DeleteObject(bmp_appear[1]);
		DeleteObject(bmp_appear[2]);
		DeleteObject(bmp_appear[3]);
		DeleteObject(bmp_stand);
		DeleteObject(bmp_jump[0]);
		DeleteObject(bmp_jump[1]);
		DeleteObject(bmp_jump[2]);
		DeleteObject(bmp_jump[3]);
		DeleteObject(bmp_jump[4]);
		DeleteObject(bmp_jump[5]);
		DeleteObject(bmp_jump[6]);
		DeleteObject(bmp_jump[7]);
		DeleteObject(bmp_jump[8]);
		DeleteObject(bmp_jump[9]);
		DeleteObject(bmp_jump[10]);
		DeleteObject(bmp_jump[11]);
		DeleteObject(bmp_explode[0]);
		DeleteObject(bmp_explode[1]);
		DeleteObject(bmp_explode[2]);
		DeleteObject(bmp_explode[3]);
		DeleteObject(bmp_explode[4]);
		DeleteObject(bmp_explode[5]);
		DeleteObject(bmp_explode[6]);
		DeleteObject(bmp_explode[7]);
		DeleteObject(bmp_explode[8]);
		DeleteObject(bmp_numbers);
		DeleteObject(bmp_points);

		DeleteDC(hCompatibleDC);
		ReleaseDC(hWnd, hDC);

		WriteInfo();

		PostQuitMessage(0);
		break;
		// ��������� ��������� �� ���������
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
// ������� ������� ���� ������� About
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
// ������� ������� ���� ������� Custom
LRESULT CALLBACK Custom(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static char szVal[10];

	switch (message)
	{
	case WM_INITDIALOG:
		std::ostrstream(szVal, sizeof(szVal)) << max_x << std::ends;
		SendDlgItemMessage(hDlg, IDC_EDIT1, WM_SETTEXT, (WPARAM)0, (LPARAM)szVal);
		std::ostrstream(szVal, sizeof(szVal)) << max_y << std::ends;
		SendDlgItemMessage(hDlg, IDC_EDIT2, WM_SETTEXT, (WPARAM)0, (LPARAM)szVal);
		std::ostrstream(szVal, sizeof(szVal)) << app_balls << std::ends;
		SendDlgItemMessage(hDlg, IDC_EDIT3, WM_SETTEXT, (WPARAM)0, (LPARAM)szVal);
		std::ostrstream(szVal, sizeof(szVal)) << del_balls << std::ends;
		SendDlgItemMessage(hDlg, IDC_EDIT4, WM_SETTEXT, (WPARAM)0, (LPARAM)szVal);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			SendDlgItemMessage(hDlg, IDC_EDIT1, WM_GETTEXT, (WPARAM)sizeof(szVal), (LPARAM)szVal);
			std::istrstream(szVal, sizeof(szVal)) >> max_x;
			SendDlgItemMessage(hDlg, IDC_EDIT2, WM_GETTEXT, (WPARAM)sizeof(szVal), (LPARAM)szVal);
			std::istrstream(szVal, sizeof(szVal)) >> max_y;
			SendDlgItemMessage(hDlg, IDC_EDIT3, WM_GETTEXT, (WPARAM)sizeof(szVal), (LPARAM)szVal);
			std::istrstream(szVal, sizeof(szVal)) >> app_balls;
			SendDlgItemMessage(hDlg, IDC_EDIT4, WM_GETTEXT, (WPARAM)sizeof(szVal), (LPARAM)szVal);
			std::istrstream(szVal, sizeof(szVal)) >> del_balls;
			EndDialog(hDlg, LOWORD(wParam));
		}
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
// ������� ������� ���� ������� BestResult
LRESULT CALLBACK BestResults(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static char szVal[50];
	static int i, h, m1, m2, s1, s2;
	switch (message) {
	case WM_INITDIALOG:
		for (i = 0; i < 3; i++) {
			h = leaders[i].time;
			s2 = h % 60; h /= 60;
			s1 = s2 % 10; s2 /= 10;
			m2 = h % 60; h /= 60;
			m1 = m2 % 10; m2 /= 10;
			std::ostrstream(szVal, sizeof(szVal)) << leaders[i].name << std::ends;;
			SendDlgItemMessage(hDlg, IDC_EDIT1 + i, WM_SETTEXT, (WPARAM)0, (LPARAM)szVal);
			std::ostrstream(szVal, sizeof(szVal)) << leaders[i].score << "   " << h << ':' << m2 << m1 << ':' << s2 << s1 << std::ends;
			SendDlgItemMessage(hDlg, IDC_EDIT4 + i, WM_SETTEXT, (WPARAM)0, (LPARAM)szVal);
		}
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
// ������� ������� ���� ������� GetName
LRESULT CALLBACK GetName(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static char szVal[30];
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			SendDlgItemMessage(hDlg, IDC_EDIT1, WM_GETTEXT, (WPARAM)sizeof(szVal), (LPARAM)szVal);
			std::ostrstream(leaders[gametype].name, sizeof(leaders[gametype].name)) << szVal << std::ends;
		}
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
// ������� "���������� �����"
void ALines(int e) {
	int y_old = y_lines;
	switch (y_lines) {
		//����� �������� ����
	case 0:
		if (e == 0 && xk1()) {
			z0();
			y_lines = 1;
		}
		break;
		//����� ���� ������� ������� ��� 
	case 1:
		if (e == 0 && xk0() && x0()) {
			z1_2();
			z1_1();
			y_lines = 2;
		}
		else {
			if (e == 0 && xk1()) {
				z1_2();
				z0();
				z1_3();
			}
			else if (e == 0 && xk2()) {
				z1_2();
				y_lines = 0;
			}
			else {
				if (e == 1) {
					z1_3();
				}
			}
		}
		break;
		//������������ �������� ����
	case 2:
		if (e == 1 && x1() && x2()) {
			z2_2();
			z2_3();
			y_lines = 3;
		}
		else if (e == 1 && x1()) {
			z2_2();
			z2_4();
			y_lines = 4;
		}
		else if (e == 1) {
			z2_1();
		}
		break;
		//�������� �����
	case 3:
		if (e == 1 && x3()) {
			z3_2();
			y_lines = 0;
		}
		else if (e == 1) {
			z3_1();
		}
		break;
		//��������� ����� �����
	case 4:
		if (e == 1 && x4() && x5()) {
			z4_2();
			z2_3();
			y_lines = 3;
		}
		else if (e == 1 && x4()) {
			z4_2();
			y_lines = 0;
		}
		else if (e == 1) {
			z4_1();
		}
		break;
	}
	if (y_old != y_lines) {
		switch (y_lines) {
		case 1:
			z1_3();
			break;
		case 2:
			z2_1();
			break;
		case 3:
			z3_1();
			break;
		case 4:
			z4_1();
			break;
		}
	}
}
//������� ����������
//��������� ������ �����
bool xk0() {
	return (click_ball.State() == 0 || click_ball.State() == 1);
}
//� ��������� ������ ���������� ���
bool xk1() {
	return (click_ball.State() == 3);
}
//� ��������� ������ ���������� ��������� ���
bool xk2() {
	return (click_ball.State() == 4);
}
//���������� ���� �� �������� ������ �� ���������
bool x0() {
	return FindPath(ball, click_ball);
}
//������������ �����������
bool x1() {
	return path.empty();
}
//��������� ������� ����� (����� ������������)
bool x2() {
	return CheckLines(ball);
}
//�������� �����������
bool x3() {
	itr = explode_list.begin();
	return (itr->State() == 0);//N3
}
//��������� �����������
bool x4() {
	itr = appear_list.begin();
	return (itr->State() == 3);//N1
}
//��������� ������� ����� (����� ��������� �����)
bool x5() {
	itr = appear_list.begin();
	while (itr != appear_list.end()) {
		CheckLines(*itr++);
	}
	return (explode_list.size() != 0);
}

// �������� �����������

//��������� ������
void z0()
{
	SetTimer(hWnd, 1, 50, NULL);
	PlaySound(MY_SOUND0, NULL, SND_FILENAME | SND_ASYNC);
	ball = click_ball;
}
//��������� ������������
void z1_1() {
	SetTimer(hWnd, 1, 50, NULL);
	ball_color = ball.Color();
}
//��������� ������
void z1_2() {
	KillTimer(hWnd, 1);
	ball.ACell(6);
}
//�������
void z1_3() {
	ball.ACell(5);
}
//����������� ��� �� ��������� ������
void z2_1() {
	ball.ACell(0);
	ball = path.top();
	path.pop();
	ball.ACell(1);
}
//��������� ������������
void z2_2() {
	KillTimer(hWnd, 1);
}
//���������  ��������
void z2_3() {
	SetTimer(hWnd, 1, 20, NULL);
	PlaySound(MY_SOUND1, NULL, SND_FILENAME | SND_ASYNC);
}
//��������� ���������
void z2_4() {
	CheckAppearList();
	SetTimer(hWnd, 1, 50, NULL);
}
//�������
void z3_1() {
	itr = explode_list.begin();
	while (itr != explode_list.end()) {
		(*itr++).ACell(4);
	}
}
//��������� ��������
void z3_2() {
	KillTimer(hWnd, 1);
	gamescore += (explode_list.size() - del_balls + 1) * explode_list.size();
	DrawScore();
	explode_list.clear();
}
//���������
void z4_1() {
	PlaySound(MY_SOUND2, NULL, SND_FILENAME | SND_ASYNC);
	itr = appear_list.begin();
	while (itr != appear_list.end()) {
		(*itr++).ACell(3);
	}
}
//��������� ���������
void z4_2() {
	KillTimer(hWnd, 1);
	GenerateAppearList();
}
//�������� ��� ����� ������ in �������� ���������� �������� �����(��� ��������� �����)
bool CheckLines(const cell& in) {
	int x = in.posx;
	int y = in.posy;
	int c = in.Color();
	int i, j;
	cell l;
	bool b = false;
	i = 1;
	while ((x + i < max_x)
		&& (map[x + i][y].y == 3)
		&& (map[x + i][y].color == c)) {
		++i;
	}
	j = 1;
	while ((x - j >= 0)
		&& (map[x - j][y].y == 3)
		&& (map[x - j][y].color == c)) {
		++j;
	}
	if (j + i - 1 >= del_balls) {
		l.posx = x + i;
		l.posy = y;
		for (int k = 0; k < i + j - 1; k++) {
			l.posx--;
			explode_list.push_back(l);
		}
		b = true;
	}
	i = 1;
	while ((y + i < max_y)
		&& (map[x][y + i].y == 3)
		&& (map[x][y + i].color == c)) {
		++i;
	}
	j = 1;
	while ((y - j >= 0)
		&& (map[x][y - j].y == 3)
		&& (map[x][y - j].color == c)) {
		++j;
	}
	if (j + i - 1 >= del_balls) {
		l.posx = x;
		l.posy = y + i;
		for (int k = 0; k < i + j - 1; k++) {
			l.posy--;
			explode_list.push_back(l);
		}
		b = true;
	}
	i = 1;
	while ((x + i < max_x)
		&& (y + i < max_y)
		&& (map[x + i][y + i].y == 3)
		&& (map[x + i][y + i].color == c)) {
		++i;
	}
	j = 1;
	while ((x - j >= 0)
		&& (y - j >= 0)
		&& (map[x - j][y - j].y == 3)
		&& (map[x - j][y - j].color == c)) {
		++j;
	}
	if (j + i - 1 >= del_balls) {
		l.posx = x + i;
		l.posy = y + i;
		for (int k = 0; k < i + j - 1; k++) {
			l.posx--;
			l.posy--;
			explode_list.push_back(l);
		}
		b = true;
	}
	i = 1;
	while ((x + i < max_x)
		&& (y - i >= 0)
		&& (map[x + i][y - i].y == 3)
		&& (map[x + i][y - i].color == c)) {
		++i;
	}
	j = 1;
	while ((x - j >= 0)
		&& (y + j < max_y)
		&& (map[x - j][y + j].y == 3)
		&& (map[x - j][y + j].color == c)) {
		++j;
	}
	if (j + i - 1 >= del_balls) {
		l.posx = x + i;
		l.posy = y - i;
		for (int k = 0; k < i + j - 1; k++) {
			l.posx--;
			l.posy++;
			explode_list.push_back(l);
		}
		b = true;
	}
	if (b) {
		explode_list.remove(in);
		explode_list.push_back(in);
	}
	return b;
}
//����� ���� ���������� ������ �� ������ from � in
bool FindPath(const cell& from, const cell& in) {
	struct {
		cell pred;
		int mark;
	}
	v[MAX_MAP_X][MAX_MAP_Y];

	cell k, l;

	std::queue<cell> q;

	for (int i = 0; i < max_x; i++)
		for (int j = 0; j < max_y; j++)
			v[i][j].mark = 0;

	v[from.posx][from.posy].mark = 1;
	q.push(from);

	while (!q.empty()) {
		k = q.front();
		for (int i = 0; i < 4; i++) {
			l = k;
			switch (i)
			{
			case 0:
				l.posx--;
				break;
			case 1:
				l.posx++;
				break;
			case 2:
				l.posy--;
				break;
			case 3:
				l.posy++;
				break;
			}
			if (Valid(l) && !v[l.posx][l.posy].mark) {
				v[l.posx][l.posy].mark = 1;
				v[l.posx][l.posy].pred = k;
				q.push(l);

				if (l == in) {
					do {
						path.push(l);
						l = v[l.posx][l.posy].pred;

					} while (l != from);
					return true;
				}
			}
		}

		q.pop();

	}

	return false;
}
// ��������� ������ ������������ ����� �� �������� ��������� ��������������� ������
void CheckAppearList() {
	int tmp;
	itr = appear_list.begin();
	while (itr != appear_list.end()) {
		if (itr->State() == 3) {
			tmp = itr->PreColor();
			itr->PreColor() = -1;
			FindEmptyCell(*itr);
			itr->PreColor() = tmp;
			(*itr).ACell(2);
		}
		++itr;
	}
}
//������� ������ ������������ �����
void GenerateAppearList() {
	appear_list.clear();
	cell l;
	for (int i = 0; i < app_balls; i++) {
		if (FindEmptyCell(l)) {
			appear_list.push_back(l);
			l.ACell(2);
		}
		else return;
	}
}
//����� ������ ������
bool FindEmptyCell(cell& in) {
	cell l;
	l.posx = random(max_x);
	l.posy = random(max_y);

	if (l.State() == 0) {
		in = l;
		return true;
	};

	for (int i = 0; i < max_x * max_y; i++) {
		if (l.posx != max_x - 1) {
			l.posx++;
		}
		else if (l.posy != max_y - 1) {
			l.posy++;
			l.posx = 0;
		}
		else {
			l.posx = 0;
			l.posy = 0;
		};
		if (l.State() == 0) {
			in = l;
			return true;
		}
	}

	GameOver();
	return false;
}
//��������� ����� �� ����� ������ in �������� ����
bool Valid(const cell& in)
{
	return (in.posx >= 0) && (in.posx < max_x) &&
		(in.posy >= 0) && (in.posy < max_y) &&
		(in.State() == 0 || in.State() == 1);
}
//���������� �������� ��� ����� ���� 
void NewGame()
{
	for (int i = 0; i < max_x; i++) {
		for (int j = 0; j < max_y; j++) {
			map[i][j].y = 0;
			map[i][j].color = 0;
			map[i][j].pre_color = -1;
			map[i][j].num_pic = 0;
		}
	}

	y_lines = 0;

	gamescore = 0;
	gametime = 0;

	SetTimer(hWnd, 0, 1000, NULL);

	cell l;

	for (int i = 0; i < del_balls; i++) {
		FindEmptyCell(l);
		ball_color = random(7);
		l.ACell(1);
	}
	GenerateAppearList();
}
//��������� ��������� �������� ��������
void GameOver() {
	KillTimer(hWnd, 0);
	PlaySound(MY_SOUND3, NULL, SND_FILENAME | SND_ASYNC);

	if (gametype < 3) {
		if (leaders[gametype].score < gamescore
			|| (leaders[gametype].score == gamescore && leaders[gametype].time > gametime)) {
			DialogBox(hInst, (LPCTSTR)IDD_GETNAMEBOX, hWnd, (DLGPROC)GetName);
			leaders[gametype].score = gamescore;
			leaders[gametype].time = gametime;
		}
		else {
			DialogBox(hInst, (LPCTSTR)IDD_BESTRESULTSBOX, hWnd, (DLGPROC)BestResults);
		}
	}
	else {
		MessageBox(hWnd, "Your custom game is over...", "Condolences", MB_OK);
	}
	NewGame();
	InvalidateRect(hWnd, NULL, FALSE);
}
//���������� �� ����� ����������������� ����
void DrawTime()
{
	SelectObject(hCompatibleDC, bmp_numbers);
	int h = gametime;
	int s2 = h % 60; h /= 60;
	int s1 = s2 % 10; s2 /= 10;
	int m2 = h % 60; h /= 60;
	int m1 = m2 % 10; m2 /= 10;
	BitBlt(hDC, max_x * CELL_SIZE - 29, 5, 20, 37, hCompatibleDC, s1 * 19, 0, SRCCOPY);
	BitBlt(hDC, max_x * CELL_SIZE - 50, 5, 20, 37, hCompatibleDC, s2 * 19, 0, SRCCOPY);
	BitBlt(hDC, max_x * CELL_SIZE - 77, 5, 20, 37, hCompatibleDC, m1 * 19, 0, SRCCOPY);
	BitBlt(hDC, max_x * CELL_SIZE - 98, 5, 20, 37, hCompatibleDC, m2 * 19, 0, SRCCOPY);
	BitBlt(hDC, max_x * CELL_SIZE - 125, 5, 20, 37, hCompatibleDC, h * 19, 0, SRCCOPY);
	SelectObject(hCompatibleDC, bmp_points);
	BitBlt(hDC, max_x * CELL_SIZE - 56, 5, 5, 37, hCompatibleDC, 0, 0, SRCCOPY);
	BitBlt(hDC, max_x * CELL_SIZE - 104, 5, 5, 37, hCompatibleDC, 0, 0, SRCCOPY);
}
//���������� �� ����� ������� ����
void DrawScore()
{
	SelectObject(hCompatibleDC, bmp_numbers);
	int t = gamescore;
	for (int i = 0; i < 5; i++) {
		BitBlt(hDC, 100 - 21 * i, 5, 20, 37, hCompatibleDC, (t % 10) * 19, 0, SRCCOPY);
		t /= 10;
	}
}
//���������� �����
void DrawTop()
{
	PatBlt(hDC, 0, 0, CELL_SIZE * max_x, TOP_HEIGHT, BLACKNESS);
	DrawScore();
	DrawTime();
}
//��������� �������� ��������� ����
void CheckCustomParameters()
{
	if (max_x < 6) max_x = 6;
	if (max_x > 20) max_x = 20;
	if (max_y < 2) max_y = 2;
	if (max_y > 12) max_y = 12;

	if (del_balls < 2) del_balls = 2;

	if (del_balls > (max_x > max_y ? max_x : max_y)) {
		del_balls = (max_x > max_y ? max_x : max_y);
	}

	if (app_balls + del_balls > max_x * max_y)
	{
		app_balls = max_x * max_y - del_balls;
	}
}
//��������� ���������� � ������ �������
void GetInfo()
{
	FILE* in;
	if (in = fopen("leaders.dat", "rb")) {
		fread(&gametype, sizeof(int), 1, in);
		fread(&max_x, sizeof(int), 1, in);
		fread(&max_y, sizeof(int), 1, in);
		fread(&app_balls, sizeof(int), 1, in);
		fread(&del_balls, sizeof(int), 1, in);
		for (int i = 0; i < 3; i++) {
			fread(&leaders[i], sizeof(info), 1, in);
		}
		fclose(in);
	}
	else {
		gametype = 0;
		max_x = 9; max_y = 9;
		app_balls = 3; del_balls = 5;
		for (int i = 0; i < 3; i++) {
			leaders[i] = null_leader;
		}
	}
}
//��������� ���������� � ������ �������
void WriteInfo()
{
	FILE* out;
	if (out = fopen("leaders.dat", "wb")) {
		fwrite(&gametype, sizeof(int), 1, out);
		fwrite(&max_x, sizeof(int), 1, out);
		fwrite(&max_y, sizeof(int), 1, out);
		fwrite(&app_balls, sizeof(int), 1, out);
		fwrite(&del_balls, sizeof(int), 1, out);
		for (int i = 0; i < 3; i++) {
			fwrite(&leaders[i], sizeof(info), 1, out);
		}
		fclose(out);
	}
}
