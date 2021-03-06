### Задание 5. Кеш

В этом задании мы создаем ядро логики memcached -- класс, который будет хранить LRU-кеш.

LRU -- last recently used, наиболее давно использовавшийся. Под использованием понимаютя операции set и get.

### Задание

Создайте класс для LRU кеша.

Кеш можно реализовать с помощью всего лишь трех операций:

bool get(Key key, Value* value, Time* exp_time, Time* update_time);
void set(Key key, Time exp_time, Value value);
bool delete(Key key);

Все остальные операции memcached могут быть реализованы с использованием этих примитивов.

Реализовать класс Cache с указанным интерфейсом. (Не забыть про exp_time!).
Типы полей Key и Value должны задаваться шаблонными параметрами.
В конструкторе Cache необходимо принимать размер кеша (на ваш выбор, измерять ли его в числе элементов или в объеме потребляемой памяти).
Описание операций, а также асимптотику выполнения операций `set`, `get` и `delete` для Вашей реализации
необходимо разместить в файле README.md в вашем репозитории, отдельным параграфом после описания
проекта.
За достижение асимптотики O(1) на обе операции и корректное доказательство будет +1 дополнительный
балл.

Также необходимо измерить, сколько времени выполняются эти операции.
В тесте нужно создать кеш фиксированного размера, полностью его заполнить различными ключами. Затем измерить, сколько времени выполняются 1000, 10000, 10000 запросов (случайно выбирая различные ключи для запросов). Посчитать, сколько времени тратится на 1 запрос. Заполнить таблицу в readme.

### Семантика операций кеша:

Далее приводится семантика операций кеша *без учета exp_time*.

#### Семантика операции set:
-- положить элемент с ключом и expiration time
 * Если кеш не заполнен, операция `set(key, exp_time, value)` добавляет новый элемент в кеш
 * Если кеш заполнен, операция `set` удаляет из кеша элемент, который там *находится дольше всего*
 (обратите внимание, что это связано не с временем жизни, а со временем, когда элемент был добавлен
  в кеш), а затем добавляет новый элемент в кеш
 * При добавлении нового элемента в кеш, если ключ там уже был, значение в кеше обновляется

#### Семантика операции get:
-- достать элемент с ключом и expiration time
 * Если элемент с таким ключом найден, заполнить поля value, exp_time, update_time и вернуть значение true
 * Если элемент с таким ключом не найден, вернуть false и поля value и time оставить без изменений

#### Семантика операции delete:
 -- удалить элемент с ключом.
 * Если элемент с таким ключом найден, удалить его и вернуть true
 * Если элемент с таким ключом не найден, вернуть false

#### Семантика поля exp_time

Если для ключа задано поле exp_time, не равное 0, то значение должно удалиться через некоторое время
в соответствии со значением этого поля.

Подробнее про интерпретацию числа в поле exp_time:
https://github.com/memcached/memcached/blob/master/doc/protocol.txt#L79

Подробнее про семантику поля exp_time:
https://github.com/memcached/memcached/blob/master/doc/protocol.txt#L163

### Варианты реализации кеша

На семинаре обсуждались следующие варианты реализации кеша:

* hashtable -- асимптотика O(1) на получение значения по ключу и вставку, но непонятно, как удалять
элемент, находящийся там дольше всего (придется сканировать хеш-таблицу)
* balanced binary tree -- асимптотика O(logN) на вставку и удаление по ключу, но непонятно, как
удалять элемент, находящийся дольше всего (если элементы упорядочены по ключу, то они необязательно
упорядочены по времени)
* priority_queue, хранящая время в качестве приоритетов -- O(N) на поиск элемента по ключу, зато
O(1) на поиск элемента, находящегося дольше всего.

Утверждается, что при помощи комбинации некоторых структур данных, можно получить O(1) на все
операции с кешем. Вам предлагается найти и реализовать такую структуру данных.
