#include <string>

#include <X11/XKBlib.h>

#include "awml_xwindow.h"

namespace awml {

    XWindow::XWindow(
        const std::wstring& title,
        uint16_t width,
        uint16_t height,
        Context context,
        WindowMode window_mode,
        CursorMode cursor_mode,
        bool resizable
    ) : m_Width(width),
        m_Height(height),
        m_Context(context),
        m_WindowMode(window_mode),
        m_CursorMode(cursor_mode),
        m_ShouldClose(false)
    {
    }

    void XWindow::Launch() 
    {
        m_Connection = XOpenDisplay(NULL);
        XkbSetDetectableAutoRepeat(m_Connection, true, NULL);

        if (!m_Connection)
        {
            //OnError(error::NULL_WINDOW, "Could not establish connection with the X server!");
            return;
        }

        uint32_t screen_num = DefaultScreen(m_Connection);
        uint32_t background_color = BlackPixel(m_Connection, screen_num);
        uint32_t border_color = WhitePixel(m_Connection, screen_num);

        m_Window = XCreateSimpleWindow(
            m_Connection,
            DefaultRootWindow(m_Connection),
            0, 0,
            m_Width,
            m_Height,
            2, border_color,
            background_color
        );

        XSelectInput(
            m_Connection,
            m_Window,
            KeyPressMask      |
            ButtonPressMask   |
            KeyReleaseMask    |
            ButtonReleaseMask |
            PointerMotionMask |
            StructureNotifyMask
        );

        XMapWindow(m_Connection, m_Window);

        Atom WM_DELETE_WINDOW = XInternAtom(m_Connection, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(m_Connection, m_Window, &WM_DELETE_WINDOW, 1);
    }

    void XWindow::SetContext(window_context wc) 
    {

    }

    void XWindow::Update() 
    {
        XPending(m_Connection);

        while (XQLength(m_Connection))
        {
            XNextEvent(m_Connection, &m_Event);

            switch (m_Event.type)
            {
            case ConfigureNotify:

                if (m_Event.xconfigure.width  == m_Width &&
                    m_Event.xconfigure.height == m_Height)
                    return;

                m_Width = m_Event.xconfigure.width;
                m_Height = m_Event.xconfigure.height;

                if (m_WindowResizedCB)
                    m_WindowResizedCB(m_Width, m_Height);

                break;

            case ButtonPress:
	    {
                auto button = m_Event.xbutton.button;
	         
	        if (button == 4 || button == 5)
		{
                    if (m_MouseScrolledCB)
			m_MouseScrolledCB(button == 4 ? 10 : -10, true);
		    break;
		}
		else if (button == 6 || button == 7)
		{
                    if (m_MouseScrolledCB)
			m_MouseScrolledCB(button == 6 ? 10 : -10, false);
		    break;
		}
		else if (m_MousePressedCB)
                    m_MousePressedCB(
                        static_cast<awml_key>(m_Event.xbutton.button)
                    );

                break;
            }
            case ButtonRelease:
            {
                auto button = m_Event.xbutton.button;
	         
	        if (button == 4 || button == 5 || button == 6 || button == 7)
                    break;

                if (m_MouseReleasedCB)
                    m_MouseReleasedCB(
                        static_cast<awml_key>(m_Event.xbutton.button)
                    );

                    break;
            }
            case KeyPress:
            {
                wchar_t typed_char = GetTypedChar();

                if (typed_char && m_CharTypedCB)
                    m_CharTypedCB(
                       typed_char
                    );

                auto key = NormalizeKeyPress();

                if (m_KeyPressedCB)
                    m_KeyPressedCB(
                        key,
                        GetKeyRepeatCount(key),
                        GetKeyRepeatCount(key)
                    );

                IncremetRepeatCount(key);

                break;
            }
            case KeyRelease:
            {
                auto key = NormalizeKeyPress();

                if (m_KeyReleasedCB)
                    m_KeyReleasedCB(key);

                ResetRepeatCount(key);

                break;
            }
            case MotionNotify:
                if (m_MouseMovedCB)
                    m_MouseMovedCB(
                        m_Event.xmotion.x,
                        m_Event.xmotion.y
                    );

                    break;

            case ClientMessage:
                m_ShouldClose = true;

                break;

            default:
                break;
            }
        }
    }

    bool XWindow::ShouldClose()
    {
        return m_ShouldClose;
    }

    void XWindow::Close()
    {
    }

    uint16_t XWindow::Width()
    {
        return m_Width;
    }

    uint16_t XWindow::Height()
    {
        return m_Height;
    }

    uint16_t XWindow::MouseX()
    {
        return m_MouseX;
    }

    uint16_t XWindow::MouseY()
    {
        return m_MouseY;
    }

    void XWindow::OnErrorFunc(
        error_callback cb
    )
    {

    }

    void XWindow::OnKeyPressedFunc(
        key_pressed_callback cb
    )
    {
        m_KeyPressedCB = cb;
    }

    void XWindow::OnKeyReleasedFunc(
        key_released_callback cb
    )
    {
        m_KeyReleasedCB = cb;
    }

    void XWindow::OnWindowResizedFunc(
        window_resized_callback cb
    )
    {
        m_WindowResizedCB = cb;
    }

    void XWindow::OnWindowClosedFunc(
        window_closed_callback cb
    )
    {

    }

    void XWindow::OnMouseMovedFunc(
        mouse_moved_callback cb
    )
    {
        m_MouseMovedCB = cb;
    }

    void XWindow::OnMousePressedFunc(
        mouse_pressed_callback cb
    )
    {
        m_MousePressedCB = cb;
    }

    void XWindow::OnMouseReleasedFunc(
        mouse_released_callback cb
    )
    {
       m_MouseReleasedCB = cb;
    }

    void XWindow::OnMouseScrolledFunc(
        mouse_scrolled_callback cb
    )
    {
       m_MouseScrolledCB = cb;
    }

    void XWindow::OnCharTypedFunc(
        char_typed_callback cb
    )
    {
        m_CharTypedCB = cb;
    }

    bool XWindow::Minimized()
    {
        return m_Width == 0 && m_Height == 0;
    }

    bool XWindow::KeyPressed(awml_key key_code)
    {

    }

    void XWindow::SetCursorMode(CursorMode cursor_mode)
    {

    }

    void XWindow::SetWindowMode(WindowMode window_mode)
    {

    }

    void XWindow::Resize(uint16_t width, uint16_t height)
    {

    }

    awml_key XWindow::NormalizeKeyPress()
    {
        auto key_sym = XkbKeycodeToKeysym(
            m_Connection,
            m_Event.xkey.keycode,
            0,
            m_Event.xkey.state & ShiftMask ? 1 : 0
        );

        KeySym lower_sym;
        KeySym upper_sym;

        XConvertCase(key_sym, &lower_sym, &upper_sym);

        return static_cast<awml_key>(lower_sym);
    }

    wchar_t XWindow::GetTypedChar()
    {
        auto key_sym = XkbKeycodeToKeysym(
            m_Connection,
            m_Event.xkey.keycode,
            0,
            m_Event.xkey.state & ShiftMask ? 1 : 0
        );

        // This is not how you actually get the typed char
        // To be fixed later.
        //static_cast<wchar_t>(*XKeysymToString(key_sym))

        return static_cast<wchar_t>(0);
    }

    uint8_t XWindow::GetKeyRepeatCount(awml_key code)
    {
        return m_RepeatCount.count(code) ?
            m_RepeatCount[code] : 0;
    }

    void XWindow::IncremetRepeatCount(awml_key code)
    {
        if (m_RepeatCount.count(code))
        {
            if (m_RepeatCount[code] != UINT8_MAX)
                m_RepeatCount[code]++;
        }
        else
            m_RepeatCount[code] = 1;
    }

    void XWindow::ResetRepeatCount(awml_key code)
    {
        if (m_RepeatCount.count(code))
            m_RepeatCount[code] = 0;
    }

    XWindow::~XWindow() 
    {
        XCloseDisplay(m_Connection);
    }
}
