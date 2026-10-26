#include "./win32_api.hpp"

#include "./file_internals.hpp"

#include <CommCtrl.h>

#include <cstdint>
#include <string_view>
#include <vector>
#include <concepts>


namespace List {

	enum class PopulateRows {
		FromFile,
		FromMemory
	};

	struct ColumnData {
		std::wstring_view name;
		std::int32_t fmt;
		std::int32_t size;
	};

	class ListView {

		HWND hwnd_{ };
		std::vector<FileInternals::Line::Data> line_data_{ };
		std::uint32_t filter_modes_{ };

		std::uint32_t list_idx_{ };

	public:

		HWND hwnd() const;

		ListView& create(HWND parent);

		template<std::same_as<ColumnData> ...T>
		ListView& populate_columns(T... args) {

			LVCOLUMNW lvc{ };
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;


			int col_counter{ 0 };

			((
				lvc.fmt = args.fmt,
				lvc.iSubItem = col_counter,
				lvc.pszText = const_cast<LPWSTR>(args.name.data()),
				lvc.cx = args.size,

				ListView_InsertColumn(hwnd_, col_counter, &lvc),
				++col_counter
				), ...);

			

			return *this;
		}

		ListView& update_filters(std::uint32_t filter_modes);

		ListView& populate_rows(PopulateRows input);

		LRESULT MessageHandler(LPARAM lParam);

		void refresh_list(PopulateRows from);

		void update_labels();

		const std::vector<FileInternals::Line::Data>& data() const;

	private:

		void rewrite_data();

		auto retrieve_filtered_view() const;

	};
}