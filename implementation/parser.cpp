module json:parser;

using namespace json;

std::string json::getAllChars(std::istream& input)
{
	return std::string{ std::istreambuf_iterator<char>{ input }, std::istreambuf_iterator<char>{} };
}

Value Parser::parse()
{
	reader_ >> strView_;

	if (strView_ == "{")
	{
		return parseObject();
	}
	else if (strView_ == "[")
	{
		return parseArray();
	}
	else
	{
		return parseBasic();
	}
}

Value Parser::parseObject()
{
	Object obj;

	while (true)
	{
		if (reader_.peek() == '}')
		{
			reader_ >> strView_;
			break;
		}

		reader_ >> str_ >> strView_;

		if (strView_ != ":")
		{
			throw std::runtime_error("Expected ':' after key in JSON object");
		}

		Value& temp = obj[std::move(str_)];
		temp = parse();

		reader_ >> strView_;

		if (strView_ == "}")
		{
			break;
		}
		else if (strView_ != ",")
		{
			throw std::runtime_error("Expected ',' or '}' in JSON object");
		}
		else if (reader_.peek() == '}')
		{
			throw std::runtime_error("Trailing comma in JSON object");
		}
	}

	return Value{ std::move(obj) };
}

Value Parser::parseArray()
{
	Array arr;

	while (true)
	{
		if (reader_.peek() == ']')
		{
			reader_ >> strView_;
			break;
		}

		arr.emplace_back(parse());
		reader_ >> strView_;
		if (strView_ == "]")
		{
			break;
		}
		else if (strView_ != ",")
		{
			throw std::runtime_error("Expected ',' or ']' in JSON array");
		}
		else if (reader_.peek() == ']')
		{
			throw std::runtime_error("Trailing comma in JSON array");
		}
	}

	return Value{ std::move(arr) };
}

Value Parser::parseBasic()
{
	if (strView_.size() != 0)
	{
		if (strView_ == "null")
		{
			return Value{ nullptr };
		}
		else if (strView_ == "true")
		{
			return Value{ true };
		}
		else if (strView_ == "false")
		{
			return Value{ false };
		}
		else
		{
			int i = 0;
			auto result = std::from_chars(strView_.data(), strView_.data() + strView_.size(), i);
			if (result.ec == std::errc())
			{
				if (result.ptr != strView_.data() + strView_.size())
				{
					double d = 0.0;
					result = std::from_chars(strView_.data(), strView_.data() + strView_.size(), d);

					if (result.ec == std::errc() && result.ptr == strView_.data() + strView_.size())
					{
						return Value{ d };
					}
				}
				else
				{
					return Value{ i };
				}
			}

			throw std::runtime_error("Invalid JSON value: " + std::string{ strView_ });
		}
	}
	else
	{
		reader_ >> str_;
		return Value{ std::move(str_) };
	}
}
