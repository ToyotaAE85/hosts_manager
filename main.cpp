#include "./win32_api.hpp"
#include "./system.hpp"


int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int nCmdShow) {

	System::MainWindow obj{ hInstance };
	obj.initialize(nCmdShow);

	MSG msg{ };
	BOOL ret{ };

	while ((ret = GetMessageW(&msg, nullptr, 0,0)) > 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return ret;
}
