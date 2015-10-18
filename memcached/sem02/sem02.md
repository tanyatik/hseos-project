### Задание 2. Протокол

В этом задании требуется реализовать протокол для сервера Memcached.
Реализации семантики команд не требуется.
Задание может быть реализовано независимо от задания 1.

### Описание протокола

С полным описанием протокола необходимо внимательно ознакомиться здесь:

https://github.com/memcached/memcached/blob/master/doc/protocol.txt#L124

Кратко, в текстовом протоколе любая команда, кроме `cas`, может быть записана как:

```
<command> <key> <flags> <exptime> <N>\r\n
<data>\r\n
```

### Классы RBuffer и WBuffer

Для упрощения задачи, Вам дана реализация классов RBuffer и WBuffer.

Классы RBuffer и WBuffer осуществляют буферизированные чтение и запись из потока данных (например, из строки или из сокета).
Внутри они хранят буфер, а также позицию в потоке.
Читают и пишут данные блоками, равными размеру буфера. При исчерпании буфера на чтение/запись, буфер обновляется.

Подробные комментарии и код смотрите в файлах `buffer.cpp` и `buffer.h`.

### Реализация протокола

#### Команды
Для начала стоит определиться с множеством команд, которое будет реализовано.
Допустим, для начала можно ограничиться командами `set`, `add`, `get`, `delete`.

Заведем перечислимый тип, храниящий типы команд:

```
enum MC_COMMAND {
    CMD_SET,
    CMD_ADD,
    CMD_GET,
    CMD_DELETE
};
```

А также структуру для информации о команде

```
struct McCommand {
    MC_COMMAND command = CMD_UNKNOWN;
    std::string key;
    int32_t flags = 0;
    time_t exp_time = 0;
    std::vector<char> data;

    void Deserialize(RBuffer* buffer);
};

MC_COMMAND CommandName2Code(const std::string& param);
```

Функция `CommandName2Code` должна бросать исключение, если команда с данным именем не найдена.

Структура информации о команде должна поддерживать возможность чтения себя из потока ввода (метод `Deserialize`).
Набросок реализации `Deserialize`:

```
void McCommand::Deserialize(RBuffer* buffer) {
    std::string cmd = buffer->ReadField(' '); // читает поле до пробельного символа
    buffer->ReadChar();                       // Читает ' ', и сдвигает текущий символ
    command = CommandName2Code(cmd);

    key = ...
    flags = ...
    exp_time = ...
    int32_t n = ...
    data = ...

    ReadCharCheck('\r'); // бросает исключение, если текущий символ не равен '\r', и сдвигает позицию текущего символа
    ReadCharCheck('\n');
}
```

#### Результат команды

Заведем перечислимый тип, означающий коды завершения команд:

```
enum MC_RESULT_CODE {
    R_STORED,
    R_NOT_STORED,
    R_EXISTS,
    R_NOT_FOUND,
    R_DELETED
};

std::string ResultCode2String(MC_RESULT_CODE code);
```

Но здесь возникает сложность --- команда `get` возвращает не код завершения, а вектор результатов.
Поэтому для возвращаемого значения не получится обойтись одним значением из enum.

Предлагается завести класс со следующим интерфейсом:

```
class McResult {
private:
    enum RESULT_TYPE {
        RT_CODE,
        RT_VALUE,
        RT_ERROR
    } type_;

    MC_RESULT_CODE code_;
    std::vector<McValue> values_;
    std::string error_message_;

public:
    McResult(MC_RESULT_CODE result_code)
        : type_(RT_CODE)
        , code_(result_code)
    {}
    McResult(const std::vector<McValue>& values)
        : type_(RT_VALUE)
        , values_(values)
    {}
    McResult(const std::string& error_message)
        : type_(RT_ERROR)
        , error_message_(error_message)
    {}

    void Serialize(WBuffer* buffer) const;
};
```

Объект класса McResult должен помнить, какой тип в нем лежит -- либо код возврата, либо вектор из
значений типа `McValue`, либо строка с описанием ошибки (но не одновременно!).

Функция `Serialize` должна записать данные в соответствии с протоколом и типом данных, содержащихся в классе.

Набросок реализации:

```
void McResult::Serialize(WBuffer* buffer) {
    switch(type_) {
        case ERROR:
            buffer->WriteField("ERROR", ' ');
            buffer->WriteField(error_message);
            break;
        case ... :
            ...
        default:
            throw ...

        buffer->WriteChar('\r');
        buffer->WriteChar('\n');
    }
}
```

Для типа `McValue`, возвращаемого командой `get`, потребуется отдельный класс:

```
class McValue {
private:
    int key_;
    int flags_;
    std::vector<char> data_block_;

public:
    McValue(std::string key, int flags, const std::vector<char> data_block);
    void Serialize(std::vector<char*>) const;
};
```

Функция `Serialize` должна записать данные в соответствии с протоколом.

### Задание

Реализовать функцию чтения `McCommand` из буфера ввода и функцию записи `McValue` и `McResult` в буфер вывода.

Для чтения и записи использовать данные Вам классы `RBuffer` и `WBuffer`.

```
void McCommand::Deserialize(RBuffer* buffer);

void McValue::Serialize(WBuffer* buffer) const;
void McResult::Serialize(WBuffer* buffer) const;
```

Реализовать также все необходимые конструкторы и вспомогательные функции.

К себе в репозиторий скопировать файлы `buffer.cpp, buffer.h, Makefile` и директорию `contrib` (в ней находится библиотека для юнит-тестирования).

Запустить Makefile. Если ошибок нет, создастся файл `run_tests`.

Запустить `run_tests`, добиться того, чтобы все подготовленные для вас тесты стали зелеными.
