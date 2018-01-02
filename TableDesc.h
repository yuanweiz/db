#ifndef __TABLEDESC_H
#define __TABLEDESC_H
#include <string>
#include <memory>
#include <vector>
#include "Types.h"
class Type{
protected:
    Type (DataType);
};

class TableColumn{
public:
    static DataType storageClassToDataType(StorageClass);
    TableColumn(StorageClass storage,const std::string& name)
        :storage_(storage),
        type_(storageClassToDataType(storage)),
        name_(name)
    {
    }
    const std::string& name()const{return name_;}
    DataType type()const{return type_;}
    StorageClass storageClass()const{return storage_;}
private:
    StorageClass storage_;
    DataType type_;
    std::string name_;
};

std::unique_ptr<TableColumn> textColumn(const std::string& name);
std::unique_ptr<TableColumn> integerColumn(const std::string& name);
std::unique_ptr<TableColumn> doubleColumn(const std::string& name);

class NewTable{
    friend class TableDesc;
    using ColumnList = std::vector<std::unique_ptr<TableColumn>>;
    using TableColumnPtr = std::unique_ptr<TableColumn>;
    std::string name_;
    ColumnList columns_;
public:
};
class TableRow{
};

class TableDesc{
    using TableColumnPtr = std::unique_ptr<TableColumn>;
    using ColumnList = std::vector<std::unique_ptr<TableColumn>>;
public:
    TableDesc& operator << (TableColumnPtr&& ptr){
        columns_.emplace_back(std::move(ptr));
        return *this;
    }
    TableDesc(const std::string& tableName)
        :name_(tableName)
    {
    }
    class iterator {
        public:
            iterator(const ColumnList::iterator&iter)
                :iter_(iter)
            {
            }
            iterator(const iterator&)=default;
            TableColumn& operator*() const{
                return *iter_->get();
            }
            iterator& operator++(){
                ++iter_;
                return *this;
            }
            iterator operator++(int){
                iterator bak = *this;
                iter_++;
                return bak;
            }
            bool operator!= (const iterator&rhs)const{
                return iter_!=rhs.iter_;
            }
            bool operator== (const iterator&rhs)const{
                return iter_==rhs.iter_;
            }
        private:
            ColumnList::iterator iter_;
    };
    iterator begin(){
        return iterator(columns_.begin());
    }
    iterator end(){
        return iterator(columns_.end());
    }
private:
    std::string name_;
    ColumnList columns_;
};

#endif// __TABLEDESC_H
