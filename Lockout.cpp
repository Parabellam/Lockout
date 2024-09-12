#include "framework.h"
#include "Lockout.h"
#include <windows.h>
#include <iostream>
#include <string>

// By Stiven Ruiz (Parabellam)

#define MAX_LOADSTRING 100

// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal

// Tamaño del botón
const int BUTTON_WIDTH = 150;
const int BUTTON_HEIGHT = 30;
const int WINDOW_MARGIN = 20; // Márgenes alrededor del botón

// Prototipos de funciones
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
void StartLock();
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

// Secuencias de desbloqueo
const std::string UNLOCK_SEQUENCE1 = "/*-+";
const std::string UNLOCK_SEQUENCE2 = "xxx6b3cxc7yhbuxxx";
std::string typedSequence = "";

// Declaración de los hooks de teclado y mouse
HHOOK hKeyboardHook;
HHOOK hMouseHook;

int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LOCKOUT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LOCKOUT));

    MSG msg;

    // Bucle principal de mensajes:
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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOCKOUT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LOCKOUT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Almacenar identificador de instancia en una variable global

    // Calcula el tamaño de la ventana basado en el tamaño del botón y márgenes
    int windowWidth = BUTTON_WIDTH * 1.2 + (2 * WINDOW_MARGIN);
    int windowHeight = BUTTON_HEIGHT * 2.5 + (2 * WINDOW_MARGIN);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, 0, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Iniciar el bloqueo al abrir la aplicación
    StartLock();

    return TRUE;
}

void StartLock() {
    // Limpiar la secuencia digitada al bloquear nuevamente
    typedSequence.clear();

    // Ocultar el cursor
    ShowCursor(FALSE);

    // Instalar el hook de teclado
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (hKeyboardHook == NULL) {
        std::cerr << "Error al instalar el hook de teclado!" << std::endl;
        return;
    }

    // Instalar el hook de mouse
    hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
    if (hMouseHook == NULL) {
        std::cerr << "Error al instalar el hook de mouse!" << std::endl;
        UnhookWindowsHookEx(hKeyboardHook);  // Desinstalar el hook de teclado si el de mouse falla
        return;
    }

    std::cout << "Bloqueo activado." << std::endl;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            char key = (char)pKeyboard->vkCode;

            if (pKeyboard->vkCode == VK_DIVIDE) {
                typedSequence += '/';
            }
            else if (pKeyboard->vkCode == VK_MULTIPLY) {
                typedSequence += '*';
            }
            else if (pKeyboard->vkCode == VK_SUBTRACT) {
                typedSequence += '-';
            }
            else if (pKeyboard->vkCode == VK_ADD) {
                typedSequence += '+';
            }
            else if (key >= 65 && key <= 90) {
                key += 32;
                typedSequence += key;
            }
            else if ((key >= 97 && key <= 122) || (key >= 48 && key <= 57)) {
                typedSequence += key;
            }

            if (typedSequence.find(UNLOCK_SEQUENCE1) != std::string::npos || typedSequence.find(UNLOCK_SEQUENCE2) != std::string::npos) {
                std::cout << "Desbloqueado." << std::endl;
                UnhookWindowsHookEx(hKeyboardHook);
                UnhookWindowsHookEx(hMouseHook);
                ShowCursor(TRUE);
                return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
            }

            return 1;
        }
    }

    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        return 1;
    }

    return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        // Crear botón para bloquear
        CreateWindow(TEXT("BUTTON"), TEXT("Bloquear nuevamente"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            WINDOW_MARGIN, WINDOW_MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT, hWnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case 1: // ID del botón
            StartLock();
            break;
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