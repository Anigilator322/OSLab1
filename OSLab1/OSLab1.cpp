#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <format>

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

    Event getEvent() {
        unique_lock<mutex> lock(_mutex);
        _cv.wait(lock, [this] { return !_queue.empty(); });
        Event event = _queue.front();
        _queue.pop();
        return event;
    }

private:
    queue<Event> _queue;
    mutex _mutex;
    condition_variable _cv;
};

void supplier(Monitor& monitor) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);
    bool isSended = false;
    while (!isSended) 
    {
        Event event = Event(dist(gen), this_thread::get_id());
        
        cout << "Поставщик "<< event._Sender <<": Сгенерировано событие с данными " << event._data << endl;
        monitor.addEvent(event);

        cout << "Поставщик: Обработка события..." << endl;
        isSended = true;
    }
}

void consumer(Monitor& monitor) {
    while (true)
    {
        Event event = monitor.getEvent();

        cout << "Потребитель: Получено событие с данными " << event._data <<" Отправитель: "<< event._Sender << endl;
        cout << "Потребитель: Обработка события..." << endl;
    }
}

int main() {
    Monitor monitor;
    setlocale(LC_ALL, "russian");
    thread supplierThread(supplier, ref(monitor));
    thread consumerThread(consumer, ref(monitor));

    supplierThread.join();
    consumerThread.join();

    return 0;
}
