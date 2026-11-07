#include "./win32_api.hpp"
#include "./system.hpp"

#include <exception>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int nCmdShow) {

	BOOL ret{ };

	System::MainWindow obj{ hInstance };

	try {
		obj.initialize(nCmdShow);

		MSG msg{ };

		while ((ret = GetMessageW(&msg, nullptr, 0, 0)) > 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	catch (const std::exception& e) {
		MessageBoxA(obj.hwnd(), e.what(), "TERMINAL ERROR: ", MB_ICONERROR | MB_OK);
	}

	

	return ret;
}
