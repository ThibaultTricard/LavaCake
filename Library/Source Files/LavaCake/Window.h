#pragma once
#include "AllHeaders.h"
#include "VulkanDestroyer.h"
#include "Tools.h"
#include "Device.h"
#include "ErrorCheck.h"


#ifdef _WIN32
#include <Windows.h>
#endif


namespace LavaCake {
	namespace Framework {

		class Mouse {
		public :

			Mouse() {
				m_actionPerformed = false;
				m_buttons[0].pressed = false;
				m_buttons[0].hold = false;
				m_buttons[0].release = false;
				m_buttons[1].pressed = false;
				m_buttons[1].hold = false;
				m_buttons[1].release = false;
				m_position.x = 0;
				m_position.x = 0;
				m_position.delta.x = 0;
				m_position.delta.y = 0;
				m_wheel.moved = false;
				m_wheel.distance = 0.0f;
			}

			void beginFrame() {
				m_wheel.moved = false;
				m_actionPerformed = false;
				m_buttons[0].pressed = false;
				m_buttons[0].release = false;
				m_buttons[1].pressed = false;
				m_buttons[2].release = false;
			}

			void click(size_t button, bool state) {
				m_buttons[button].pressed = state;
				m_buttons[button].hold = state;
				m_buttons[button].release = !state;
				m_actionPerformed = true;
			}

			void move(int x, int y) {
				m_position.delta.x = x - m_position.x;
				m_position.delta.y = y - m_position.y;
				m_position.x = x;
				m_position.y = y;
				m_actionPerformed = true;
			}

			void roll(float d) {
				m_wheel.moved = true;
				m_wheel.distance += d;
				m_actionPerformed = true;
			}


			struct Button {
				bool pressed;
				bool hold;
				bool release;
			} m_buttons[2];

			struct Position {
				int x;
				int y;
				struct Delta {
					int x;
					int y;
				} delta;
			} m_position;

			struct Wheel {
				bool  moved;
				float distance;
			} m_wheel;

			bool m_actionPerformed;
		};

		struct Key
		{
			bool pressed;
			bool hold;
			bool realesed;
		};

		class KeyBoard {
		public:

			KeyBoard() {
				for (int i = 0; i < 255; i++) {
					m_keys[i].hold = false;
					m_keys[i].pressed = false;
					m_keys[i].realesed = false;
				}
			}

			void keyUp(int param) {
				m_keys[param].hold = false;
				m_keys[param].pressed = false;
				m_keys[param].realesed = true;
			}

			void keyDown(int param) {
				m_keys[param].pressed = !m_keys[param].hold ;
				m_keys[param].hold = true;
				m_keys[param].realesed = false;;
			}

			Key m_keys[255];

			Key getKey(std::string keyName);
		};

		class Window{
		public :

			Window(
				const char               * window_title,
				int                        x,
				int                        y,
				int                        width,
				int                        height);


			void Show();

			void UpdateInput();

			~Window() {
				LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
				d->end();

				if (m_windowParams.HWnd) {
					DestroyWindow(m_windowParams.HWnd);
				}

				if (m_windowParams.HInstance) {
					UnregisterClass("LavaCake", m_windowParams.HInstance);
				}
			}


			WindowParameters													m_windowParams;
			vec2i																			m_windowSize;
			Mouse																			m_mouse;
			KeyBoard																	m_keyBoard;
			bool																			m_loop = true;
		};

	}
}