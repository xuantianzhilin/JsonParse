import json;
import std;

int main()
{
	std::ifstream inf{ "test.json", std::ios::in };

	if (inf.good())
	{
		try
		{
			std::string jsonString = json::getAllChars(inf);
			inf.close();
			json::Parser parser{ jsonString };
			json::Value value = parser.parse();
			std::cout << "Parsed JSON successfully!" << std::endl;

			std::ofstream ouf{ "output.json", std::ios::out };
			json::Serializer::dump(value, ouf);
			ouf.close();
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