#include "LavaCake/Framework.h"

using namespace LavaCake::Framework;

int main() {

	Window w("Creating a Window", 0, 0, 500, 500);
	w.PrepareVulkanContext(1, 1);
	w.Show();

	MSG message;

	while (w.m_loop) {
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {

		}
		else {

		}
	}
}