#include <SimpleFileSystem.h>
#include <vector>

using StringList = std::vector<std::string>;
class SimpleCursor: public Cursor{
    public:
    class Impl: public Cursor::Impl{
        public:
        Impl& advance()override{
            ++iter_;
            return *this;
        }
        StringView get()const override{
            return *iter_;
        }
        bool equals(const Cursor::Impl &rhs_)override{
            const Impl& rhs=static_cast<const Impl&>(rhs_);
            return iter_ == rhs.iter_;
        }
        explicit Impl(const StringList::iterator& iter)
            :iter_(iter)
        {
        }
        private:
        StringList::iterator iter_;
    };
    //SimpleCursor(const StringList::iterator& iter)
    //    //:Cursor(new Impl(iter))
    //{
    //}
    static Cursor fromIterator(const StringList::iterator & iter){
        //return Cursor(new Impl(iter));
        return Cursor::fromPimpl(new Impl(iter));
    }
};
class SimpleFile: public File{
public:
    SimpleFile()=delete;
    SimpleFile(SimpleFile&&)=default;
    class Impl : public File::Impl{
        public:
            Cursor begin(){
                return SimpleCursor::fromIterator(records_.begin());
            }
            Cursor end(){
                return SimpleCursor::fromIterator(records_.end());
            }
        private:
        StringList records_;
    };
    static File fromFileName(const std::string&){
        return File::fromPimpl(new Impl);
    }
};


class SimpleFileSystem::Impl : public FileSystem::Impl{
    public:
    File openFile(const std::string &name)
    {
        return SimpleFile::fromFileName(name);
    }
};
//SimpleFileSystem::SimpleFileSystem()
//    :FileSystem(new Impl())
//{
//}
