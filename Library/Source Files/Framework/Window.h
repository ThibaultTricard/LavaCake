#pragma once
#include "AllHeaders.h"
#include "Tools.h"
#include "Device.h"
#include "ErrorCheck.h"
#include "Math/basics.h"


namespace LavaCake {
	namespace Framework {

		class Mouse {
			
		public:

			static Mouse* getMouse() {
				return m_mouse;
			}

			static void init(GLFWwindow* window);

			bool leftButton = false;
			bool rightButton = false;
			bool middleButton = false;
			vec2d position = vec2d({ 0.0,0.0 });
			double wheel = 0.0;

		private:

			static Mouse* m_mouse;

			Mouse() {};

			
		};


		class Window{
		public :

      Window(
          const char               * window_title,
          int                        width,
          int                        height) {
          
          glfwInit();

          
          glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
          //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
          
          m_window = glfwCreateWindow(width, height, window_title, nullptr, nullptr);
          
          m_windowParams.Window = m_window;

          Mouse::init(m_window);

      }

			void updateInput();

			bool running() {
				return !glfwWindowShouldClose(m_window);
			}

			~Window() {
				glfwSetCursor(m_window, NULL);
				glfwDestroyWindow(m_window);
				glfwTerminate();
			}

			GLFWwindow*																m_window;
			WindowParameters													m_windowParams;
		};

	}
}
