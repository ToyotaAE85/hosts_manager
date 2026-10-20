#include "./component.hpp"

#include "./system.hpp"
#include "./localization.hpp"

#include <type_traits>
#include <format>

template <typename T>
requires std::is_scoped_enum_v<T> || std::is_integral_v<T>
void AppendToMenu(HMENU to, UINT flags, T uId, Localization::Text stringId = Localization::Text::NO_TEXT) {
	if (!AppendMenuW(to, flags, static_cast<UINT_PTR>(uId), Localization::get_text(stringId).data()))
		Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_UIFailed);
}

namespace Component {
	void MenuBar::create(HWND parent) {

		parent_ = parent;
		hMenu_ = build_menu();

		SetMenu(parent_, hMenu_);
		DrawMenuBar(parent_);
	}

	HMENU MenuBar::build_menu() {

		HMENU hMenu = CreateMenu();

		HMENU hFile{ CreatePopupMenu() };
		AppendToMenu(hFile, MF_STRING, WmId::MenuBar::File_EmptyFile, Localization::Text::File_EmptyFile);
		AppendToMenu(hFile, MF_SEPARATOR, 0);
		AppendToMenu(hFile, MF_STRING, WmId::MenuBar::File_BackupFile, Localization::Text::File_BackupFile);
		AppendToMenu(hFile, MF_STRING, WmId::MenuBar::File_RestoreFile, Localization::Text::File_RestoreFile);
		AppendToMenu(hFile, MF_SEPARATOR, 0);
		AppendToMenu(hFile, MF_STRING, WmId::MenuBar::File_Exit, Localization::Text::File_Exit);


		HMENU setText{ CreatePopupMenu() };
		AppendToMenu(setText, MF_STRING | MF_GRAYED, WmId::MenuBar::Text_ActualSize, Localization::Text::Text_ActualSize);
		AppendToMenu(setText, MF_SEPARATOR, 0);
		AppendToMenu(setText, MF_STRING, WmId::MenuBar::Text_IncreaseSize, Localization::Text::Text_IncreaseSize);
		AppendToMenu(setText, MF_STRING, WmId::MenuBar::Text_DecreaseSize, Localization::Text::Text_DecreaseSize);


		HMENU setLang{ CreatePopupMenu() };
		AppendToMenu(setLang, MF_STRING, WmId::MenuBar::Language_English, Localization::Text::Language_English);
		AppendToMenu(setLang, MF_STRING, WmId::MenuBar::Language_Spanish, Localization::Text::Language_Spanish);


		HMENU setFilter{ CreatePopupMenu() };
		AppendToMenu(setFilter, MF_STRING, WmId::MenuBar::Filter_Comments, Localization::Text::Filter_Comments);
		AppendToMenu(setFilter, MF_STRING, WmId::MenuBar::Filter_Blank, Localization::Text::Filter_Blank);
		AppendToMenu(setFilter, MF_STRING, WmId::MenuBar::Filter_Invalid, Localization::Text::Filter_Invalid);
		AppendToMenu(setFilter, MF_SEPARATOR, 0);
		AppendToMenu(setFilter, MF_STRING, WmId::MenuBar::Filter_All, Localization::Text::Filter_All);


		HMENU hSettings{ CreatePopupMenu() };
		AppendToMenu(hSettings, MF_POPUP, (UINT_PTR)setText, Localization::Text::Settings_Text);
		AppendToMenu(hSettings, MF_POPUP, (UINT_PTR)setLang, Localization::Text::Settings_Language);
		AppendToMenu(hSettings, MF_SEPARATOR, 0);
		AppendToMenu(hSettings, MF_POPUP, (UINT_PTR)setFilter, Localization::Text::Settings_FilterEntries);
		AppendToMenu(hSettings, MF_STRING, WmId::MenuBar::Settings_ReloadFile, Localization::Text::Settings_ReloadFile);


		AppendToMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, Localization::Text::Menu_File);
		AppendToMenu(hMenu, MF_POPUP, (UINT_PTR)hSettings, Localization::Text::Menu_Settings);

		update_menu_labels(hMenu);

		return hMenu;
	}


	bool MenuBar::check_menu_item(HMENU hMenu, WmId::MenuBar id, bool check) {

		MENUITEMINFOW mii{ 0 };
		mii.cbSize = sizeof(MENUITEMINFOW);
		mii.fMask = MIIM_STATE;
		mii.fState = check ? MFS_CHECKED : MFS_UNCHECKED;

		return SetMenuItemInfoW(hMenu, static_cast<UINT>(id), FALSE, &mii);
	}


	bool MenuBar::rename_menu_item(HMENU hMenu, WmId::MenuBar id, std::wstring_view new_text) {

		MENUITEMINFOW mii{ };
		mii.cbSize = sizeof(MENUITEMINFOW);
		mii.fMask = MIIM_STRING;
		mii.dwTypeData = const_cast<LPWSTR>(new_text.data());

		return SetMenuItemInfoW(hMenu, static_cast<UINT>(id), FALSE, &mii);
	}


	bool MenuBar::update_menu_labels(HMENU hMenu) {

		using namespace System;

		auto* pWnd{ reinterpret_cast<MainWindow*>(GetPropW(parent_, L"MAINMENUWINDOW")) };


		if (!pWnd)
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_HandleFailed);


		const auto& [language, textsize, filtermodes] {
			pWnd->request_settings<Setting::Language, Setting::TextSize, Setting::FilterModes>()
		};

		switch (language) {
			case Language::Type::Spanish: check_menu_item(hMenu, WmId::MenuBar::Language_Spanish, true); break;
			default: check_menu_item(hMenu, WmId::MenuBar::Language_English, true); break;
		}

		check_menu_item(hMenu, WmId::MenuBar::Filter_Invalid, filtermodes & 1);
		check_menu_item(hMenu, WmId::MenuBar::Filter_Blank, filtermodes & (1 << 1));
		check_menu_item(hMenu, WmId::MenuBar::Filter_Comments, filtermodes & (1 << 2));

		const std::wstring current_size_text{ std::format(L"{}: {}", Localization::get_text(Localization::Text::Text_ActualSize), textsize) };
		rename_menu_item(hMenu, WmId::MenuBar::Text_ActualSize, current_size_text);

		return true;
	}


	LRESULT MenuBar::MessageHandler(WORD cmd) {

		using namespace System;

		auto* pWnd{ reinterpret_cast<System::MainWindow*>(GetPropW(parent_, L"MAINMENUWINDOW")) };

		if (!pWnd)
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_HandleFailed);


		auto [language, textsize, filtermodes] {
			pWnd->request_settings<Setting::Language, Setting::TextSize, Setting::FilterModes>()
			};

		auto update_setting{ [&](auto setting, auto value, WmId::Custom id) -> void {
			pWnd->update_settings<decltype(setting)>(value);
			PostMessageW(parent_, (UINT)id, 0, 0);
			update_menu_labels(hMenu_);
		} };

		auto wmId{ static_cast<WmId::MenuBar>(cmd) };

		switch (wmId) {
			case WmId::MenuBar::Language_English: {

				if (language != Language::Type::English)
					update_setting(Setting::Language{}, Language::Type::English, WmId::Custom::Request_Language_Change);

				return 0;
			}
			case WmId::MenuBar::Language_Spanish: {

				if (language != Language::Type::Spanish)
					update_setting(Setting::Language{}, Language::Type::Spanish, WmId::Custom::Request_Language_Change);

				return 0;
			}
			case WmId::MenuBar::Filter_Invalid: [[fallthrough]];
			case WmId::MenuBar::Filter_Blank: [[fallthrough]];
			case WmId::MenuBar::Filter_Comments: [[fallthrough]];
			case WmId::MenuBar::Filter_All: {
				auto bit_shift{ static_cast<std::uint8_t>(WmId::MenuBar::Filter_Invalid) -
					static_cast<std::uint8_t>(wmId) };

				filtermodes ^= (wmId == WmId::MenuBar::Filter_All ? 0b00000111 : (1 << bit_shift));
				update_setting(Setting::FilterModes{}, filtermodes, WmId::Custom::Request_Filter_Change);
				return 0;
			}
			case WmId::MenuBar::Settings_ReloadFile: {
				PostMessageW(parent_, (UINT)WmId::Custom::Request_List_Reload, 0, 0);
				return 0;
			}
			case WmId::MenuBar::Text_IncreaseSize: [[fallthrough]];
			case WmId::MenuBar::Text_DecreaseSize: {
				textsize += wmId == WmId::MenuBar::Text_DecreaseSize ? -1 : 1;
				update_setting(Setting::TextSize{}, textsize, WmId::Custom::Request_TextSize_Change);
				return 0;
			}
			case WmId::MenuBar::File_Exit: {
				PostMessageW(parent_, WM_CLOSE, 0, 0);
				return 0;
			}

			case WmId::MenuBar::File_EmptyFile: {
				PostMessageW(parent_, (UINT)WmId::Custom::Request_Clear_File, 0, 0);
				return 0;
			}
			case WmId::MenuBar::File_BackupFile: {
				PostMessageW(parent_, (UINT)WmId::Custom::Request_Backup_File, 0, 0);
				return 0;
			}
			case WmId::MenuBar::File_RestoreFile: {
				PostMessageW(parent_, (UINT)WmId::Custom::Request_Restore_File, 0, 0);
				return 0;
			}
			default: break;
		}

		return 0;
	}
} // namespace Component.Menubar