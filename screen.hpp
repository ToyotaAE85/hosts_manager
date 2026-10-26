#pragma once

#include "./win32_api.hpp"
#include <cstdint>


namespace Screen {

	struct Percentage {
		std::int32_t prc_;
		explicit Percentage(std::int32_t prc) noexcept : prc_{ prc } {}
	};

	struct WindowSize {
		std::int32_t x;
		std::int32_t y;
	};

	enum class From {
		Screen,
		Window,
	};

	enum class Dimension {
		Width,
		Height,
	};

	struct Data {

		WindowSize abso_pos_;
		WindowSize client_size_;

		void generate(HWND parent);

		void regenerate_client_size(std::int32_t width, std::int32_t height);
	};


	struct Pipestate {
		const Data& data;
		From from;
		Dimension dim{ Dimension::Width };
	};


	Pipestate operator|(Data& obj, From from);

	Pipestate operator|(Pipestate obj, Dimension dim);

	int operator|(Pipestate obj, Percentage prc);

}