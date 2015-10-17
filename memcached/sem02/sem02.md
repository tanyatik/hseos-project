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

### Задание

Написать функции

Функция `ReadCommand` должна принимать данные в формате, описанном в протоколе, и возвращать ответ тоже по
протоколу.
```
std::vector<char> ProcessMessage(const std::vector<char>& input);
```

Функция `ProcessCommand` на текущем этапе выполнения проекта должна обрабатывать код команды и
возвращать фиктивный ответ.
Для команды `get` нужно возвращать две записи фиктивных данных.

```
McResult ProcessCommand(const McCommand& command);
```

*Примечание.* Можно придумать другой дизайн -- это один из возможных вариантов.

### Один из способов реализации задания.

Для начала стоит определиться с множеством команд, которое будет реализовано.
Допустим, для начала можно ограничиться командами `set`, `add`, `get`, `delete`.

Заведем перечислимый тип, храниящий типы команд:

```
enum MC_COMMANDS {
    UNKNOWN,
    SET,
    ADD,
    GET,
    DELETE,
    MC_COMMANDS_NUMBER  // always goes last
};
```

А также структуру для информации о команде

```
struct McCommand {
    MC_COMMAND command_;
    std::string key_;
    int32_t flag_;
    time_t exp_time_;
    std::vector<char> data_;
};
```

Удобно завести перечислимый тип, означающий коды завершения команд:

```
enum MC_RESULT_CODE {
    STORED,
    NOT_STORED,
    EXISTS,
    NOT_FOUND
};
```

Но здесь возникает сложность --- команда `get` возвращает не код завершения, а вектор результатов.
Поэтому для возвращаемого значения не получится обойтись одним значением из enum.

Предлагается завести класс со следующим интерфейсом:

```
class McResult {
private:
    // ...
public:
    McResult(MC_RESULT_CODE result_code);
    McResult(const std::vector<McValue>& values);

    void Serialize(std::vector<char>*) const;
};
```

Для типа `McValue`, возвращаемого командой `get`, потребуется отдельный класс:

```
class McValue {
private:
    int key_;
    int flags_;
    std::vector<char> data_block_;
public:
    void Serialize(std::vector<char*>) const;
};
```

Объект класса McResult должен помнить, какой тип в нем лежит -- либо код возврата, либо вектор из
значений типа `McValue` (но не одновременно!).
