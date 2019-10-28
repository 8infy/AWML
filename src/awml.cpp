#ifdef _WIN32
    #include "awml_windows.h"
#endif

namespace awml {

    Window::SharedWindow Window::Create(
        const std::wstring& title,
        uint16_t width,
        uint16_t height,
        Context context,
        WindowMode window_mode,
        CursorMode cursor_mode,
        bool resizable
    )
    {
      #ifdef _WIN32
        auto window =
            std::make_shared<WindowsWindow>(
                title,
                width,
                height,
                context,
                window_mode,
                cursor_mode,
                resizable
            );

        switch (context)
        {
        case Context::NONE:
            return window;
        case Context::OpenGL:
            window->SetContext(
                std::make_unique<WindowsOpenGLContext>()
            );
            return window;
        default:
            return window;
        }

      #endif
    }
}
