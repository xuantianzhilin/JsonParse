module json:value;

using namespace json;

Value& json::Value::operator[](const char* key)
{
	return operator[](std::string{ key });
}

const Value& json::Value::operator[](const char* key) const
{
	return operator[](std::string{ key });
}

Value& Value::operator[](const std::string& key)
{
	return const_cast<Value&>(static_cast<const Value&>(*this)[key]);
}

const Value& json::Value::operator[](const std::string& key) const
{
	if (auto objPtr = std::get_if<Object>(&value_))
	{
		return objPtr->at(key);
	}
	else
	{
		throw std::runtime_error("Value is not an object");
	}
}

Value& Value::operator[](int index)
{
	return const_cast<Value&>(static_cast<const Value&>(*this)[index]);
}

const Value& json::Value::operator[](int index) const
{
	if (auto arrPtr = std::get_if<Array>(&value_))
	{
		return arrPtr->at(index);
	}
	else
	{
		throw std::runtime_error("Value is not an array");
	}
}
