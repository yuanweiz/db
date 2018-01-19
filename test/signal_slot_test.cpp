#include <typeinfo>
#include <stdio.h>
#include <assert.h>
#include <SignalSlot.h>
#include <gtest/gtest.h>

class SimplePODSignalSlotTest : ::testing::Test{
public:
    class Sender{
        public:
            Sender(int v):v_(v){}
            Signal<int,int> valueChange;
            void changeVal(int v){
                int oldValue = v_;
                v_ = v;
                valueChange.emit(oldValue,v);
            }
        private:
            int v_;
    };

    class Receiver{
        public:
            Receiver()
                :valueChange([this](int oldValue,int newValue){
                        onValueChange(oldValue,newValue);
                        })
            {}
            ~Receiver(){

            }
            Slot<int,int> valueChange;
        private:
            void onValueChange(int oldValue,int newValue){
                printf("oldValue=%d newValue=%d\n",oldValue,newValue);
            }
    };
};



struct Foo{
};

struct Bar{
    Bar(const Foo&){
        puts("copy ctor");
    }
    Bar(Foo&&){
        puts("move ctor");
    }
};
TEST( SimplePODSignalSlotTest, LifeTime){
    using Sender = SimplePODSignalSlotTest::Sender;
    using Receiver = SimplePODSignalSlotTest::Receiver;
    {
        Sender sender(42);
        Receiver receiver;
        receiver.valueChange.connect(sender.valueChange);
        sender.changeVal(3);
    }
    {
        Sender sender(42);
        {
            Receiver receiver;
            receiver.valueChange.connect(sender.valueChange);
        }
        assert(sender.valueChange.numOfReceivers()==0);
        sender.changeVal(3);
    }
    {
        Receiver receiver;
        {
            Sender sender(42);
            receiver.valueChange.connect(sender.valueChange);
        }
        assert(receiver.valueChange.numOfSenders() == 0);
    }
}
int main (int argc, char ** argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
