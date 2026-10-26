#include "./list_view.hpp"

#include "./localization.hpp"
#include "./win32_utility.hpp"

#include <ranges>
#include <algorithm>

namespace List {

	HWND ListView::hwnd() const {
		return hwnd_;
	}

	ListView& ListView::create(HWND parent) {

		INITCOMMONCONTROLSEX icex{ };
		icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icex.dwICC = ICC_LISTVIEW_CLASSES;

		InitCommonControlsEx(&icex);

		hwnd_ = Win32Utility::create_window({
				.className = WC_LISTVIEW,
				.style = WS_CHILD | WS_VISIBLE | LVS_REPORT
						| LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_EDITLABELS,
				.hParent = parent,
				.id = 10001,
				.hInst = GetModuleHandleW(NULL),
			});

		ListView_SetExtendedListViewStyle(hwnd_,
			LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER
			| LVS_EX_COLUMNSNAPPOINTS | LVS_EX_GRIDLINES
		);

		return *this;
	}

	ListView& ListView::update_filters(std::uint32_t filter_modes) {
		filter_modes_ = filter_modes;
		return *this;
	}

	auto ListView::retrieve_filtered_view() const {

		auto vw{
			line_data_
			| std::views::enumerate
			| std::views::filter([=](const auto& p) {
				const auto& [idx, line] { p };

				if (line.type == FileInternals::Line::Type::DELETED)
					return false;

				if ((filter_modes_ & (1 << 0)) &&
					line.type == FileInternals::Line::Type::INVALID)
					return false;

				if ((filter_modes_ & (1 << 1)) &&
					line.type == FileInternals::Line::Type::BLANK)
					return false;

				if ((filter_modes_ & (1 << 2)) &&
					line.type == FileInternals::Line::Type::COMMENT)
					return false;

				return true;
			})
		};

		return vw;
	}


	ListView& ListView::populate_rows(List::PopulateRows from) {

		if (from == List::PopulateRows::FromFile) {
			rewrite_data();
		}

		auto vw{ retrieve_filtered_view() };
		list_idx_ = 0;

		for (const auto& [l_idx, line] : vw) {

			auto cnt_text{ std::to_wstring(l_idx + 1) };

			LVITEMW lvItem{ };
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = list_idx_++;

			lvItem.iSubItem = 0;
			lvItem.pszText = const_cast<LPWSTR>(cnt_text.c_str());

			auto in_idx{ ListView_InsertItem(hwnd_, &lvItem) };

			lvItem.iItem = in_idx;
			lvItem.iSubItem = 1;
			lvItem.pszText = const_cast<LPWSTR>(line.ip.c_str());
			ListView_SetItem(hwnd_, &lvItem);

			lvItem.iItem = in_idx;
			lvItem.iSubItem = 2;
			lvItem.pszText = const_cast<LPWSTR>(Localization::get_text(line.type).data());
			ListView_SetItem(hwnd_, &lvItem);

			lvItem.iItem = in_idx;
			lvItem.iSubItem = 3;
			lvItem.pszText = const_cast<LPWSTR>(line.domains.c_str());
			ListView_SetItem(hwnd_, &lvItem);
		}

		return *this;
	}


	LRESULT ListView::MessageHandler(LPARAM lParam) {
		LPNMHDR lpmsg{ reinterpret_cast<LPNMHDR>(lParam) };
		
		switch (lpmsg->code) {
			case LVN_BEGINLABELEDIT: return FALSE;
			case LVN_ENDLABELEDIT: {
				NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;

				// pszText will be NULL if the user cancelled editing (e.g., pressed ESC)
				if (pdi->item.pszText != NULL) {
					// Update the internal data structure / item text
					ListView_SetItemText(lpmsg->hwndFrom, pdi->item.iItem, 0, pdi->item.pszText);
					return TRUE; // Return TRUE to accept the change
				}
				return FALSE; // Return FALSE if cancelled
			}
			default: return 0;
		}
	}


	void ListView::rewrite_data() {

		auto data{ FileInternals::load_data(FileInternals::from_target_file()) };

		if (!data) {
			// we should def deal wt this, later tho lmao
			// Deal with the error later
			return;
		}

		line_data_ = std::move(data.value());
	}


	void ListView::refresh_list(List::PopulateRows from) {
		ListView_DeleteAllItems(hwnd_);

		populate_rows(from);

		UpdateWindow(hwnd_);
	}


	void ListView::update_labels() {
		LVCOLUMN lvc = { };
		lvc.mask = LVCF_TEXT;

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Line).data());
		ListView_SetColumn(hwnd_, 0, &lvc);

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Ip).data());
		ListView_SetColumn(hwnd_, 1, &lvc);

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Type).data());
		ListView_SetColumn(hwnd_, 2, &lvc);

		lvc.pszText = const_cast<LPWSTR>(Localization::get_text(Localization::Text::Lv_Domain).data());
		ListView_SetColumn(hwnd_, 3, &lvc);

		auto vw{ retrieve_filtered_view() };

		for (const auto& [idx, line] : vw)
			ListView_SetItemText(hwnd_, idx, 2, const_cast<LPWSTR>(Localization::get_text(line.type).data()));

		UpdateWindow(hwnd_);
	}

	const std::vector<FileInternals::Line::Data>& ListView::data() const {
		return line_data_;
	}
}