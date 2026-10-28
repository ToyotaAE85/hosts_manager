#include "./file_internals.hpp"

#include <WS2tcpip.h>

#include <fstream>
#include <ranges>
#include <format>
#include <cwctype>
#include <algorithm>
	
FileInternals::Line::Type validate_ip(std::wstring_view input) {

	IN_ADDR protocol_4{ };

	if (InetPtonW(AF_INET, input.data(), &protocol_4) == 1)
		return FileInternals::Line::Type::IPV4;
	
	IN6_ADDR protocol_6{ };

	if (InetPtonW(AF_INET6, input.data(), &protocol_6) == 1)
		return FileInternals::Line::Type::IPV6;

	return FileInternals::Line::Type::INVALID;
}


//something IS wrong here, we always end up with invalid entries, regardless of an actual valid IP
FileInternals::ReturnType<FileInternals::Line::Data> parse_line(const std::wstring& input) {

	// do we really need to materialize them...
	const auto tokens{
		input
		| std::views::split(L' ')
		| std::ranges::to<std::vector<std::wstring>>()
	};

	FileInternals::Line::Data line{ };

	if (std::ranges::all_of(tokens | std::views::join, [](wchar_t c) { return std::iswspace(c); })) {
		line.type = FileInternals::Line::Type::BLANK;
	}
	else {

		const auto& ip_token{ tokens[0] };
		
		auto rvw{
			[](auto&& range) {
				return range
					| std::views::join_with(' ')
					| std::ranges::to<std::wstring>();
			}
		};
		
		if (ip_token.starts_with(L'#')) {
			line.type = FileInternals::Line::Type::COMMENT;
			line.domains = rvw(tokens);
		}
		else {
			line.type = validate_ip(ip_token);
			line.ip = tokens[0];
			line.domains = rvw(tokens | std::views::drop(1));
		}

	}

	return line;
}


FileInternals::ReturnType<std::filesystem::path> to_temp(std::filesystem::path original, const std::vector<FileInternals::Line::Data>& data) {

	auto temp{ original };
	temp.replace_extension(".tmp");

	std::wofstream temp_file { temp };

	if (!temp_file)
		return std::unexpected(FileInternals::ErrorData{ Message::Type::Info,FileInternals::Error::FILE_OPEN_FAILED});

	auto vw{
		data | std::views::filter([](const auto& ln) {
			return ln.type != FileInternals::Line::Type::DELETED;
		})
	};

	for (const auto& line : vw) {
		temp_file << std::format(L"{} {}", line.ip, line.domains) << '\n';

		if (!temp_file) {
			std::filesystem::remove(temp);
			return std::unexpected(FileInternals::ErrorData{ Message::Type::Info, FileInternals::Error::FILE_WRITE_FAILED });
		}
	}

	temp_file.close();

	if (!temp_file) {
		std::filesystem::remove(temp);
		return std::unexpected(FileInternals::ErrorData{ Message::Type::Info, FileInternals::Error::FILE_WRITE_FAILED });
	}

	return temp;
}


namespace FileInternals {

	//something here is wrong... yet i can't find the thing..., maybe it's just me idk

	ReturnType<Line::Data> revalidate_line_data(const Line::Data& data) {
		auto res{ parse_line(std::format(L"{} {}", data.ip, data.domains)) };

		if (!res)
			return std::unexpected(res.error());

		return res.value();
	}

	ReturnType<std::vector<Line::Data>> load_data(std::filesystem::path obj) {

		if (obj == L"") 
			obj = hosts;

		if (!std::filesystem::exists(obj))
			return std::unexpected(ErrorData{ Message::Type::Info, Error::FILE_MISSING });

		std::wifstream load{ obj };

		if (!load)
			return std::unexpected(ErrorData{ Message::Type::Info, Error::FILE_OPEN_FAILED });

		std::vector<Line::Data> host_data{ };
		std::wstring line{ };

		while (std::getline(load, line)) {
			auto data{ parse_line(line) };

			if (!data)
				return std::unexpected(data.error());

			host_data.push_back(std::move(data.value()));
		}

		return host_data;
	}


	ReturnType<void> save_data(const std::vector<Line::Data>& data) {

		auto temp{ to_temp(hosts, data) };

		if (!temp)
			return std::unexpected(temp.error());


		std::error_code ec{ };
		std::filesystem::rename(temp.value(), hosts, ec);


		if (ec) {
			std::filesystem::remove(temp.value());
			return std::unexpected(ErrorData{Message::Type::Info, Error::FILE_WRITE_FAILED });
		}

		return {};
	}

}