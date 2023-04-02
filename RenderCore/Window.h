#pragma once
#include "Common.h"

class Window 
{
public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    Window();
    Window(const Window& rhs) = delete;
    Window(Window&& rhs) = delete;
    Window& operator=(const Window& rhs) = delete;
    Window& operator=(Window&& rhs) = delete;
    ~Window() = default;

    HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName);
    PCWSTR GetWindowClassName() const;
    LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    HWND GetWindow() const;

private:
    HRESULT initialize(
        _In_ HINSTANCE hInstance,
        _In_ INT nCmdShow,
        _In_ PCWSTR pszWindowName,
        _In_ DWORD dwStyle,
        _In_opt_ INT x = CW_USEDEFAULT,
        _In_opt_ INT y = CW_USEDEFAULT,
        _In_opt_ INT nWidth = CW_USEDEFAULT,
        _In_opt_ INT nHeight = CW_USEDEFAULT,
        _In_opt_ HWND hWndParent = nullptr,
        _In_opt_ HMENU hMenu = nullptr
    );

    HINSTANCE m_hInstance;
    HWND m_hWnd;
    LPCWSTR m_pszWindowName;
};