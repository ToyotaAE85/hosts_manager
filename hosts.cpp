#include "./hosts.hpp"

#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>

#include <fstream>
#include <ranges>



Hosts::Line::Type validate_ip(std::wstring_view input) {

	IN_ADDR protocol_4{ };

	if (InetPtonW(AF_INET, input.data(), &protocol_4) == 1)
		return Hosts::Line::Type::IPV4;
	
	IN6_ADDR protocol_6{ };

	if (InetPtonW(AF_INET6, input.data(), &protocol_6) == 1)
		return Hosts::Line::Type::IPV6;

	return Hosts::Line::Type::INVALID;
}


Hosts::ReturnType<Hosts::Line::Data> parse_line(const std::wstring& input) {

	const auto tokens{
		input
		| std::views::split(' ')
		| std::ranges::to<std::vector<std::wstring>>()
	};

	Hosts::Line::Data line{ };

	if (tokens.empty()) {
		line.type = Hosts::Line::Type::BLANK;
		return line;
	}

	const auto& ip_token{ tokens[0] };

	if (ip_token.starts_with(L'#'))
		line.type = Hosts::Line::Type::COMMENT;
	else
		line.type = validate_ip(ip_token);

	line.ip = tokens[0];
	line.domains = tokens | std::views::drop(1) | std::views::join_with(' ') | std::ranges::to<std::wstring>();

	return line;
}


namespace Hosts {

	ReturnType<std::vector<Line::Data>> load_data() {

		std::wifstream file{ LR"(C:\Windows\System32\drivers\etc\hosts)" };

		if (!file)
			return std::unexpected(Error::FILE_OPEN_FAILED);

		std::vector<Line::Data> hosts{ };
		std::wstring line{ };

		while (std::getline(file, line)) {
			auto data{ parse_line(line) };

			if (!data)
				return std::unexpected(data.error());

			hosts.push_back(std::move(data.value()));
		}

		return hosts;
	}

}