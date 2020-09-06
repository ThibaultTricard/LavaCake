#include "Window.h"


namespace LavaCake {
	namespace Framework {

		static void callbackMousePosition(GLFWwindow* window, double xpos, double ypos) {
			Mouse* m = Mouse::getMouse();
			m->position[0] = xpos;
			m->position[1] = ypos;
		}


		static void callbackWheel(GLFWwindow* window, double xoffset, double yoffset)
		{
			Mouse* m = Mouse::getMouse();
			m->wheel = yoffset;
		}


		static void callbackMouseButton(GLFWwindow* window, int button, int action, int mods)
		{

			Mouse* m = Mouse::getMouse();
			if (action == GLFW_PRESS) {
				if (button == GLFW_MOUSE_BUTTON_RIGHT) {
					m->rightButton = true;
				}
				else if (button == GLFW_MOUSE_BUTTON_LEFT) {
					m->leftButton = true;
				}
				else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
					m->middleButton = true;
				}
			}
			if (action == GLFW_RELEASE) {
				if (button == GLFW_MOUSE_BUTTON_RIGHT) {
					m->rightButton = false;
				}
				else if (button == GLFW_MOUSE_BUTTON_LEFT) {
					m->leftButton = false;
				}
				else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
					m->middleButton = false;
				}
			}
				
		}


		Mouse* Mouse::m_mouse;
		
		void Mouse::init(GLFWwindow* window) {
			m_mouse = new Mouse();
			glfwSetCursorPosCallback(window, callbackMousePosition);
			glfwSetScrollCallback(window, callbackWheel);
			glfwSetMouseButtonCallback(window, callbackMouseButton);
		}

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

			m_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
			glfwSetCursor(m_window, m_cursor);
			Mouse::init(m_window);

		}


		void Window::updateInput() {
			glfwPollEvents();
		}

	}
}