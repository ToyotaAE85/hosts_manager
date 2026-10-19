#pragma once

#include "./localization.hpp"

#include <string>
#include <vector>

#include <expected>
#include <filesystem>

namespace Hosts {

	namespace Line {
		enum class Type {
			IPV4,
			IPV6,
			COMMENT,
			BLANK,
			INVALID
		};

		struct Data {
			std::wstring domains;
			std::wstring ip;
			Type type;
		};
	}

	enum class Error {
		FILE_OPEN_FAILED,
		FILE_INVALID_DATA,
		FILE_EMPTY,
	};

	template <typename T>
	using ReturnType = std::expected<T, Error>;

	ReturnType<std::vector<Line::Data>> load_data();
	//ReturnType<void> save_data(const std::vector<Line::Data>&);
}

template <>
struct Localization::TextMap<Hosts::Line::Type> {
	static constexpr Localization::Text map(Hosts::Line::Type type) noexcept {
		switch (type) {
			case Hosts::Line::Type::IPV4:    return Localization::Text::Ht_Ipv4;
			case Hosts::Line::Type::IPV6:    return Localization::Text::Ht_Ipv6;
			case Hosts::Line::Type::COMMENT: return Localization::Text::Ht_Comment;
			case Hosts::Line::Type::BLANK:   return Localization::Text::Ht_Blank;
			case Hosts::Line::Type::INVALID: return Localization::Text::Ht_Invalid;
			[[unlikely]] default:            return Localization::Text::NO_TEXT;
		}
	}
};