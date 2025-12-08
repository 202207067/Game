// Game.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Game.h"
#include <time.h>

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst; // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING]; // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING]; // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GAME, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 애플리케이션 초기화를 수행합니다:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAME));

	MSG msg;

	// 기본 메시지 루프입니다:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
// 함수: MyRegisterClass()
//
// 용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GAME);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
// 함수: InitInstance(HINSTANCE, int)
//
// 용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
// 주석:
//
// 이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
// 주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
// 함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
// 용도: 주 창의 메시지를 처리합니다.
//
// WM_COMMAND - 애플리케이션 메뉴를 처리합니다.
// WM_PAINT - 주 창을 그립니다.
// WM_DESTROY - 종료 메시지를 게시하고 반환합니다.
//
//
int g_stage = 1; /// 현재 스테이지 번호
int g_life = 5; /// 남은 목숨 수
int g_time = 15; /// 제한 시간 (초 단위라고 가정)
bool g_hard = false; /// 난이도 (false = 쉬움, true = 어려움)
bool g_started = false; /// 게임이 시작되었는지 여부 (false면 난이도 선택 단계)

#define MAX_LEN 200		/// 최대 200개 방향키 시퀀스 저장 가능
int g_pattern[MAX_LEN]; /// 정답 방향키 시퀀스를 저장하는 배열
int g_index = 0; /// 플레이어가 지금 몇 번째 화살표까지 맞췄는지
int g_len = 5; /// 이번 스테이지의 패턴 길이

bool g_showPattern = true; /// 패턴을 화면에 보여줄지 여부
int g_showTimer = 3; /// 패턴을 몇 초 동안 보여줄지 (어려움 모드에서 씀)

bool g_hintActive = false;      /// 힌트가 켜져 있는지
int  g_hintTimer = 0;           /// 힌트를 몇 초 동안 보여줄지 (2초)
int g_hintCount = 3;			/// 사용할 수 있는 힌트 총 3개

#define TIMER_GAME 1			/// 제한시간 감소용 타이머 ID
#define TIMER_HIDE 2			/// “패턴 숨기기”용 타이머 ID
#define TIMER_HINT 3            /// 힌트 타이머 ID




HWND g_hwnd;

void MakePattern() /// 정답 패턴 만드는 함수
{
	for (int i = 0; i < g_len; i++) { /// g_len = 5;
		g_pattern[i] = rand() % 4; /// 0↑ 1↓ 2← 3→ 현재 스테이지에서 사용할 정답 방향키 시퀀스를 랜덤으로 만들어서 g_pattern 배열에 저장.
	}
	g_index = 0; /// 플레이어가 아직 아무 것도 입력 안 했으니까 g_index를 0으로 초기화.
	/// 지금까지 맞춘 개수를 0으로 리셋.

}


void DrawPattern(HDC hdc) /// 화면에 화살표 그리는 함수
{
	if (!g_started) return;             /// 아직 게임 시작 전이면 패턴 안 그림

	if (!g_showPattern && g_hard) return; /// 어려움 모드에서 가려야 하면 안 그림

	int x = 200, y = 300; /// 화살표들을 찍기 시작할 기준 좌표.

	for (int i = 0; i < g_len; i++) /// 이번 스테이지 패턴 길이만큼 반복 ( 초기값 5 )
	{
		if (i < g_index) {
			TextOut(hdc, x, y, L" ", 1); /// 이미 입력한 화살표는 공백으로 처리 ( 내가 어디까지 입력했는지 보여주기 위해 )
		}
		else {

			// 아직 입력해야 할 화살표
			switch (g_pattern[i])
			{
			case 0: TextOut(hdc, x, y, L"↑", 1); break;
			case 1: TextOut(hdc, x, y, L"↓", 1); break;
			case 2: TextOut(hdc, x, y, L"←", 1); break;
			case 3: TextOut(hdc, x, y, L"→", 1); break;
				/// 1. 디바이스 컨텍스트(DC) 핸들
				/// 2. 문자열을 시작할 x 좌표
				/// 3. 문자열을 시작할 y 좌표
				/// 4. 출력할 문자열
				/// 5. 출력할 문자열의 길이 (문자 개수)
			}
		}


		x += 30; /// 화살표들을 찍기 시작할 기준 좌표.


	}
}

void StartGame() /// 게임 시작용 함수 만들기 (난이도 결정 후 호출)
{
	g_stage = 1;
	g_life = 5;
	g_time = 15;
	g_len = 5;
	g_index = 0;

	g_showPattern = true;
	g_showTimer = 3;
	g_hintCount = 3;     /// 사용할 수 있는 힌트 총 3개

	MakePattern();  /// 첫 패턴 생성

	/// 타이머 시작 (제한시간)
	SetTimer(g_hwnd, TIMER_GAME, 1000, NULL);

	/// 어려움 모드면 '패턴 숨기기' 타이머도 같이 시작
	if (g_hard) {
		KillTimer(g_hwnd, TIMER_HIDE);			 ///  반드시 기존 타이머 제거
		g_showTimer = 3;						/// 다음 스테이지 표현 시간 초기화
		SetTimer(g_hwnd, TIMER_HIDE, 1000, NULL);
	}


	g_started = true;   /// 이제부터는 게임 진행 상태
}

void ResetGame(HWND hWnd)
{
	g_started = false;
	g_hard = false;

	g_index = 0;
	g_life = 5;
	g_time = 15;

	// 화면 지우기
	InvalidateRect(hWnd, NULL, TRUE);
}


void GameOver(HWND hWnd, const wchar_t* reason)
{
	// 모든 타이머 중단
	KillTimer(hWnd, TIMER_GAME);
	KillTimer(hWnd, TIMER_HIDE);

	wchar_t msg[200];
	wsprintf(msg, L"%s\n다시 해볼텨 ?", reason);

	int res = MessageBox(hWnd, msg, L"GAME OVER", MB_YESNO | MB_ICONQUESTION);


	if (res == IDYES) {
		ResetGame(hWnd);   // ← 재시작 처리
	}
	else {
		DestroyWindow(hWnd);   // ← 프로그램 종료
	}
}

void KeyCheck(int key) /// 방향키 입력을 정답과 비교하는 함수
{
	int input = -1; /// 아직 방향키로 변환하기 전 상태를 -1로 둔다.

	if (key == VK_UP) input = 0;
	if (key == VK_DOWN) input = 1;
	if (key == VK_LEFT) input = 2;
	if (key == VK_RIGHT) input = 3;

	if (input == -1) return; /// 방향키가 아니면 함수 종료..

	/// 오답
	if (input != g_pattern[g_index]) {
		g_life--;		/// 목숨 하나 감소
		if (g_life <= 0) {
			GameOver(g_hwnd, L"목숨을 모두 잃었소...!");
			return;
		}
		return;
	}

	/// 정답
	g_index++;

	/// 패턴 끝까지 맞춘 경우
	if (g_index == g_len) { /// 사용자의 입력 길이와 스테이지의 패턴 길이 비교식, 이번 스테이지 패턴을 전부 성공적으로 입력.
		g_stage++; /// 스테이지 번호 증가
		g_len++; /// 다음 스테이지는 한 개 더 길게
		g_time = 15; /// 제한 시간 초기화
		g_showPattern = true; /// 패턴 다시 보여주기
		

		g_hintActive = false;	/// 힌트 중이었으면 강제 종료
		KillTimer(g_hwnd, TIMER_HINT);
		g_hintTimer = 0;
		MakePattern(); /// 새로운 패턴 다시 생성

		if (g_hard) {
			KillTimer(g_hwnd, TIMER_HIDE);			 ///  반드시 기존 타이머 제거
			g_showTimer = 3;						 /// 다음 스테이지 패턴 보여주는 시간 초기화
			SetTimer(g_hwnd, TIMER_HIDE, 1000, NULL); /// 어려움 모드면 '패턴 숨기기' 타이머도 다시 시작
			/// 1초마다 WM_TIMER / TIMER_HIDE 발생
			/// g_showTimer 가 0이 될 때까지 1초씩 감소하다가 0이 되면 패턴 숨김

		}
	}
}


void UseHint()
{
	if (!g_hard) return;           /// 힌트는 어려움 모드에서만
	if (!g_started) return;        /// 게임 시작 전이면 무시
	if (g_showPattern) return;     /// 이미 패턴을 보여주는 중이면 힌트 불필요
	if (g_hintCount <= 0) return; /// 힌트 다 썼으면 종료

	g_hintCount--;               /// 힌트 1개 소모


	g_hintActive = true;
	g_showPattern = true;          /// 패턴을 다시 보여줌
	g_hintTimer = 2;               /// 2초 동안 표시

	/// 힌트 타이머 시작
	SetTimer(g_hwnd, TIMER_HINT, 1000, NULL);

	InvalidateRect(g_hwnd, NULL, TRUE);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		g_hwnd = hWnd;                      /// 전역 HWND 저장
		srand((unsigned)time(NULL));        /// 랜덤 시드

		/// 바로 게임 시작하지 않고,
		/// E/H 키 입력으로 난이도 선택할 때까지 대기
	}
	break;
	case WM_TIMER:
	{
		if (!g_started)
			break;

		/// 제한시간 감소용 타이머
		if (wParam == TIMER_GAME) {
			g_time--;
			if (g_time <= 0) {
				GameOver(hWnd, L"시간 초과!");
			}
		}

		/// 어려움 모드에서만 작동
		if (wParam == TIMER_HIDE && g_hard) {
			g_showTimer--;
			if (g_showTimer <= 0) {
				g_showPattern = false; /// 패턴 숨기기
				KillTimer(hWnd, TIMER_HIDE);
			}
		}

		if (wParam == TIMER_HINT && g_hintActive && g_hard)
		{
			g_hintTimer--;

			if (g_hintTimer <= 0)
			{
				g_hintActive = false;
				g_showPattern = false; /// 다시 숨기기
				KillTimer(hWnd, TIMER_HINT);
			}
		}

		InvalidateRect(hWnd, NULL, TRUE);
	}
	return 0;

	case WM_KEYDOWN:
	{
		/// 아직 게임 시작 전이면 → 난이도 선택 단계
		if (!g_started)
		{
			if (wParam == 'E' || wParam == 'e')
			{
				g_hard = false;   /// 쉬움
				StartGame();
			}
			else if (wParam == 'H' || wParam == 'h')
			{
				g_hard = true;    /// 어려움
				StartGame();
			}


			InvalidateRect(hWnd, NULL, TRUE);
			break;
		}

		if (wParam == 'T' || wParam == 't')
		{
			UseHint();
			break;
		}

		/// 이미 게임이 시작된 상태라면 → 방향키 입력 처리
		KeyCheck((int)wParam);
		InvalidateRect(hWnd, NULL, TRUE);
	}
	break;


	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...


		if (!g_started)
		{
			/// 아직 게임 시작 전 
			TextOut(hdc, 100, 50, L"방향키 누르기 게임", lstrlenW(L"방향키 누르기 게임"));
			TextOut(hdc, 100, 80, L"------------------------------------------------------------------------------------------------------------------------------------------------------",
				lstrlenW(L"------------------------------------------------------------------------------------------------------------------------------------------------------"));
			TextOut(hdc, 100, 110, L"★ 스테이지별로 화면에 출력된 방향키를 순서대로 누르면 스테이지 클리어", lstrlenW(L"★ 스테이지별로 화면에 출력된 방향키를 순서대로 누르면 스테이지 클리어"));
			TextOut(hdc, 100, 140, L"★ 한 스테이지 당 제한시간 15초, 제한시간 내 스테이지를 클리어하여 최다 스테이지를 기록하시지요",
				lstrlenW(L"★ 한 스테이지 당 제한시간 15초, 제한시간 내 스테이지를 클리어하여 최다 스테이지를 기록하시지요"));
			TextOut(hdc, 100, 170, L"★ 스테이지 넘어갈 때 마다 눌러야 할 방향키 수가 하나씩 증가합니다.", lstrlenW(L"★ 스테이지 넘어갈 때 마다 눌러야 할 방향키 수가 하나씩 증가합니다."));
			TextOut(hdc, 100, 200, L"★ 한번 틀릴 때마다 생명이 줄어듭니다.", lstrlenW(L"★ 한번 틀릴 때마다 생명이 줄어듭니다."));
			TextOut(hdc, 100, 230, L"★ 제한시간이 넘어가면 게임오버되며 게임종료 됩니다.", lstrlenW(L"★ 제한시간이 넘어가면 게임오버되며 게임종료 됩니다."));
			TextOut(hdc, 100, 260, L"★ 어려움 모드는 3초 이후 화면이 사라집니다 잘 기억해봐여", lstrlenW(L"★ 어려움 모드는 3초 이후 화면이 사라집니다 잘 기억해봐여"));
			TextOut(hdc, 100, 290, L"★ 어려워 할 당신을 위해 힌트도 만들었는데 T 누르면 2초간 보여주니까 홧팅", lstrlenW(L"★ 어려워 할 당신을 위해 힌트도 만들었는데 T 누르면 2초간 보여주니까 홧팅"));
			TextOut(hdc, 180, 330, L"그럼 Good Luck ♣", lstrlenW(L"그럼 Good Luck ♣"));

			TextOut(hdc, 100, 360, L"------------------------------------------------------------------------------------------------------------------------------------------------------",
				lstrlenW(L"------------------------------------------------------------------------------------------------------------------------------------------------------"));

			TextOut(hdc, 100, 390, L"--- 난이도 선택 ---", lstrlenW(L"--- 난이도 선택 ---"));
			TextOut(hdc, 100, 420, L"[E] 쉬움  /  [H] 어려움", lstrlenW(L"[E] 쉬움  /  [H] 어려움"));
			TextOut(hdc, 100, 450, L"난이도를 고른 후 방향키로 화살표를 입력하세요.", lstrlenW(L"난이도를 고른 후 방향키로 화살표를 입력하세요."));
		}
		else
		{
			/// 게임 진행 중: STAGE / LIFE / TIME / 패턴 출력
			wchar_t text[100];

			wsprintf(text, L"STAGE : %d", g_stage);
			TextOut(hdc, 100, 80, text, lstrlen(text));

			wsprintf(text, L"LIFE : %d", g_life);
			TextOut(hdc, 100, 110, text, lstrlen(text));

			wsprintf(text, L"TIME : %d", g_time);
			TextOut(hdc, 100, 140, text, lstrlen(text));

			if (g_hard) {
				wsprintf(text, L"HINT : %d", g_hintCount);
				TextOut(hdc, 100, 170, text, lstrlen(text));
			}


			/// 패턴 출력
			DrawPattern(hdc);
		}

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}