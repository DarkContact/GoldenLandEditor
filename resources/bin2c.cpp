#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: bin2c <input> <output_cpp> <variable_name>\n";
        return 1;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in) return 2;

    std::ofstream out(argv[2]);
    if (!out) return 3;

    std::string var_name = argv[3];

    out << "#include <stdint.h>\n\n";
    out << "extern const uint8_t " << var_name << "[] = {\n    ";

    size_t count = 0;
    unsigned char byte;
    while (in.read(reinterpret_cast<char*>(&byte), 1)) {
        if (count > 0) out << ",";
        if (count % 16 == 0 && count > 0) out << "\n    ";

        out << (int)byte; // Пишем в Decimal
        count++;
    }

    out << "\n};\n";
    out << "extern const uint32_t " << var_name << "_size = " << count << ";\n";

    return 0;
}
