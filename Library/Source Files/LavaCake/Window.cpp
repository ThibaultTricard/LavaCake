#include "Window.h"

enum UserMessage {
	USER_MESSAGE_RESIZE = WM_USER + 1,
	USER_MESSAGE_QUIT,
	USER_MESSAGE_MOUSE_CLICK,
	USER_MESSAGE_MOUSE_MOVE,
	USER_MESSAGE_MOUSE_WHEEL,
	USER_MESSAGE_KEY_UP,
	USER_MESSAGE_KEY_DOWN
};



LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_LBUTTONDOWN:
		PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 1);
		break;
	case WM_LBUTTONUP:
		PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 0, 0);
		break;
	case WM_RBUTTONDOWN:
		PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 1);
		break;
	case WM_RBUTTONUP:
		PostMessage(hWnd, USER_MESSAGE_MOUSE_CLICK, 1, 0);
		break;
	case WM_MOUSEMOVE:
		PostMessage(hWnd, USER_MESSAGE_MOUSE_MOVE, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEWHEEL:
		PostMessage(hWnd, USER_MESSAGE_MOUSE_WHEEL, HIWORD(wParam), 0);
		break;
	case WM_SIZE:
	case WM_EXITSIZEMOVE:
		PostMessage(hWnd, USER_MESSAGE_RESIZE, wParam, lParam);
		break;
	case WM_KEYDOWN:
		if (VK_ESCAPE == wParam) {
			PostMessage(hWnd, USER_MESSAGE_QUIT, wParam, lParam);
		}
		else {
			PostMessage(hWnd, USER_MESSAGE_KEY_DOWN, wParam, lParam);
		}
		break;
	case WM_KEYUP :
		PostMessage(hWnd, USER_MESSAGE_KEY_UP, wParam, lParam);
		break;

	case WM_CLOSE:
		PostMessage(hWnd, USER_MESSAGE_QUIT, wParam, lParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

namespace LavaCake {
	namespace Framework {
		Window::Window(
			const char               * window_title,
			int                        x,
			int                        y,
			int                        width,
			int                        height) {
			m_windowParams.HInstance = GetModuleHandle(nullptr);

			WNDCLASSEX window_class = {
				sizeof(WNDCLASSEX),               // UINT         cbSize
																					/* Win 3.x */
				CS_HREDRAW | CS_VREDRAW,          // UINT         style
				WindowProcedure,                  // WNDPROC      lpfnWndProc
				0,                                // int          cbClsExtra
				0,                                // int          cbWndExtra
				m_windowParams.HInstance,           // HINSTANCE    hInstance
				nullptr,                          // HICON        hIcon
				LoadCursor(nullptr, IDC_ARROW),   // HCURSOR      hCursor
				(HBRUSH)(COLOR_WINDOW + 1),       // HBRUSH       hbrBackground
				nullptr,                          // LPCSTR       lpszMenuName
				"LavaCake",												// LPCSTR       lpszClassName
																					/* Win 4.0 */
				nullptr                           // HICON        hIconSm
			};

			m_windowSize = { width ,height };

			if (!RegisterClassEx(&window_class)) {
        ErrorCheck::setError("Failed to register window class");
			}

			m_windowParams.HWnd = CreateWindow("LavaCake", window_title, WS_OVERLAPPEDWINDOW, x, y, width, height, nullptr, nullptr, m_windowParams.HInstance, nullptr);
			if (!m_windowParams.HWnd) {
        ErrorCheck::setError("Failed to create window");
			}
		}


		void Window::PrepareVulkanContext( int nbComputeQueue, int nbGraphicQueue,  VkPhysicalDeviceFeatures* desired_device_features ) {
			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			d->initDevices(desired_device_features, nbComputeQueue, nbGraphicQueue, m_windowParams);
		}


		void Window::PrepareFrames(
			uint32_t									 framesCount,
			VkImageUsageFlags          swapchain_image_usage,
			bool                       use_depth,
			VkImageUsageFlags          depth_attachment_usage) {

			LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
			d->prepareFrames(framesCount, swapchain_image_usage, use_depth, depth_attachment_usage);
		}


		void Window::Show() {
			ShowWindow(m_windowParams.HWnd, SW_SHOWNORMAL);
			UpdateWindow(m_windowParams.HWnd);
		}



		void Window::UpdateInput() {
			MSG message;
			m_mouse.beginFrame();
			if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
				switch (message.message) {
				case USER_MESSAGE_MOUSE_CLICK:
					m_mouse.click(static_cast<size_t>(message.wParam), message.lParam > 0);
					break;
				case USER_MESSAGE_MOUSE_MOVE:
					m_mouse.move(static_cast<int>(message.wParam), static_cast<int>(message.lParam));
					break;
				case USER_MESSAGE_MOUSE_WHEEL:
					m_mouse.roll(static_cast<short>(message.wParam) * 0.002f);
					break;
				case USER_MESSAGE_KEY_DOWN:
					m_keyBoard.keyDown(message.wParam);
					std::cout << "test" << std::endl;
					break;
				case USER_MESSAGE_KEY_UP:
					m_keyBoard.keyUp(message.wParam);
					break;
				case USER_MESSAGE_RESIZE:
					/*if (!Sample.Resize()) {
						loop = false;
					}*/
					break;
				case USER_MESSAGE_QUIT:
					m_loop = false;
					break;
				}
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}


		Key KeyBoard::getKey(std::string keyName) {
			if (keyName.length() == 1) {
				return m_keys[keyName[0]];
			}
			else {
				if (keyName == "ctrl") {
					return m_keys[VK_CONTROL];
				}
				if (keyName == "shift") {
					return m_keys[VK_SHIFT];
				}
				if (keyName == "space") {
					return m_keys[VK_SPACE];
				}
				if (keyName == "alt") {
					return m_keys[VK_SHIFT];
				}
				if (keyName == "up") {
					return m_keys[VK_UP];
				}
				if (keyName == "down") {
					return m_keys[VK_DOWN];
				}
				if (keyName == "left") {
					return m_keys[VK_LEFT];
				}
				if (keyName == "right") {
					return m_keys[VK_RIGHT];
				}
			}
		}
	}
}