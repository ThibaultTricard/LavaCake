#pragma once
#include "AllHeaders.h"
#include "VulkanDestroyer.h"
#include "Tools.h"
#include "Device.h"
#include "ErrorCheck.h"


#ifdef _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32 true
#endif

#define GLFW_INCLUDE_NONE
#include "glfw3.h"
#include "glfw3native.h"

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
				int                        height);



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

			Mouse* m_mouse;
		};

	}
}