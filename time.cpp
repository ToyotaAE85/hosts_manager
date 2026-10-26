#include "./time.hpp"

#include <chrono>
#include <format>

namespace Time {
	std::wstring stamp() {
		const auto now{ std::chrono::system_clock::now() };
		const auto local{ std::chrono::current_zone()->to_local(now) };

		return std::format(
			L"{:%Y-%m-%d_%H-%M-%S}",
			std::chrono::floor<std::chrono::seconds>(local)
		);
	}
}

