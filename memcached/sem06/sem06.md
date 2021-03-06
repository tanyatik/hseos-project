### Задание 6. Сервер, принимающий множество подключений

На этом этапе у нас уже есть сервер, умеющий принимать одновременно одно подключение.
В реальности сервер memcached должен обслуживать множество подключений одновременно.

Существует несколько способов, как это сделать. Я здесь буду рассматривать два -- создание потока на
каждое соединение и ThreadPool.

### Задание

1. Реализовать одновременную обработку множества соединений одним из описанных способов
2. Добавить интеграционный тест, в котором осуществлять одновременное подключение к серверу от
   множества клиентов. Смотри ниже объяснение.
3. (на **дополнительный +1 балл**) Реализовать остановку сервера.

### Потоки

Каждый процесс в системе имеет один или несколько потоков (нитей, thread-ов).

Поток -- единица обработки, исполнение ее назначается ядром операционной системы (а не
        программистом). Внутри процесса потоки совместно используют память, в отличие от процессов.
Также они разделяют код и его контекст.

Многопоточность быстрее работает на машинах, на которых есть несколько ядер процессора, так как
позволяет задействовать все ядра.

Для многопоточности можно использовать `pthread` и `std::thread`.

`pthread` это POSIX-библиотека для работы с потоками.
`std::thread` это C++-11 библиотека (дальше пойдут примеры с ее использованием).

#### Пример с race condition

Приведу простой и *неправильный* пример использования потоков.

```
#include <thread>
#include <vector>
#include <iostream>
#include <set>

int main() {
    std::vector<std::thread> threads;
    std::set<int> v;

    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread([&v, i](){
            v.insert(i);
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All threads finished" << std::endl;
    for (int x : v) {
        std::cout << x << std::endl;
    }

    return 0;
}
```

Поток создается в конструкторе std::thread. Единственный аргумент этого конструктора -- это *некий
объект*, который может быть вызван без параметров. Например, это может быть функция без параметров, std::function, результат std::bind и так далее.
В данном примере в `thread` передается lambda-функция. Здесь я не буду рассказывать про
lambda-функции, подробнее можно прочитать здесь
http://stackoverflow.com/questions/7627098/what-is-a-lambda-expression-in-c11 .

Для каждого потока нужно вызвать либо `join`, либо `detach`.
Если не позвать ни то ни другое к моменту уничтожения *объекта std::thread* (обратите внимание, что
        это далеко не то же самое, что завершение работы потока), программа завершится вызовом
std::terminate.
Вызов метода `join` блокируется до завершения потока, для которого вызван этот метод. Вызов метода
`detach` отсоединяется от потока, поэтому поток продолжит работу даже после вызова деструктора
объекта `std::thread`.

**Вопрос**. Что произойдет, если в примере выше заменить вызов `t.join` на вызов `t.detach`?

Скомпилируйте и запустите этот пример. 10 запусков должно хватить, чтобы программа выдала
`segmentation fault`.

В чем причина?

Причина в том, что доступ к переменной `std::set<int> v` производится одновременно из нескольких
потоков.
Представим, что два потока делают вставку в дерево одновременно.
Вставка в дерево элемента `x` состоит из следующих шагов.
1. Найти позицию для вставки элемента.
Допустим, мы добавляем `x` в качестве правого сына для `y`, и правым сыном нового элемента будет
текущий правый сын `y`.
2. Сохраняем текущего правого сына для `y` в переменную `z`.
3. Поставить указатель правого сына `x` на `z`.
4. Поставить указатель на правого сына в `y` на `x`.

Представим, что вставку в дерево осуществляют два потока.
Например, она может происходить так:

1. Thread 1 выполнил шаг 1
2. Thread 2 выполнил шаг 1
3. Thread 1 выполнил шаг 2
3. Thread 1 выполнил шаг 3
3. Thread 1 выполнил шаг 4
4. Thread 2 выполнил шаг 3
4. Thread 2 выполнил шаг 4

Структура дерева придет в очевидно неконсистентное состояние, при котором `x` ссылается сам на себя
(проверьте!).

Возникло состояние гонки (race condition). Так называется любая ситуация, исход которой зависит от
порядка выполнения операций в разных потоках. Это не обязательно плохо само по себе, но может
вызвать проблему, в случае если нарушается *инвариант* структуры данных (как в приведенном выше
        примере). Под гонкой обычно понимают именно проблематичную гонку.

Существует несколько способов избежать состояния гонки.
Самый простой -- снабдить структуру данных защитным механизмом, позволяющим выполнять модификацию
разделяемых данных только одним потоком одновременно.
Другой способ -- использовать атомарные изменения данных, не нарушающие инварианты.

#### Атомарные переменные

Существуют типы данных, модификации которых выполняются атомарно. Это означает, что операции
неделимы: ни из одного потока в системе невозможно увидеть, чтобы операция была выполнена
наполовину. Операция либо выполнена целиком, либо не выполнена вовсе.

Атомарные типы в C++ являются инстанцированием шаблона `std::atomic<T>`. Вы не можете подставить
туда пользовательский тип, но можете использовать шаблон для многих стандартных типов данных.

Пример использования: `std::atomic<int>` для счетчика, `std::atomic<bool>` для флага.

#### Mutex

Слово `mutex` происходит от `mutual execution`. Mutex -- это примитив, который позволяет выполнять
взаимное исключение: если один поток начинает выполнять участок кода, все остальные потоки должны
ждать.
Mutex может быть захвачен (lock) и освобожден (unlock). Библиотека Thread library гарантирует, что
если один поток захватил некоторый mutex, то другие потоки не смогут захватить тот же  mutex, пока
он не будет освобожден.

В примере про вставку в дерево поток, прежде чем совершать вставку, должен вызвать `lock` у mutex,
    который защищает эту структуру данных. После завершения работы с `mutex` поток должен вызвать
    `unlock`, чтобы освободить mutex и дать другим потокам возможность выполнить тот же участок
    кода.

#### Lock guard

Очень важно не забыть вызвать `unlock` для `mutex`, если вы уже вызвали `lock`. Для того, чтобы это
не забыть, хорошо подходит идиома RAII. Согласно этой идиоме, некоторый объект захватывает mutex в
конструкторе, освобождая его в деструкторе, что гарантирует, что `unlock` наверняка будет вызван.

Эту идиому для mutex реализует объект `std::lock_guard`.

С учетом вышесказанного, перепишем пример следующим образом:

```
#include <thread>
#include <vector>
#include <iostream>
#include <set>

int main() {
    std::vector<std::thread> threads;
    std::set<int> v;
    std::mutex m;

    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread([&v, i](){
            std::lock_guard<std::mutex> lock_guard(m);
            v.insert(i);
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << "All threads finished" << std::endl;
    for (int x : v) {
        std::cout << x << std::endl;
    }

    return 0;
}
```

Теперь доступ к `v` защищен с помощью mutex m, который захватывается в конструкторе lock_guard и
освобождается в деструкторе lock_guard.

#### Deadlock (взаимоблокировка)

Представьте игрушку, состоящую из двух частей -- барабан и палочки. А еще представьте двух детей,
    которые любят побарабанить и очень хотят найти в ящике для игрушек барабан и палочки (лежащие
            отдельно). Оба захотели поиграть одновременно и полезли в ящик искать эти предметы, но
    один нашел барабан, а другой -- палочки. Они оказались в тупике, потому что ни один не хочет
    уступить другому, но ни один не может начать играть без отсутствующего у него предмета.

В многопоточном программировании может возникнуть абсолютно та же ситуация -- в случае, если в
каждом потоке для выполнения некоторой операции требуется захватить два mutex одновременно. Если
сложилось так, что один из потоков захватил mutex A, а другой поток захватил mutex B, то ни один
поток не может продолжить выполнение -- возникла взаимоблокировка.

Общая рекомендация, как избежать взаимоблокировок, состоит в том, чтобы захватывать mutex всегда в
одном и том же порядке -- в приведенном примере, вначале mutex A, а потом mutex B. Однако это не
всегда возможно.


Ниже приводится список задач, которые можно порешать для тренировки в многопоточном
программировании.

**Задача**. Спроектируйте интерфейс и реализуйте класс `ThreadsafeStack`. Пользователь класса должен
иметь возможность доступа из нескольких потоков к методам `push`, `pop`, `top` и `empty`.
Как поменяется интерфейс класса по сравнению с интерфейсом `std::stack` и почему?

**Задача**. Решите задачу про "обедающих философов" на C++.

### Методы реализации множества одновременных подключений для сервера

Таких методов существует три. Ниже они рассканы в порядке увеличения максимального RPS (количество
        запросов, которое сервер может обработать за секунду) и в порядке увеличения сложности.

#### Создание потока на каждое соединение

При этом способе для каждого нового соединения создается отдельный поток, в котором обрабатывается
соединение.
Результат обработки соединения отправляется обратно в сокет, вызывающему потоку он не нужен.
Но для корректной остановки сервера и завершения его работы необходимо дожидаться завершения каждого
потока с помощью метода `join()`.
Создавать поток удобно в обработчике соединения.

Примерный псевдокод:

Что было:

```
while (true) {
    accept_fd = accept(...);
    memcached->ProcessConnection(accept_fd);
}
```

Что стало:

```
while (true) {
    accept_fd = accept(...);
    threads.push_back(std::thread([&memcached, accept_fd]() {
        memcached->ProcessConnection(accept_fd);
    });
}

for (const auto& t : threads) {
    t.join();
}
```


#### Использование ThreadPool.

Этот способ сложнее. За грамотную реализацию ThreadPool без race condition будет даваться +2 балла.

Идея ThreadPool состоит в следующем.
В начале работы программы создается множество потоков фиксированного размера (лучше всего -- столько
        потоков, сколько ядер у процессора (- 1, потому что есть еще поток для сервера)).
Также ThreadPool содержит в себе очередь задач, которые необходимо выполнить.

При обработке нового соединения создается задача на обработку соединения в ThreadPool. Больше при
обработке соединения с точки зрения сервера ничего не происходит.

Каждый поток в ThreadPool в цикле делает следующее:
-- получает новую задачу из очереди задач
-- выполняет эту задачу

Задачу удобно представить как объект `std::function` без параметров, тогда выполнение задачи будет
заключаться в том, чтобы вызвать этот объект.

Очередь задач должна поддерживать доступ из нескольких потоков, поэтому ее данные должны быть
защищены.


#### Получение значения из задачи

В сервере Memcached результата обработки соединения нет -- данные просто пишутся обратно в сокет.
Бывают ситуации, когда в результате обработки соединения нужно вернуть какое-то значение. Однако,
    поскольку процесс выполняется параллельно, не совсем ясно, как получать значение, и в какой
    момент. Существует удобный механизм, называемый `std::future`, созданный для получения значения.

Есть несколько способов получить `std::future` -- с помощью `std::packaged_task`, `std::async` и
`std::promise` -- вам нужен первый способ. С помощью этого класса реализуйте возврат значения из
задачи, запущенной в ThreadPool.

### Остановка сервера

Для сервера необходимо предусмотреть возможность остановки при вызове метода `stop`. Для этого
можно, например, завести атомарную переменную, которую выставлять в `true` при вызове этого метода,
    а в цикле перед тем, как повиснуть на соединении, проверять, не выставлена ли она в `true`.
Для того, чтобы это сделать, необходимо задать timeout на операцию ожидания и принятия соединения
(иначе сервер навечно зависнет на вызове accept). Для этого предлагается заменить системный вызов `accept` на системный вызов `poll`.
Необходимо заполнить массив структур pollfd (в нашем случае, в массиве будет 1 элемент), записав
туда серверный сокет и необходимые действия, а также выставив timeout.

### Тест

В этом задании обязательным требованием является написание теста, в котором множество клиентво
выполняют одновременные подключения.
В python это можно сделать при помощи класса Pool модуля multithreading.
Посмотрите на пример https://docs.python.org/2/library/multiprocessing.html#using-a-pool-of-workers
. Вам необходимо сделать нечто похожее.

Псевдокод:

```
# Функция, выполняющая подключение (было раньше)
def connect(port, message):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", port))
    s.send(message)
    return s.recv(4096)

# То, что было раньше (у вас может оказаться по-другому)
def test_single_connection():
    port = int(sys.argv[1])
    mc = subprocess.Popen(['./server', sys.argv[1]])
    time.sleep(1)
    message = "..." # TODO fill
    res = connect(message)
    # assert(...)
    mc.kill()

# Новый тест
def test_multiple_connections():
    port = int(sys.argv[1])
    mc = subprocess.Popen(['./server', sys.argv[1]])
    time.sleep(1)

    message = "..." # TODO fill
    expected_answer = "..." TODO fill
    pool = Pool(processes=100)              # start 100 worker processes
    result = pool.apply_async(connect, [port, message], callback=lambda x: x == expected_answer)    # evaluate asynchronously
    result.wait()
    mc.kill()

```
