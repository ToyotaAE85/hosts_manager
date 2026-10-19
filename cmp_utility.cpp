#include "./component.hpp"

namespace Utility {

	HWND create_window(const CreateWindowData& data) {

		HWND wnd{ CreateWindowExW(data.exStyle,
			data.className.data(), data.windowName.data(), data.style,
			data.x, data.y, data.width, data.height,
			data.hParent, data.id, data.hInst, data.lpParam)
		};

		if (!wnd)
			Message::display(Message::Type::CriticalError, Localization::Text::CriticalError_UIFailed);

		return wnd ? wnd : nullptr;
	}

	HFONT create_font(const CreateFontData& data) {
		HFONT fnt{
			CreateFontW(data.font_height, data.font_width, data.escape_angle, data.orientation,data.font_weight,
				data.italic, data.underline, data.strike_out, data.char_set, data.out_precision, data.clip_precision,
				data.quality, data.pitch_family, data.font_name)
		};

		if (!fnt)
			Message::display(Message::Type::Error, Localization::Text::Error_FontCreationFailed);

		return fnt;
	}

}