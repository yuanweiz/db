#ifndef __SCOPE_EXIT_H
#define __SCOPE_EXIT_H
template <class F>
class ScopeExit{
public:
    ScopeExit(const F& f):f_(f){
    }
    ~ScopeExit(){
        f_();
    }
private:
    F f_;
};
#endif //__SCOPE_EXIT_H
