export module json:serializer;

import std;
import :value;

export namespace json
{
	template<typename Func>
	concept Output = requires(Func&& func, const std::string& str)
	{
		{ func(str) } -> std::same_as<void>;
	};

	constexpr int indent = 2;

	class Serializer
	{
	public:

		static std::string dump(const Value& value);
		static void dump(const Value& value, std::ostream& output);

		template<typename Output>
		static void serialize(const Value& value, Output&& output, int layer = 0);

	private:

		template<typename Output>
		static void serializeObject(const Value& value, Output&& output, int layer);
		template<typename Output>
		static void serializeArray(const Value& value, Output&& output, int layer);
		template<typename Output>
		static void serializeBasic(const Value& value, Output&& output);
	};

	std::string Serializer::dump(const Value& value)
	{
		std::string result;
		serialize(value, [&result](const std::string& str) { result += str; });
		return result;
	}

	void Serializer::dump(const Value& value, std::ostream& output)
	{
		serialize(value, [&output](const std::string& str) { output << str; });
	}

	template<typename Output>
	void Serializer::serialize(const Value& value, Output&& output, int layer)
	{
		++layer;

		if (value.isType<Object>())
		{
			serializeObject(value, std::forward<Output>(output), layer);
		}
		else if (value.isType<Array>())
		{
			serializeArray(value, std::forward<Output>(output), layer);
		}
		else
		{
			serializeBasic(value, std::forward<Output>(output));
		}

		--layer;
	}

	template<typename Output>
	void Serializer::serializeObject(const Value& value, Output&& output, int layer)
	{
		const Object& obj = value.get<Object>();

		output("{\n");

		std::size_t index = 0;
		std::size_t size = obj.size();

		for (const auto& [key, val] : obj)
		{
			output(std::string(layer * indent, ' ') + "\"" + key + "\": ");
			serialize(val, std::forward<Output>(output), layer);

			if (++index < size)
			{
				output(",\n");
			}
		}

		output("\n" + std::string((layer - 1) * indent, ' ') + "}");
	}

	template<typename Output>
	void Serializer::serializeArray(const Value& value, Output&& output, int layer)
	{
		const Array& arr = value.get<Array>();

		output("[\n");

		for (std::size_t i = 0; i < arr.size(); ++i)
		{
			output(std::string(layer * indent, ' '));
			serialize(arr[i], std::forward<Output>(output), layer);

			if (i + 1 < arr.size())
			{
				output(",\n");
			}
		}

		output("\n" + std::string((layer - 1) * indent, ' ') + "]");
	}

	template<typename Output>
	void Serializer::serializeBasic(const Value& value, Output&& output)
	{
		if (value.isType<Null>())
		{
			output("null");
		}
		else if (value.isType<bool>())
		{
			output(value.get<bool>() ? "true" : "false");
		}
		else if (value.isType<int>())
		{
			output(std::to_string(value.get<int>()));
		}
		else if (value.isType<double>())
		{
			output(std::to_string(value.get<double>()));
		}
		else if (value.isType<std::string>())
		{
			output("\"" + value.get<std::string>() + "\"");
		}
	}
}