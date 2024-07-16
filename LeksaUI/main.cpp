#include <windows.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <string>
#include <thread>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "comctl32.lib")

#define ID_EDIT_USER 1001
#define ID_EDIT_PASS 1002
#define ID_BUTTON1 2001
#define ID_BUTTON2 2002
#define ID_CHECKBOX_REMEMBER 3001
#define ID_BUTTON_DECONNECT 4001
#define ID_BUTTON_EXIT 4002

HRGN CreateRoundedRectRgn(int width, int height, int radius) {
    return CreateRoundRectRgn(0, 0, width, height, radius, radius);
}

void DrawCloseButton(HDC hdc, RECT rect) {
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
    HGDIOBJ hOldPen = SelectObject(hdc, hPen);
    MoveToEx(hdc, rect.left + 2, rect.top + 2, nullptr);
    LineTo(hdc, rect.right - 2, rect.bottom - 2);
    MoveToEx(hdc, rect.right - 2, rect.top + 2, nullptr);
    LineTo(hdc, rect.left + 2, rect.bottom - 2);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

LRESULT CALLBACK ButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        DrawCloseButton(hdc, rect);
        EndPaint(hwnd, &ps);
    } return 0;

    case WM_LBUTTONDOWN: {
        SendMessage(GetParent(hwnd), WM_COMMAND, GetDlgCtrlID(hwnd), 0);
    } return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void DrawCustomButton(HDC hdc, RECT rect, LPCWSTR text, BOOL isPressed) {
    HBRUSH hBrush = CreateSolidBrush(RGB(50, 50, 50));
    if (isPressed) {
        hBrush = CreateSolidBrush(RGB(70, 70, 70));
    }
    HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(100, 100, 100));
    HGDIOBJ hOldPen = SelectObject(hdc, hPen);
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 20, 20);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

struct Placeholder {
    HWND hwnd;
    std::wstring text;
    bool showing;
};

Placeholder placeholders[2];

LRESULT CALLBACK EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    for (auto& ph : placeholders) {
        if (hwnd == ph.hwnd) {
            switch (uMsg) {
            case WM_SETFOCUS:
                if (ph.showing) {
                    SetWindowText(hwnd, L"");
                    ph.showing = false;
                }
                break;
            case WM_KILLFOCUS:
                if (GetWindowTextLength(hwnd) == 0) {
                    SetWindowText(hwnd, ph.text.c_str());
                    ph.showing = true;
                }
                break;
            case WM_PAINT: {
                LRESULT result = DefSubclassProc(hwnd, uMsg, wParam, lParam);
                if (ph.showing && GetWindowTextLength(hwnd) == 0) {
                    HDC hdc = GetDC(hwnd);
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    SetTextColor(hdc, RGB(169, 169, 169));
                    SetBkMode(hdc, TRANSPARENT);
                    DrawText(hdc, ph.text.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                    ReleaseDC(hwnd, hdc);
                }
                return result;
            }
            }
            break;
        }
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

void ShowLoginControls(HWND hwnd, BOOL show) {
    ShowWindow(GetDlgItem(hwnd, ID_EDIT_USER), show ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_EDIT_PASS), show ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 2), show ? SW_SHOW : SW_HIDE); // Login
    ShowWindow(GetDlgItem(hwnd, 3), show ? SW_SHOW : SW_HIDE); // Register
    ShowWindow(GetDlgItem(hwnd, 4), show ? SW_SHOW : SW_HIDE); // About Us
    ShowWindow(GetDlgItem(hwnd, 5), show ? SW_SHOW : SW_HIDE); // Exit
    ShowWindow(GetDlgItem(hwnd, ID_CHECKBOX_REMEMBER), show ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 3002), show ? SW_SHOW : SW_HIDE); // Remember Me text
}

void ShowMainControls(HWND hwnd, BOOL show) {
    ShowWindow(GetDlgItem(hwnd, ID_BUTTON1), show ? SW_SHOW : SW_HIDE); // Spoofer
    ShowWindow(GetDlgItem(hwnd, ID_BUTTON2), show ? SW_SHOW : SW_HIDE); // Cleaner
    ShowWindow(GetDlgItem(hwnd, ID_BUTTON_DECONNECT), show ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, ID_BUTTON_EXIT), show ? SW_SHOW : SW_HIDE);
}

void HandleLogin(HWND hwnd) {
    wchar_t userText[100], passText[100];
    GetWindowText(GetDlgItem(hwnd, ID_EDIT_USER), userText, 100);
    GetWindowText(GetDlgItem(hwnd, ID_EDIT_PASS), passText, 100);
    if (wcslen(userText) == 0 || wcslen(passText) == 0 || wcscmp(userText, L"Username") == 0 || wcscmp(passText, L"Password") == 0) {
        MessageBox(hwnd, L"Please enter both username and password.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Simulate a long login process
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Switch to main controls
    PostMessage(hwnd, WM_COMMAND, MAKELONG(1000, 0), 0);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hButtonClose;
    static HWND hEditUser;
    static HWND hEditPass;
    static HWND hButtonLogin;
    static HWND hButtonRegister;
    static HWND hButtonAbout;
    static HWND hButtonExit;
    static HWND hButton1;
    static HWND hButton2;
    static HWND hCheckboxRemember;
    static HWND hCheckboxText;

    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_CREATE: {
        RECT rect;
        GetClientRect(hwnd, &rect);

        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = ButtonProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wc.lpszClassName = L"CustomButton";
        RegisterClass(&wc);

        hButtonClose = CreateWindowEx(0, L"CustomButton", NULL, WS_VISIBLE | WS_CHILD, rect.right - 40, 10, 30, 30, hwnd, (HMENU)1, GetModuleHandle(NULL), NULL);

        hEditUser = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Username", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, rect.left + 100, rect.top + 100, 300, 30, hwnd, (HMENU)ID_EDIT_USER, GetModuleHandle(NULL), NULL);
        hEditPass = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Password", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD, rect.left + 100, rect.top + 140, 300, 30, hwnd, (HMENU)ID_EDIT_PASS, GetModuleHandle(NULL), NULL);

        placeholders[0] = { hEditUser, L"Username", true };
        placeholders[1] = { hEditPass, L"Password", true };

        SetWindowSubclass(hEditUser, EditProc, 0, 0);
        SetWindowSubclass(hEditPass, EditProc, 0, 0);

        hCheckboxRemember = CreateWindow(L"BUTTON", L"", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, rect.left + 100, rect.top + 180, 20, 20, hwnd, (HMENU)ID_CHECKBOX_REMEMBER, GetModuleHandle(NULL), NULL);
        hCheckboxText = CreateWindow(L"STATIC", L"Remember Me", WS_VISIBLE | WS_CHILD, rect.left + 130, rect.top + 183, 120, 30, hwnd, (HMENU)3002, GetModuleHandle(NULL), NULL);

        SendMessage(hCheckboxText, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

        int buttonWidth = 80, buttonHeight = 30, buttonSpacing = 10, totalButtonWidth = 4 * buttonWidth + 3 * buttonSpacing, buttonStartX = (rect.right - totalButtonWidth) / 2;

        hButtonLogin = CreateWindow(L"BUTTON", L"Login", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonStartX, rect.top + 210, buttonWidth, buttonHeight, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButtonLogin, GWLP_USERDATA, (LONG_PTR)L"Login");

        hButtonRegister = CreateWindow(L"BUTTON", L"Register", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonStartX + buttonWidth + buttonSpacing, rect.top + 210, buttonWidth, buttonHeight, hwnd, (HMENU)3, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButtonRegister, GWLP_USERDATA, (LONG_PTR)L"Register");

        hButtonAbout = CreateWindow(L"BUTTON", L"About Us", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonStartX + 2 * (buttonWidth + buttonSpacing), rect.top + 210, buttonWidth, buttonHeight, hwnd, (HMENU)4, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButtonAbout, GWLP_USERDATA, (LONG_PTR)L"About Us");

        hButtonExit = CreateWindow(L"BUTTON", L"Exit", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, buttonStartX + 3 * (buttonWidth + buttonSpacing), rect.top + 210, buttonWidth, buttonHeight, hwnd, (HMENU)5, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButtonExit, GWLP_USERDATA, (LONG_PTR)L"Exit");

        hButton1 = CreateWindow(L"BUTTON", L"Spoofer", WS_TABSTOP | WS_CHILD | BS_OWNERDRAW, buttonStartX, rect.top + 210, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BUTTON1, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButton1, GWLP_USERDATA, (LONG_PTR)L"Spoofer");

        hButton2 = CreateWindow(L"BUTTON", L"Cleaner", WS_TABSTOP | WS_CHILD | BS_OWNERDRAW, buttonStartX + buttonWidth + buttonSpacing, rect.top + 210, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BUTTON2, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButton2, GWLP_USERDATA, (LONG_PTR)L"Cleaner");

        HWND hButtonDeconnect = CreateWindow(L"BUTTON", L"Deconnect", WS_TABSTOP | WS_CHILD | BS_OWNERDRAW, buttonStartX, rect.top + 210 + buttonHeight + buttonSpacing, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BUTTON_DECONNECT, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButtonDeconnect, GWLP_USERDATA, (LONG_PTR)L"Deconnect");

        HWND hButtonExitMain = CreateWindow(L"BUTTON", L"Exit", WS_TABSTOP | WS_CHILD | BS_OWNERDRAW, buttonStartX + buttonWidth + buttonSpacing, rect.top + 210 + buttonHeight + buttonSpacing, buttonWidth, buttonHeight, hwnd, (HMENU)ID_BUTTON_EXIT, GetModuleHandle(NULL), NULL);
        SetWindowLongPtr(hButtonExitMain, GWLP_USERDATA, (LONG_PTR)L"Exit");

        MARGINS margins = { 1, 1, 1, 1 };
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        ShowMainControls(hwnd, FALSE);
    } return 0;

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 1:
        case 5:
        case ID_BUTTON_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case 2: {
            std::thread loginThread(HandleLogin, hwnd);
            loginThread.detach();
        } break;
        case 3:
            MessageBox(hwnd, L"Register clicked", L"Info", MB_OK);
            break;
        case 4:
            MessageBox(hwnd, L"About Us clicked", L"Info", MB_OK);
            break;
        case ID_BUTTON_DECONNECT:
            ShowMainControls(hwnd, FALSE);
            ShowLoginControls(hwnd, TRUE);
            break;
        case 1000:
            ShowLoginControls(hwnd, FALSE);
            ShowMainControls(hwnd, TRUE);
            break;
        }
    } return 0;

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
        if (lpDrawItem->CtlType == ODT_BUTTON) {
            BOOL isPressed = (lpDrawItem->itemState & ODS_SELECTED) ? TRUE : FALSE;
            LPCWSTR text = (LPCWSTR)GetWindowLongPtr(lpDrawItem->hwndItem, GWLP_USERDATA);
            DrawCustomButton(lpDrawItem->hDC, lpDrawItem->rcItem, text, isPressed);
            return TRUE;
        }
    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
        const wchar_t* title = L"LEKSA667";
        HFONT hFont = CreateFont(43, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(0, 255, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, rect.right - 180, 3, title, wcslen(title));
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        EndPaint(hwnd, &ps);
    } return 0;

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        SetTextColor(hdcStatic, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_NCHITTEST: {
        LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
        if (hit == HTCLIENT) {
            return HTCAPTION;
        }
        return hit;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_LAYERED, CLASS_NAME, L"Modern Rounded UI", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 500, 400, nullptr, nullptr, hInstance, nullptr);
    if (hwnd == nullptr) {
        return 0;
    }
    HRGN hRgn = CreateRoundedRectRgn(500, 400, 20);
    SetWindowRgn(hwnd, hRgn, TRUE);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}