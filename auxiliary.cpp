#include "./auxiliary.hpp"

#include <chrono>
#include <format>

#include <cstdlib>

namespace Auxiliary {

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

	namespace System {
		void flush() {
			std::system("echo off");
			std::system("echo flushing dns...");
			std::system("ipconfig /flushdns");
		}
	}
}
