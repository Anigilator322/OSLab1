#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <atomic>

using namespace std;

struct Event {
    int _data;
    std::thread::id _Sender;
    Event(int data, std::thread::id sender)
    {
        _data = data;
        _Sender = sender;
    }
};

class Monitor {
public:
    void addEvent(const Event& event) {
        unique_lock<mutex> lock(_mutex);
        _queue.push(event);
        _cv.notify_one();
    }

    bool getEvent(Event& event) {
        unique_lock<mutex> lock(_mutex);
        _cv.wait(lock, [this] { return !_queue.empty() || _isStopped; });
        if (_isStopped && _queue.empty()) {
            return false;
        }
        event = _queue.front();
        _queue.pop();
        return true;
    }

    void stop() {
        unique_lock<mutex> lock(_mutex);
        _isStopped = true;
        _cv.notify_all();
    }

private:
    queue<Event> _queue;
    mutex _mutex;
    condition_variable _cv;
    bool _isStopped = false;
};

void supplier(Monitor& monitor) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);

    for (int i = 0; i < 5; ++i) {
        Event event = Event(dist(gen), this_thread::get_id());

        cout << "Поставщик " << event._Sender << ": Сгенерировано событие с данными " << event._data << endl;
        monitor.addEvent(event);

        this_thread::sleep_for(chrono::milliseconds(500));
    }

    cout << "Поставщик: Завершение работы." << endl;
}

void consumer(Monitor& monitor) {
    while (true) {
        Event event(0, thread::id());
        if (!monitor.getEvent(event)) {
            break;
        }

        cout << "Потребитель: Получено событие с данными " << event._data << " Отправитель: " << event._Sender << endl;
        cout << "Потребитель: Обработка события..." << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }

    cout << "Потребитель: Завершение работы." << endl;
}

int main() {
    Monitor monitor;
    setlocale(LC_ALL, "russian");

    thread supplierThread(supplier, ref(monitor));
    thread consumerThread(consumer, ref(monitor));

    supplierThread.join();
    monitor.stop();
    consumerThread.join();

    return 0;
}

