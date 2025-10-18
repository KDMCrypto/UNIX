#!/bin/sh -e

# Директория для общего тома и файл блокировки
SHARED_DIR="/data"
LOCK_FILE="${SHARED_DIR}/file_lock"

# Генерация уникального ID контейнера
CONTAINER_ID=$(od -An -N3 -tx1 /dev/urandom | tr -d ' \n')

FILE_COUNTER=0

echo "Container ID: ${CONTAINER_ID}"
echo "Shared Directory: ${SHARED_DIR}"

while true; do
    # Выбираем имя и создаем файл
    NEXT_FILE_NAME=$(flock -x "${LOCK_FILE}" -c '
        i=1
        while :; do
            potential_name=$(printf "%03d" "$i")
            if [ ! -f "'"${SHARED_DIR}"'/$potential_name" ]; then
                # Создаём и сразу записываем в файл содержимое
                printf "%s %d\n" "'"${CONTAINER_ID}"'" "'"${FILE_COUNTER}"'" > "'"${SHARED_DIR}"'/$potential_name"
                printf "%s" "$potential_name"
                exit 0
            fi
            i=$((i + 1))
            [ "$i" -gt 999 ] && exit 1
        done
    ')

    FILE_COUNTER=$((FILE_COUNTER + 1))

    echo "[CREATE] Container: ${CONTAINER_ID} | File: ${NEXT_FILE_NAME} | Counter: ${FILE_COUNTER}"

    sleep 1

    # Удаление созданного файла
    rm -f "${SHARED_DIR}/${NEXT_FILE_NAME}"
    echo "[DELETE] Container: ${CONTAINER_ID} | File: ${NEXT_FILE_NAME}"

    sleep 1
done
