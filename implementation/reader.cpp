module json:reader;

using namespace json;

const std::unordered_set<char> Reader::specials{ '{', '}', '[', ']', ',', ':' };
const std::unordered_map<std::string_view, char> Reader::escapes{
	{ "\\\"", '\"' },
	{ "\\\\", '\\' },
	{ "\\/", '\/' },
	{ "\\b", '\b' },
	{ "\\f", '\f' },
	{ "\\n", '\n' },
	{ "\\r", '\r' },
	{ "\\t", '\t' }
};

Reader& Reader::operator>>(std::string& out)
{
	skipWhitespace();

	out.clear();

	if (data_[pos_] == '"')
	{
		++pos_;
		std::size_t start = pos_;

		while (pos_ < data_.size())
		{
			if (data_[pos_] == '\\')
			{
				out.append(data_.substr(start, pos_ - start));

				pos_ += 2;
				start = pos_;

				if (eof())
				{
					throw std::runtime_error("Invalid escape sequence in JSON string");
				}

				out.append(1, escapes.at(data_.substr(pos_ - 2, 2)));
			}
			else if (data_[pos_] == '"')
			{
				out.append(data_.substr(start, pos_ - start));
				++pos_;
				break;
			}
			else if (std::isspace(static_cast<unsigned char>(data_[pos_])) && data_[pos_] != ' ')
			{
				throw std::runtime_error("Unexpected whitespace in JSON string");
			}
			else
			{
				++pos_;
			}
		}
	}
	else
	{
		throw std::runtime_error("Expected '\"' at the beginning of JSON string");
	}

	return *this;
}

Reader& json::Reader::operator>>(std::string_view& out)
{
	skipWhitespace();

	if (specials.contains(data_[pos_]))
	{
		out = data_.substr(pos_++, 1);
	}
	else if (data_[pos_] == '"')
	{
		out = "";
	}
	else
	{
		std::size_t start = pos_;

		while (!eof()
			&& !std::isspace(static_cast<unsigned char>(data_[pos_]))
			&& !specials.contains(data_[pos_]))
		{
			++pos_;
		}
		out = data_.substr(start, pos_ - start);
	}

	return *this;
}

char json::Reader::peek()
{
	skipWhitespace();

	return data_[pos_];
}

void json::Reader::skipWhitespace()
{
	while (!eof()
		&& std::isspace(static_cast<unsigned char>(data_[pos_])))
	{
		++pos_;
	}
}
