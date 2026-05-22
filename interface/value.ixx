export module json:value;

import std;

export namespace json
{
	class Value;

	using Null = std::nullptr_t;
	using Array = std::vector<Value>;
	using Object = std::map<std::string, Value>;

	template<typename T>
	concept Type = std::same_as<T, Null>
		|| std::same_as<T, bool>
		|| std::same_as<T, int>
		|| std::same_as<T, double>
		|| std::same_as<T, std::string>
		|| std::same_as<T, Array>
		|| std::same_as<T, Object>;

	class Value
	{
	public:

		Value() : value_(nullptr) {}
		Value(Null) : value_(nullptr) {}
		Value(bool b) : value_(b) {}
		Value(int i) : value_(i) {}
		Value(double d) : value_(d) {}
		Value(std::string&& s) : value_(std::move(s)) {}
		Value(std::string_view s) : value_(std::string(s)) {}
		Value(const Array& arr) : value_(arr) {}
		Value(Array&& arr) : value_(std::move(arr)) {}
		Value(const Object& obj) : value_(obj) {}
		Value(Object&& obj) : value_(std::move(obj)) {}

		operator Null() const { return std::get<Null>(value_); }
		operator bool() const { return std::get<bool>(value_); }
		operator int() const { return std::get<int>(value_); }
		operator double() const { return std::get<double>(value_); }
		operator const std::string&() const { return std::get<std::string>(value_); }
		operator std::string& () { return std::get<std::string>(value_); }
		operator std::string_view() const { return std::get<std::string>(value_); }

		Value& operator[](const char* key);
		const Value& operator[](const char* key) const;
		Value& operator[](const std::string& key);
		const Value& operator[](const std::string& key) const;
		Value& operator[](int index);
		const Value& operator[](int index) const;

		template<Type T>
		const T& get() const { return std::get<T>(value_); }
		template<Type T>
		T& get() { return std::get<T>(value_); }

	private:

		std::variant<Null, bool, int, double, std::string, Array, Object> value_;
	};
}