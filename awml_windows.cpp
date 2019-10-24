#include <iostream>
#include <string>
#include <functional>

#include "awml_windows.h"

#define AWML_WINDOWS_WINDOW_STYLE WS_OVERLAPPEDWINDOW

namespace awml {

    WindowsWindow::WindowsWindow(
        const std::wstring& title,
        uint16_t width,
        uint16_t height
    ) : m_ClassName(title),
        m_WindowTitle(title),
        m_WinProps(),
        m_Window(),
        m_Width(0),
        m_Height(0),
        m_MouseX(0),
        m_MouseY(0),
        m_ShouldClose(false)
    {
        m_ClassName += std::to_wstring(s_WindowID++);

        m_WinProps.lpfnWndProc = WindowEventHandler;
        m_WinProps.hInstance = s_ThisInstance;
        m_WinProps.lpszClassName = m_ClassName.c_str();
        m_WinProps.cbWndExtra = sizeof(WindowsWindow*);
        RegisterClassW(&m_WinProps);

        RECT rect;
        rect.left = rect.top = 0;
        rect.right = width;
        rect.bottom = height;
        AdjustWindowRect(&rect, AWML_WINDOWS_WINDOW_STYLE, false);

        m_Window = CreateWindowExW(
            0,
            m_ClassName.c_str(),
            m_WindowTitle.c_str(),
            AWML_WINDOWS_WINDOW_STYLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            NULL,
            NULL,
            s_ThisInstance,
            NULL
        );

        if (m_Window == NULL)
            throw std::exception("Could not create a window!");

        SetWindowLongPtrW(m_Window, 0, reinterpret_cast<LONG_PTR>(this));

        ShowWindow(m_Window, SW_NORMAL);
    }

    void WindowsWindow::PollEvents()
    {
        auto message = MSG();
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    bool WindowsWindow::ShouldClose()
    {
        return m_ShouldClose;
    }

    void WindowsWindow::Close()
    {
        if (m_Window)
        {
            DestroyWindow(m_Window);
            m_Window = nullptr;
        }
    }

    uint16_t WindowsWindow::Width()
    {
        return m_Width;
    }

    uint16_t WindowsWindow::Height()
    {
        return m_Height;
    }

    uint16_t WindowsWindow::MouseX()
    {
        return m_MouseX;
    }
    
    uint16_t WindowsWindow::MouseY()
    {
        return m_MouseY;
    }

    void WindowsWindow::OnKeyPressedFunc(key_pressed_callback cb)
    {
        m_KeyPressedCB = cb;
    }

    void WindowsWindow::OnKeyReleasedFunc(key_released_callback cb)
    {
        m_KeyReleasedCB = cb;
    }

    void WindowsWindow::OnWindowResizedFunc(window_resized_callback cb)
    {
        m_WindowResizedCB = cb;
    }

    void WindowsWindow::OnWindowClosedFunc(window_closed_callback cb)
    {
        m_WindowClosedCB = cb;
    }

    void WindowsWindow::OnMouseMovedFunc(mouse_moved_callback cb)
    {
        m_MouseMovedCB = cb;
    }

    void WindowsWindow::OnMousePressedFunc(mouse_pressed_callback cb)
    {
        m_MousePressedCB = cb;
    }

    void WindowsWindow::OnMouseReleasedFunc(mouse_released_callback cb)
    {
        m_MouseReleasedCB = cb;
    }

    void WindowsWindow::OnMouseScrolledFunc(mouse_scrolled_callback cb)
    {
        m_MouseScrolledCB = cb;
    }

    void WindowsWindow::OnCharTypedFunc(char_typed_callback cb)
    {
        m_CharTypedCB = cb;
    }

    bool WindowsWindow::KeyPressed(awml_keycode key_code)
    {
        return AWML_KEY_PRESSED_BIT & GetKeyState(key_code);
    }

    WindowsWindow::~WindowsWindow()
    {
        Close();
    }

    void WindowsWindow::OnWindowResized(WORD width, WORD height)
    {
        m_Width = width;
        m_Height = height;

        if (m_WindowResizedCB)
            m_WindowResizedCB(width, height);
    }

    void WindowsWindow::OnWindowClosed()
    {
        if (m_WindowClosedCB)
            m_WindowClosedCB();
    }

    void WindowsWindow::OnMouseMoved(WORD xpos, WORD ypos)
    {
        // The window can generate spurious mouse moved events,
        // so we have to handle that case here.
        if (xpos != m_MouseX || ypos != m_MouseY)
        {
            m_MouseX = xpos;
            m_MouseY = ypos;

            if (m_MouseMovedCB)
                m_MouseMovedCB(m_MouseX, m_MouseY);
        }
    }

    void WindowsWindow::OnMousePressed(UINT code)
    {
        if (m_MousePressedCB)
            m_MousePressedCB(
                static_cast<awml_keycode>(code)
            );
    }

    void WindowsWindow::OnMouseReleased(UINT code)
    {
        if (m_MouseReleasedCB)
            m_MouseReleasedCB(
                static_cast<awml_keycode>(code)
            );
    }

    void WindowsWindow::OnMouseScrolled(int16_t rotation, bool vertical)
    {
        rotation /= 12;

        // make sure we're not out of bounds
        if      (rotation >  10) rotation =  10;
        else if (rotation < -10) rotation = -10;

        if (m_MouseScrolledCB)
            m_MouseScrolledCB(rotation, vertical);
    }

    void WindowsWindow::OnKeyPressed(WPARAM key_code, bool repeated, uint16_t repeat_count)
    {
        if (m_KeyPressedCB)
            m_KeyPressedCB(
                static_cast<awml_keycode>(key_code),
                repeated, repeat_count
            );
    }

    void WindowsWindow::OnKeyReleased(WPARAM key_code)
    {
        if (m_KeyReleasedCB)
            m_KeyReleasedCB(
                static_cast<awml_keycode>(key_code)
            );
    }

    void WindowsWindow::OnCharTyped(wchar_t typed_char)
    {
        if (m_CharTypedCB)
            m_CharTypedCB(typed_char);
    }

    LRESULT CALLBACK WindowsWindow::WindowEventHandler(
        HWND window,
        UINT message,
        WPARAM param_1,
        LPARAM param_2
    )
    {
        WindowsWindow* owner =
            reinterpret_cast<WindowsWindow*>(GetWindowLongPtrW(window, 0));

        if (!owner)
            return DefWindowProcW(window, message, param_1, param_2);

        switch (message)
        {
        case WM_DESTROY:
            owner->OnWindowClosed();
            return 0;
        case WM_CLOSE:
            owner->m_ShouldClose = true;
            return 0;
        case WM_SIZE:
            owner->OnWindowResized(
                LOWORD(param_2),
                HIWORD(param_2)
            );
            return 0;
        case WM_MOUSEMOVE:
            owner->OnMouseMoved(
                GET_X_LPARAM(param_2),
                GET_Y_LPARAM(param_2)
            );
            return 0;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            owner->OnMousePressed(message);
            return 0;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            owner->OnMouseReleased(message);
            return 0;
        case WM_MOUSEWHEEL:
            owner->OnMouseScrolled(
                GET_WHEEL_DELTA_WPARAM(param_1),
                true
            );
            return 0;
        case WM_MOUSEHWHEEL:
            owner->OnMouseScrolled(
                GET_WHEEL_DELTA_WPARAM(param_1),
                false
            );
            return 0;
        case WM_KEYDOWN:
            owner->OnKeyPressed(
                param_1,
                param_2 & AWML_REPEATED_BIT,
                param_2 & AWML_REPEAT_COUNT_MASK
            );
            return 0;
        case WM_KEYUP:
            owner->OnKeyReleased(param_1);
            return 0;
        case WM_CHAR:
            owner->OnCharTyped(static_cast<wchar_t>(param_1));
            return 0;
        default:
            return DefWindowProcW(window, message, param_1, param_2);
        }
    }

    HINSTANCE WindowsWindow::s_ThisInstance = GetModuleHandleW(NULL);
    uint16_t  WindowsWindow::s_WindowID = 1;
}