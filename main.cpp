import json;
import std;

int main()
{
	std::ifstream file{ "test.json", std::ios::in };

	if (file.good())
	{
		try
		{
			std::string jsonString = json::getAllChars(file);
			json::Parser parser{ jsonString };
			json::Value value = parser.parse();
			std::cout << "Parsed JSON successfully!" << std::endl;

			std::string_view str = value["departments"][0]["projects"][0]["deadline"];

			std::cout << "Department name: " << str << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error parsing JSON: " << e.what() << std::endl;
		}
	}
	else
	{
		std::cerr << "Failed to open file." << std::endl;
	}
}