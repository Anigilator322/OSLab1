#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <random>

using namespace std;

struct Message
{
	int _data;
	thread::id _SenderId;
	Message(int data, thread::id id)
	{
		_data = data;
		_SenderId = id;
	};
};
class Monitor
{
private:
	queue<Message> _queue;
	mutex _mutex;
	condition_variable _condition;
	bool _isStopped = false;
public:
	void SendMessage(const Message& message)
	{
		unique_lock<mutex> lock(_mutex);
		_queue.push(message);
		_condition.notify_all();
	}

	bool TryReceiveMessage(Message& message)
	{
		unique_lock<mutex> lock(_mutex);
		_condition.wait(lock, [this] {return !_queue.empty() || _isStopped; });
		if (_isStopped && _queue.empty())
		{
			return false;
		}
		message = _queue.front();
		_queue.pop();
		return true;
	}

	void Stop()
	{
		unique_lock<mutex> lock(_mutex);
		_isStopped = true;
		_condition.notify_all();
	}
};

void Consumer(Monitor& monitor)
{
	while (true) {
		Message message(0, thread::id());
		if (!monitor.TryReceiveMessage(message)) {
			break;
		}

		cout << "Потребитель: Получено событие с данными " << message._data << " Отправитель: " << message._SenderId << endl;
		cout << "Потребитель: Обработка события..." << endl;
		this_thread::sleep_for(chrono::milliseconds(1000));
	}

	cout << "Потребитель: Завершение работы." << endl;
}

void Supplier(Monitor& monitor)
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(1, 100);

	for (int i = 0; i < 5; ++i) {
		Message message = Message(dist(gen), this_thread::get_id());

		cout << "Поставщик " << message._SenderId << ": Сгенерировано событие с данными " << message._data << endl;
		monitor.SendMessage(message);

		this_thread::sleep_for(chrono::milliseconds(500));
	}

	cout << "Поставщик: Завершение работы." << endl;
}


int main() 
{
	setlocale(LC_ALL, "russian");
	Monitor monitor;
	thread supplierThread(Supplier, ref(monitor));
	thread consumerThread(Consumer, ref(monitor));

	supplierThread.join();
	monitor.Stop();
	consumerThread.join();

	return 0;
}

