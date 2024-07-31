# Mutex

## Пререквизиты

- [fiber/yield](/tasks/fiber/yield)
- [sched/run_loop](/tasks/sched/run_loop)
- [рекомендуется] [sync/queue_spinlock](/tasks/sync/queue_spinlock)  
- [рекомендуется] [sched/intrusive](/tasks/sched/intrusive)
- [рекомендуется] [fiber/strand](/tasks/fiber/strand)

---

Реализуйте примитивы синхронизации для файберов:
- [`Event`](exe/fiber/sync/event.hpp)
- [`Mutex`](exe/fiber/sync/mutex.hpp)
- [`WaitGroup`](exe/fiber/sync/wait_group.hpp)

Методы `Event::Wait`, `Mutex::Lock` и `WaitGroup::Wait` должны останавливать<sup>†</sup> файбер, но не должны блокировать поток планировщика, в котором этот файбер исполняется.

<sup>†</sup> Файберы – _останавливаются_ (_suspend_), потоки – _блокируются_ (_block_).

## Пример

```cpp
void SyncExample() {
  using namespace exe;
  
  sched::ThreadPool scheduler{4};
  scheduler.Start();
  
  thread::WaitGroup example;
  example.Add(1);
  
  fiber::Go(scheduler, [&example] {
    fiber::Mutex mutex;
    size_t cs = 0;
    
    // https://gobyexample.com/waitgroups
    fiber::WaitGroup wg;
    
    for (size_t i = 0; i < 123; ++i) {
      wg.Add(1);
      
      fiber::Go([&] {
        // https://gobyexample.com/defer
        Defer defer([&wg] {
          wg.Done();
        });
        
        for (size_t j = 0; j < 1024; ++j) {
          std::lock_guard guard{mutex};
          ++cs;  // <-- в критической секции
        }
      });
    }
    
    // Дожидаемся завершения группы файберов
    wg.Wait();
    
    fmt::println("# critical sections: {}", cs);
    // <-- Напечатано 123 * 1024
    
    example.Done();
  });
  
  // Дожидаемся завершения примера
  example.Wait();
  
  scheduler.Stop();
}
```

### Кондвар?

Несмотря на то, что в Golang есть [`sync.Cond`](https://pkg.go.dev/sync#Cond), мы намеренно не включаем кондвар
в набор примитивов синхронизации для файберов.

Подумайте над этим, решая задачу.

## Реализация

### Спинлок

Для реализации примитивов синхронизации вам потребуется взаимное исключение.

Реализуйте и используйте [собственный спинлок](exe/thread/spinlock.hpp).

### Fast Path

На _быстром пути_ в примитивах синхронизации не должно быть переключений контекста.

### Sync

Изолируйте всю синхронизацию в примитивах из `fiber/sync`.

Детали реализации (атомики, спинлоки, интрузивные списки) отдельных примитивов синхронизации не должны проникать в `fiber/core`.

### Lock-free

Продвинутый уровень!

Напишите лок-фри реализацию `Event`, `Mutex` и `WaitGroup`, без использования взаимного исключения.

### Symmetric Transfer

Продвинутый уровень!

Реализуйте серийный лок-фри `Mutex`.

Чем больше будет нагружен ваш мьютекс, тем **эффективнее** (с точки зрения протокола когерентности) будут исполняться критические секции.

Пройдите тесты [pipelines/fiber/sync/mutex/all.json](pipelines/fiber/sync/mutex/all.json)

## Задание

1) Реализуйте примитивы синхронизации:
   1) [`Event`](exe/fiber/sync/event.hpp)
   2) [`Mutex`](exe/fiber/sync/mutex.hpp)
   3) [`WaitGroup`](exe/fiber/sync/wait_group.hpp)
2) Оптимизируйте [динамические аллокации](alloc.md)
3) Напишите `Strand` из задачи [fiber/strand](/tasks/fiber/strand)   
4) Напишите лок-фри реализацию примитивов синхронизации
5) Напишите серийный лок-фри `Mutex`
6) Оптимизируйте `memory_order` в примитивах синхронизации
