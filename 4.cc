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
X       std::unique_lock<std::mutex> lk(m);
X       data.emplace(std::move(t));
X       cv.notify_one();  // if someone's waiting, get them to wake up and pop this item
    }

    T blocking_pop() {
X       while (data.empty()) {
X           std::unique_lock<std::mutex> lk(m);
X           cv.wait(lk);  // wait for an item to be pushed
X       }
X       std::unique_lock<std::mutex> lk(m);
X       T result = std::move(data.top());
X       data.pop();
X       lk.unlock();
X       return result;
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
