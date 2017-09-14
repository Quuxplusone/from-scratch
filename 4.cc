#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stack>
#include <thread>
#include <vector>
#include <stdio.h>

using namespace std::chrono_literals;

template<class T>
struct ThreadSafeStack {
    std::mutex m;  // guards the "data" member
    std::condition_variable cv;  // wait on this if "data" is empty
    std::stack<T> data;
    
    void push(T t) {
Y       std::unique_lock<std::mutex> lk(m);
Y       data.emplace(std::move(t));
Y       cv.notify_one();  // if someone's waiting, get them to wake up and pop this item
    }

    T blocking_pop() {
Y       while (data.empty()) {
Y           std::unique_lock<std::mutex> lk(m);
Y           cv.wait(lk);  // wait for an item to be pushed
Y       }
Y       std::unique_lock<std::mutex> lk(m);
Y       T result = std::move(data.top());
Y       data.pop();
Y       lk.unlock();
Y       return result;
    }
};

int main()
{
    ThreadSafeStack<int> myStack;
    std::vector<std::thread> threads;
    std::atomic<bool> retrieved[52] {};
    myStack.push(50);
    myStack.push(51);  // "Seed" the stack with just a couple of items.

    for (int i=0; i < 50; ++i) {
        threads.emplace_back([&, i](){
            // Pop an item, then push our own "thread id" (0-49) for someone else to pop.
            int j = myStack.blocking_pop();
            myStack.push(i);
            // Check that the item we popped hasn't been popped before.
            if (retrieved[j]) {
                printf("Uh-oh! Thread %d retrieved %d for the second time!\n", i, j);
            } else {
                retrieved[j] = true;
                printf("thread %d retrieved %d\n", i, j);
            }
        });
    }
    for (auto&& t : threads) {
        t.join();
    }
}
