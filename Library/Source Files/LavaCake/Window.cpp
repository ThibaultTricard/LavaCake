#include "Window.h"


namespace LavaCake {
	namespace Framework {
		Window::Window(
			const char               * window_title,
			int                        width,
			int                        height) {

			glfwInit();


			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			m_window = glfwCreateWindow(width, height, window_title, nullptr, nullptr);
			m_windowParams = { GetModuleHandleW(NULL), glfwGetWin32Window(m_window) };


			if (!m_windowParams.HWnd) {
        ErrorCheck::setError("Failed to create window");
			}
		}


		void Window::UpdateInput() {
			glfwPollEvents();
		}

	}
}