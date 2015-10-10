### Задание 1. Сервер

Книжка про сетевое программирование: http://beej.us/guide/bgnet/translations/bgnet_A4_rus.pdf.

Memcached -- сетевое приложение, обслуживающее клиентов, которые подключаются к определенному порту.

Чтобы создать сервер (программу, которая принимает соединения), необходимо выполнить следующие действия:

* подготовить структуру `addrinfo` (**`getaddrinfo`**)
* создать сокет сервера (слушающий) (**`socket`**)
* связать сокет (**`bind`**)
* слушать сокет (**`listen`**)
* в цикле принимать соединения (**`accept`**)

В результате работы функции `accept` получится дескриптор сокета, с которым можно выполнять операции
ввода/вывода (**`read`**/**`write`** или **`sendv`**/**`recv`**).

### Для сдачи первого задания

1. Написать echo-сервер. При получении запроса он должен вернуть обратно клиенту то же самое, что прочитал.
Порт, который будет слушать сервер, рекомендую задавать аргументом командной строки.
Корректно обработать все ошибки всех системных функций.
2. Запустить сервер, например: `./server 3542` (или `./server`, если вы задаете порт в коде). В другом окне терминала запустить `python test.py 3542`. Убедиться, что последняя команда завершилась успешно.
3. Создать репозиторий на Github (или любой другой системе контроля версий), ссылку прислать на почту tanyatik@yandex.ru . Пожалуйста, указывайте в теме письма [HSEOS].

### getaddrinfo

Функция `getaddrinfo` имеет следующий прототип:

```
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node,
                const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);

```

Параметры этой функции могут принимать множество комбинаций значений.
Более подробную информацию смотрите в справке man или в руководстве [1].

`node` -- в нашем случае, мы передаем флаг AI_PASSIVE (смотрите ниже), так что здесь будет NULL
`service` -- в нашем случае, номер порта
`hints` -- параметры, передаваемые в `getaddrinfo`
`res` -- связный список результатов работы функции

Пример вызова и обработки ошибки:

```
int status;
struct addrinfo hints;
struct addrinfo *servinfo;       // will point to the results
const char[] port = "3249";      // CHANGE ME

memset(&hints, 0, sizeof hints); // make sure the struct is empty
hints.ai_family = AF_INET;       // IPv4
hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
}

// use servinfo

freeaddrinfo(servinfo); // free the linked-list
```

В качестве результата для простоты можно взять первый результат, но важно сохранить указатель на
голову списка, чтобы затем корректно освободить его при помощи `freeaddrinfo`.

### socket

Функция `socket` имеет следующий прототип:

```
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

Для того, чтобы задать ее параметры, мы будем использовать уже заполненную структуру `servinfo`.

Пример вызова:

```
int sd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
```

В случае ошибки `socket` возвращает -1 и устанавливает `errno`.

Теперь в переменной `sd` у нас есть дескриптор сокета, который можем использовать далее.

### bind

Функция `bind` имеет следующий прототип:

```
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

В случае ошибки `bind` возвращает -1 и устанавливает переменную `errno`.

Пример вызова:

```
bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
```

### listen

Функция `listen` имеет следующий прототип:

```
int listen(int sockfd, int backlog);
```

### accept

<!---
Обратите внимание, что эта функция используется только в первой версии нашего сервера.
-->

Функция `accept` имеет следующий прототип:

```
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

`sockfd` -- это дескриптор, который используется для того, чтобы слушать входящие соединения.
`addr` -- это указатель на структуру sockaddr, которая будет заполнена информацией о новом соединении.
`addrlen` -- это размер структуры sockaddr.

Пример вызова:

```
struct sockaddr_storage conn_addr;
addr_size = sizeof conn_addr;
conn_fd = accept(sockfd, (struct sockaddr *)&conn_addr, &addr_size);
```

### read и write

Функции `read` и `write` работают для любых файловых дескрипторов, в том числе для сетевых сокетов.
Они уже были рассмотрены на лекциях и семинарах.
Обратите внимание, что *read и write не обязаны прочитать/записать столько чисел, сколько вы
попросили, могут прочитать меньше!*

Пример использования:

```
int conn_fd = ...
std::vector<char> buffer;

// read all data from socket
int read_bytes;
while ((read_bytes = read(conn_fd, buf, sizeof(buf)) > 0) {
    buffer.insert(buffer.end(), buf, buf + read_bytes);
}

// write all data to socket
int write_bytes;
char* data = buffer.data();
int bytes_to_write = buffer.size();
while ((write_bytes = write(conn_fd, data, bytes_to_write)) > 0) {
    data += write_bytes;
    bytes_to_write -= write_bytes;
}

if (bytes_to_write > 0) {  // not all written, probably socket on the other side closed early
    // work around
}
```


[1]. http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#getaddrinfo
[2]. http://beej.us/guide/bgnet/translations/bgnet_A4_rus.pdf
