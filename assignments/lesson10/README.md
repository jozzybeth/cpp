# Урок 10: Многопоточность и параллелизм в C++

#### 1. Еще раз о многопоточности и параллелизме
   - **Многопоточность** — это способность программы выполнять несколько потоков одновременно, позволяя ей работать более эффективно и использовать преимущества многоядерных процессоров.
   - **Параллелизм** — это выполнение нескольких задач одновременно на разных ядрах процессора.
   - Преимущества многопоточности:
      - Увеличение производительности на многоядерных системах.
      - Более отзывчивый интерфейс в приложениях.
   - Недостатки:
      - Сложности в отладке.
      - Необходимость управления доступом к общим данным (например, с помощью мьютексов).

#### 2. Создание и управление потоками
   - В C++ для работы с потоками используется библиотека `<thread>`.
   - Основные компоненты:
     - `std::thread` — класс для создания и управления потоками.
     - `std::this_thread` — предоставляет функции для работы с текущим потоком.
   - **Создание потока:**
     ```cpp
     #include <thread>
     #include <iostream>

     void print_message() {
         std::cout << "Hello from thread!" << std::endl;
     }

     int main() {
         std::thread t(print_message); // Создаем поток, который выполняет функцию print_message
         t.join(); // Ожидаем завершения потока
         return 0;
     }
     ```
   - Методы управления потоком:
     - `join()` — присоединяет поток к основному, блокируя его до завершения.
     - `detach()` — отделяет поток, позволяя ему работать независимо от основного.

#### 3. Синхронизация потоков: `std::mutex` и `std::lock_guard`
   - **Проблема гонок данных (data race):**
      - Возникает, когда несколько потоков одновременно изменяют общие данные, что может привести к некорректным результатам.
   - **Решение** — использовать синхронизацию, чтобы управлять доступом к общим ресурсам.
   - **`std::mutex`:**
      - Мьютекс (mutex) — это механизм синхронизации, который используется для защиты критических секций кода.
      - Основные методы:
        - `lock()` — блокирует мьютекс.
        - `unlock()` — разблокирует мьютекс.
      - Пример:
        ```cpp
        #include <iostream>
        #include <thread>
        #include <mutex>

        int counter = 0;
        std::mutex mtx;

        void increment() {
            for (int i = 0; i < 100; ++i) {
                mtx.lock();
                ++counter;
                mtx.unlock();
            }
        }

        int main() {
            std::thread t1(increment);
            std::thread t2(increment);

            t1.join();
            t2.join();

            std::cout << "Counter: " << counter << std::endl;
            return 0;
        }
        ```
   - **`std::lock_guard`:**
      - Автоматически блокирует мьютекс в начале области видимости и разблокирует при выходе из области.
      - Рекомендуется использовать для исключения ошибок разблокировки.
      - Пример:
        ```cpp
        #include <iostream>
        #include <thread>
        #include <mutex>

        int counter = 0;
        std::mutex mtx;

        void increment() {
            for (int i = 0; i < 100; ++i) {
                std::lock_guard<std::mutex> guard(mtx);
                ++counter;
            }
        }

        int main() {
            std::thread t1(increment);
            std::thread t2(increment);

            t1.join();
            t2.join();

            std::cout << "Counter: " << counter << std::endl;
            return 0;
        }
        ```

### Примеры многопоточных программ

1. **Счетчик с мьютексом**
   - Программа создает два потока, которые инкрементируют общий счетчик.
   - Мьютекс предотвращает одновременный доступ к переменной `counter`.
   - Пример был рассмотрен выше.

2. **Пример с банком (управление балансом)**
   - Пример работы с параллельным доступом к банковскому счету.
   - Код:
     ```cpp
     #include <iostream>
     #include <thread>
     #include <mutex>

     class BankAccount {
     private:
         int balance;
         std::mutex mtx;

     public:
         BankAccount() : balance(0) {}

         void deposit(int amount) {
             std::lock_guard<std::mutex> guard(mtx);
             balance += amount;
             std::cout << "Deposited " << amount << ", Balance: " << balance << std::endl;
         }

         void withdraw(int amount) {
             std::lock_guard<std::mutex> guard(mtx);
             if (balance >= amount) {
                 balance -= amount;
                 std::cout << "Withdrew " << amount << ", Balance: " << balance << std::endl;
             } else {
                 std::cout << "Insufficient funds!" << std::endl;
             }
         }
     };

     void deposit_task(BankAccount& account) {
         for (int i = 0; i < 10; ++i) {
             account.deposit(10);
         }
     }

     void withdraw_task(BankAccount& account) {
         for (int i = 0; i < 10; ++i) {
             account.withdraw(5);
         }
     }

     int main() {
         BankAccount account;

         std::thread t1(deposit_task, std::ref(account));
         std::thread t2(withdraw_task, std::ref(account));

         t1.join();
         t2.join();

         return 0;
     }
     ```

# Дамошнее задание: Улучшение многопоточности и взаимодействия змей

---

### Основные задачи и подходы

1. **Создание потоков для препятствий и бонусов**:
   - **Задача**: реализовать потоки для генерации препятствий и бонусных объектов, которые будут периодически появляться и исчезать на игровом поле.
   - **Реализация**: создадим отдельные потоки, которые через интервалы времени будут добавлять новые объекты на игровое поле. Для случайного размещения и таймингов используем функции `std::this_thread::sleep_for` и генератор случайных чисел.

2. **Логика взаимодействия со змейками**:
   - **Задача**: настроить логику, чтобы змейки могли собирать бонусы и завершать игру при столкновении с препятствием.
   - **Реализация**: добавим функции обработки взаимодействия, которые будут учитывать координаты бонусов, препятствий и змей.

3. **Дополнительная логика управления для второй змейки**:
   - **Задача**: если вторая змейка управляется автоматически, добавить реакцию на бонусные объекты и избегание препятствий.
   - **Реализация**: в алгоритме управления автоматической змейкой добавим проверку на наличие бонусов и препятствий. Например, если бонус находится на видимом расстоянии, змея стремится к нему, а если препятствие — наоборот, отклоняется от курса.

4. **Синхронизация потоков**:
   - **Задача**: использовать `std::mutex` и `std::lock_guard` для корректного доступа к игровым данным и предотвратить коллизии при доступе к общим ресурсам.
   - **Реализация**: для каждого потока, взаимодействующего с игровым полем (змеи, препятствия, бонусы), реализуем блокировку мьютексами в критических секциях кода, например, при добавлении нового препятствия или изменении позиции змеи.

5. **Обработка коллизий с учетом новых элементов**:
   - **Задача**: усовершенствовать логику коллизий, чтобы правильно обрабатывать столкновения змей с препятствиями и сбор бонусов.
   - **Реализация**: обновим функции обработки коллизий с проверками на совпадение координат змей с препятствиями и бонусами.

6. **Оптимизация потоков для плавной работы**:
   - **Задача**: обеспечить работу игры без зависаний и лагов.
   - **Реализация**: убедимся, что каждый поток выполняется без долгих блокировок, оптимизируем обновление игрового поля, чтобы не задерживать основные функции игры.

---

### Пошаговое руководство по реализации

#### Шаг 1: Создание потоков для препятствий и бонусов

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>

// Глобальные данные
std::vector<std::pair<int, int>> obstacles; // координаты препятствий
std::vector<std::pair<int, int>> bonuses;   // координаты бонусов
std::mutex game_mutex;                       // мьютекс для синхронизации доступа к полю
std::condition_variable cv;                  // условная переменная для уведомления об обновлении поля

// Генератор случайных координат
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(1, 20); // размер игрового поля 20x20

// Функция потока для обновления препятствий
void obstacle_thread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        std::lock_guard<std::mutex> lock(game_mutex);

        obstacles.clear();
        for (int i = 0; i < 5; ++i) {
            obstacles.push_back({dist(gen), dist(gen)});
        }
        cv.notify_all(); // Уведомляем другие потоки об изменении
    }
}

// Функция потока для обновления бонусов
void bonus_thread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        std::lock_guard<std::mutex> lock(game_mutex);

        bonuses.clear();
        bonuses.push_back({dist(gen), dist(gen)});
        cv.notify_all(); // Уведомляем другие потоки об изменении
    }
}
```

#### Шаг 2: Логика взаимодействия со змейками

- При проверке столкновения змейки с препятствиями и бонусами используем замок `std::lock_guard`, чтобы избежать несогласованного доступа к ресурсам.

```cpp
void check_collision_with_obstacles(int snake_x, int snake_y, bool& game_over) {
    std::lock_guard<std::mutex> lock(game_mutex);
    for (auto& obstacle : obstacles) {
        if (obstacle.first == snake_x && obstacle.second == snake_y) {
            game_over = true; // столкновение с препятствием
        }
    }
}

void check_collision_with_bonuses(int snake_x, int snake_y, int& snake_length) {
    std::lock_guard<std::mutex> lock(game_mutex);
    for (auto it = bonuses.begin(); it != bonuses.end(); ++it) {
        if (it->first == snake_x && it->second == snake_y) {
            snake_length += 1;  // увеличиваем длину змейки
            bonuses.erase(it);  // удаляем бонус
            break;
        }
    }
}
```

#### Шаг 3: Логика для автоматической змейки

В функции перемещения автоматической змейки добавим логику стремления к бонусам и избегания препятствий:

```cpp
void auto_snake_move(int& snake_x, int& snake_y, bool& game_over) {
    // Пример простого алгоритма движения
    std::lock_guard<std::mutex> lock(game_mutex);

    // Проверка на бонусы
    if (!bonuses.empty()) {
        auto [bonus_x, bonus_y] = bonuses[0];
        if (bonus_x > snake_x) snake_x++;
        else if (bonus_x < snake_x) snake_x--;
        else if (bonus_y > snake_y) snake_y++;
        else if (bonus_y < snake_y) snake_y--;
    }

    // Проверка на препятствия и изменение траектории
    for (auto& obstacle : obstacles) {
        if (obstacle.first == snake_x + 1 && obstacle.second == snake_y) {
            snake_x--;
        }
    }

    check_collision_with_obstacles(snake_x, snake_y, game_over);
    check_collision_with_bonuses(snake_x, snake_y, snake_length);
}
```

#### Шаг 4: Синхронизация потоков

Используем `std::condition_variable`, чтобы уведомлять потоки об изменениях:

```cpp
void game_update_loop() {
    std::unique_lock<std::mutex> lock(game_mutex);
    while (true) {
        cv.wait(lock); // Ждем уведомления об изменении поля

        // Обновление игрового поля
        // Логика отрисовки, проверки коллизий и т.д.
    }
}
```

#### Шаг 5: Оптимизация работы потоков

Для плавной работы игры избегаем длительных блокировок и проверяем корректность кода на наличие гонок данных. Важные ресурсы блокируем только на время критических операций, а остальные действия выполняем вне области блокировки.

### Deadline

20 декабря - требуется сделать Pull Request с полным функционалом и комментариями к измененным частям кода.
