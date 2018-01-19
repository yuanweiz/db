#ifndef __SIGNAL_SLOT_H
#define __SIGNAL_SLOT_H
#include <functional>
#include <vector>
template <typename ...>class Slot;
template <typename ...Args>
class Signal{
    friend class Slot<Args...>;
    using Receiver = class Slot<Args...>;
public:
    void emit(Args ...args){
        for (auto * pReceiver: receivers_){
            pReceiver->invoke(args...); //TODO: std::forward
        }
    }
    void addReceiver( Receiver* r){
        receivers_.push_back(r);
    }
    void removeReceiver(Receiver * r){
        for (auto it = receivers_.begin();it!=receivers_.end();){
            if (*it==r){
                it = receivers_.erase(it);
            }
            else ++it;
        }
    }
    size_t numOfReceivers()const {
        return receivers_.size();
    }
    ~Signal(){
        for (auto * receiver: receivers_){
            receiver->disconnect(this);
        }
    }
private:
    std::vector<Receiver*> receivers_;
};

template <typename ...Args>
class Slot{
    //friend class Signal;
public:
    //init with a method
    //
    using Sender = Signal<Args...>;
    template <class Func>
    Slot( const Func& func)
        : func_(func)
    {
    }
    ~Slot(){
        for(auto * sender :senders_){
            sender->removeReceiver(this);
        }
    }
    void connect(Sender& sender){
        sender.addReceiver(this);
        senders_.push_back(&sender);
    }
    void invoke(Args ... args){
        func_(args...);
    }
    size_t numOfSenders()const {
        return senders_.size();
    }
    void disconnect(Sender* sender){
        auto it = senders_.begin();
        for (; it!=senders_.end();){
            if (*it == sender){
                it = senders_.erase(it);
            }
            else ++it;
        }
    }
private:
    std::function<void(Args...)> func_;
    std::vector<Sender*> senders_;
};

#endif// __SIGNAL_SLOT_H
