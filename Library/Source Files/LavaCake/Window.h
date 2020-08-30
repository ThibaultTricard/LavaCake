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

		class Window{
		public :

			Window(
				const char               * window_title,
				int                        width,
				int                        height);



			void UpdateInput();

			bool running() {
				return !glfwWindowShouldClose(m_window);
			}

			~Window() {
				glfwDestroyWindow(m_window);
				glfwTerminate();
			}

			GLFWwindow*																m_window;
			WindowParameters													m_windowParams;
		};

	}
}