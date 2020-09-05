#include "Framework/Framework.h"

using namespace LavaCake::Framework;

int main() {

	Window w("Creating a Window", 500, 500);


	while (w.running()) {
	
		w.UpdateInput();

	}
}