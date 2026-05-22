export module json:reader;

import std;

export namespace json
{
	class Reader
	{
	public:

		Reader(std::string_view data) : data_(data) , pos_(0) {}

		Reader& operator>>(std::string& out);
		Reader& operator>>(std::string_view& out);

		char peek();

		bool eof() const { return pos_ >= data_.size(); }
		void skipWhitespace();

	private:

		std::string_view data_;
		std::size_t pos_;

		static const std::unordered_set<char> specials;
		static const std::unordered_map<std::string_view, char> escapes;
	};
}