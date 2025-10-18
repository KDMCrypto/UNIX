FROM alpine:latest

# Установка util-linux (содержит flock)
RUN apk update && apk add util-linux

# Создаем директорию для общего тома
WORKDIR /data
# Общий том для данных между контейнерами
VOLUME /data

# Копируем скрипт в образ и делаем его исполняемым
COPY con_man.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/con_man.sh

# Запускаем скрипт
CMD ["/usr/local/bin/con_man.sh"]
