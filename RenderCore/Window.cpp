#include "Window.h"

LRESULT Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
        DestroyWindow(hWnd);
	case WM_DESTROY:
		PostQuitMessage(0);
        return 0;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

Window::Window()
	: m_hInstance(nullptr)
	, m_hWnd(nullptr)
	, m_pszWindowName(L"Stress Test Client")
{
}

HRESULT Window::Initialize(HINSTANCE hInstance, INT nCmdShow, PCWSTR pszWindowName)
{
	return E_NOTIMPL;
}

PCWSTR Window::GetWindowClassName() const
{
    return L"Window Class";
}

LRESULT Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}

HWND Window::GetWindow() const
{
	return m_hWnd;
}

HRESULT Window::initialize(
    _In_ HINSTANCE hInstance,
    _In_ INT nCmdShow,
    _In_ PCWSTR pszWindowName,
    _In_ DWORD dwStyle,
    _In_opt_ INT x,
    _In_opt_ INT y,
    _In_opt_ INT nWidth,
    _In_opt_ INT nHeight,
    _In_opt_ HWND hWndParent,
    _In_opt_  HMENU hMenu
)
{
    // Register the window class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_ERROR);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = GetWindowClassName();
    wcex.hIconSm = LoadIcon(nullptr, IDI_ERROR);

    if (!RegisterClassEx(&wcex))
    {
        DWORD dwError = GetLastError();
        MessageBox(
            nullptr,
            L"Call to RegisterClassEx failed!",
            L"RenderCore",
            NULL
        );
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
        {
            return HRESULT_FROM_WIN32(dwError);
        }
        return E_FAIL;
    }

    // Creates the window
    m_hInstance = hInstance;

    m_hWnd = CreateWindowEx(
        0,
        GetWindowClassName(),
        pszWindowName,
        dwStyle,
        x, y, nWidth, nHeight,
        hWndParent,
        hMenu,
        hInstance,
        this
    );

    if (!m_hWnd)
    {
        DWORD dwError = GetLastError();
        MessageBox(
            nullptr,
            L"Call to CreateWindow failed!",
            L"RenderCore",
            NULL
        );
        if (dwError != ERROR_CLASS_ALREADY_EXISTS)
        {
            return HRESULT_FROM_WIN32(dwError);
        }
        return E_FAIL;
    }

    // show window
    ShowWindow(m_hWnd, nCmdShow);

    return S_OK;
}
