#include "./component.hpp"


namespace Screen {
	void Data::generate(HWND parent) {

		auto& [px, py] { abso_pos_ };
		auto& [clx, cly] { client_size_ };

		RECT ClientRect{ };

		px = GetSystemMetrics(SM_CXSCREEN);
		py = GetSystemMetrics(SM_CYSCREEN);

		if (!GetClientRect(parent, &ClientRect) || px == 0 || py == 0) {
			// pressing no in the prompt will terminate program execution. PostQuitMessage(0) -> std::terminate()
			if (Message::display(Message::Type::Error, Localization::Text::Error_ScreenDimensionFailed) == IDYES) {
				px = py = 0;
				clx = cly = CW_USEDEFAULT;
			}
		}
		else {
			clx = ClientRect.right;
			cly = ClientRect.bottom;
		}
	}

	void Data::regenerate_client_size(std::int32_t width, std::int32_t height) {
		auto& [clx, cly] { client_size_ };

		clx = width;
		cly = height;
	}

	Pipestate operator|(Data& obj, From from) {
		return Pipestate{ .data = obj, .from = from };
	}

	Pipestate operator|(Pipestate obj, Dimension dim) {
		obj.dim = dim;
		return obj;
	}

	int operator|(Pipestate obj, Percentage prc) {

		int dim_data{ };

		auto set_dim{ [&](WindowSize wsz, Dimension dim) -> void {
			if (dim == Dimension::Width)
				dim_data = wsz.x;
			else
				dim_data = wsz.y;
		} };


		if (obj.from == From::Screen)
			set_dim(obj.data.abso_pos_, obj.dim);
		else
			set_dim(obj.data.client_size_, obj.dim);

		return (dim_data * prc.prc_ + 50) / 100;
	}

} //Namespace Screen