#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: bin2c <input> <output_cpp> <variable_name>\n";
        return 1;
    }

    // Используем стандартные потоки с отключенной синхронизацией для скорости
    std::ios_base::sync_with_stdio(false);

    std::ifstream in(argv[1], std::ios::binary);
    if (!in) return 2;

    std::ofstream out(argv[2]);
    if (!out) return 3;

    const std::string var_name = argv[3];

    out << "#include <stdint.h>\n\n";
    out << "extern const uint8_t " << var_name << "[] = {\n  ";

    const size_t CHUNK_SIZE = 4096;
    char buffer[CHUNK_SIZE];

    size_t total_count = 0;
    bool first_byte = true;

    while (in) {
        in.read(buffer, CHUNK_SIZE);
        std::streamsize bytes_read = in.gcount();

        for (std::streamsize i = 0; i < bytes_read; ++i) {
            if (!first_byte) {
                out << ",";
                // Перенос строки для стабильности IDE и компиляторов
                if (total_count % 64 == 0) {
                    out << "\n  ";
                }
            }

            // static_cast через unsigned char важен, чтобы байт 255 не стал "-1"
            out << static_cast<int>(static_cast<unsigned char>(buffer[i]));

            first_byte = false;
            total_count++;
        }
    }

    out << "\n};\n";
    out << "extern const uint32_t " << var_name << "_size = " << static_cast<uint32_t>(total_count) << ";\n";

    return 0;
}
