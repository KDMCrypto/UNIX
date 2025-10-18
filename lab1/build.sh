#!/bin/sh

# Проверяем, что имя файла передано как аргумент
if [ $# -eq 0 ]; then
    echo "Error: No input file specified."
    exit 1
fi

INPUT_FILE="$1"
# Проверка на существование и чтение файла
if [ ! -f "$INPUT_FILE" ] || [ ! -r "$INPUT_FILE" ]; then
    echo "Error: Cannot read file '$INPUT_FILE' or it does not exist!"
    exit 2
fi

# Ищем в файле строку с комментарием &Output:
OUTPUT_NAME=$(grep "&Output:" "$INPUT_FILE" | cut -d' ' -f2)

# Проверка на пустоту переменной OUTPUT_NAME
if [ -z "$OUTPUT_NAME" ]; then
    echo "Error: Could not find '&Output:' comment in '$INPUT_FILE'!"
    exit 3
fi

# Создаем временный каталог
TEMP_DIR=$(mktemp -d)
echo "Building in temporary directory: $TEMP_DIR"

# Функция для удаления временной папки при выходе
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TEMP_DIR"
}
# Устанавливаем эту функцию как обработчик сигналов
trap cleanup EXIT INT TERM

cp "$INPUT_FILE" "$TEMP_DIR/"

cd "$TEMP_DIR" || exit 4

case "$INPUT_FILE" in
    *.cpp)
        # Компилируем C++-файл
        if ! g++ -o "$OUTPUT_NAME" "$(basename "$INPUT_FILE")"; then
            echo "Error: Compilation failed!"
            exit 5
        fi
        ;;
    *)
        echo "Error: Unsupported file type!"
        exit 6
        ;;
esac

cp "$OUTPUT_NAME" "$OLDPWD/"

echo "Build successful! Output file: $(pwd)/$OUTPUT_NAME"
