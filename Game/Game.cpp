// Game.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Game.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
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

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
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
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//


int g_stage = 1;      /// 현재 스테이지 번호
int g_life = 3;       /// 남은 목숨 수
int g_time = 10;      /// 제한 시간 (초 단위라고 가정)
bool g_hard = false;  /// 난이도 (false = 쉬움, true = 어려움)

#define MAX_LEN 10
int g_pattern[MAX_LEN]; /// 정답 방향키 시퀀스를 저장하는 배열
int g_index = 0;        /// 플레이어가 지금 몇 번째 화살표까지 맞췄는지
int g_len = 5;          /// 이번 스테이지의 패턴 길이

bool g_showPattern = true; /// 패턴을 화면에 보여줄지 여부 (어려움 모드에서 씀)
int g_showTimer = 3;       /// 패턴을 몇 초 동안 보여줄지 (어려움 모드에서 씀)

#define TIMER_GAME  1   /// 제한시간 감소용 타이머 ID
#define TIMER_HIDE  2   /// “패턴 숨기기”용 타이머 ID

HWND g_hwnd;

void MakePattern()                  /// 정답 패턴 만드는 함수
{
    for (int i = 0; i < g_len; i++)  /// g_len = 5;
        g_pattern[i] = rand() % 4; /// 0↑ 1↓ 2← 3→ 현재 스테이지에서 사용할 정답 방향키 시퀀스를 랜덤으로 만들어서 g_pattern 배열에 저장.
        g_index = 0;               /// 플레이어가 아직 아무 것도 입력 안 했으니까 g_index를 0으로 초기화.
                                    /// 지금까지 맞춘 개수를 0으로 리셋.
}


void DrawPattern(HDC hdc)               /// 화면에 화살표 그리는 함수
{
    if (!g_showPattern && g_hard) return; /// 어려움 모드에서 가려야 하면 안 그림

    int x = 100, y = 100;                 /// 화살표들을 찍기 시작할 기준 좌표.

    for (int i = 0; i < g_len; i++)
    {
        switch (g_pattern[i])
        {
        case 0: TextOut(hdc, x, y, L"↑", 1); break;
        case 1: TextOut(hdc, x, y, L"↓", 1); break;
        case 2: TextOut(hdc, x, y, L"←", 1); break;
        case 3: TextOut(hdc, x, y, L"→", 1); break;
        }
        x += 30;                            /// 화살표들을 찍기 시작할 기준 좌표.
    }
}


void KeyCheck(int key)                /// 방향키 입력을 정답과 비교하는 함수
{
    int input = -1;                 /// 아직 방향키로 변환하기 전 상태를 -1로 둔다.

    if (key == VK_UP)    input = 0;
    if (key == VK_DOWN)  input = 1;
    if (key == VK_LEFT)  input = 2;
    if (key == VK_RIGHT) input = 3;

    if (input == -1) return;           /// 방향키가 아니면 함수 종료..

    /// 오답
    if (input != g_pattern[g_index]) {
        g_life--;
        if (g_life <= 0) {
            MessageBox(NULL, L"GAME OVER", L"END", MB_OK);
            exit(0);
        }
        return;
    }

    /// 정답
    g_index++;

    /// 패턴 끝까지 맞춘 경우
    if (g_index == g_len) {             /// 사용자의 입력 길이와 스테이지의 패턴 길이 비교식, 이번 스테이지 패턴을 전부 성공적으로 입력.
        g_stage++;          // 스테이지 번호 증가
        g_len++;            // 다음 스테이지는 한 개 더 길게
        g_time = 10;        // 제한 시간 초기화
        g_showPattern = true;
        g_showTimer = 3;
        MakePattern();      // 새로운 패턴 다시 생성
        SetTimer(g_hwnd, TIMER_HIDE, 1000, NULL); // 어려움 모드에서 1초마다 hide 타이머
    }
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
    {
        if (wParam == TIMER_GAME) {
            g_time--;
            if (g_time <= 0) {
                MessageBox(hWnd, L"시간초과! GAME OVER", L"END", MB_OK);
                exit(0);
            }
        }

        if (wParam == TIMER_HIDE && g_hard) {
            g_showTimer--;
            if (g_showTimer <= 0) {
                g_showPattern = false;
                KillTimer(hWnd, TIMER_HIDE);
            }
        }

        InvalidateRect(hWnd, NULL, TRUE);
    }
    return 0;

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

         // 스테이지, LIFE, TIME 표시
        wchar_t text[100];
        wsprintf(text, L"STAGE : %d", g_stage);
        TextOut(hdc, 50, 20, text, lstrlen(text));

        wsprintf(text, L"LIFE : %d", g_life);
        TextOut(hdc, 50, 50, text, lstrlen(text));

        wsprintf(text, L"TIME : %d", g_time);
        TextOut(hdc, 50, 80, text, lstrlen(text));

        // 패턴 출력
        DrawPattern(hdc);
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
