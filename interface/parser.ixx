export module json:parser;

import std;
import :value;
import :reader;

export namespace json
{
	std::string getAllChars(std::istream& input);

	class Parser
	{
	public:

		Parser(std::string_view data) : reader_(data) {}

		Value parse();

	private:

		Value parseObject();
		Value parseArray();
		Value parseBasic();

		Reader reader_;
		std::string str_;
		std::string_view strView_;
	};
}