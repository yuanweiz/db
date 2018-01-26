#include <gtest/gtest.h>
#include <FileSystem.h>
#include "helper.h"

bool stringViewLess(StringView a,StringView b){
    return std::string(a) < std::string(b); //slow
}
void test(){
    {
        Helper::MockPageAllocator allocator;
        FileSystem fs(&allocator);
        auto file = fs.openFile("master");
        char buf[1024];
        std::vector<std::string> sortedStrings;
        for (int i=0;i<20;++i){
            size_t size = rand()%100;
            Helper::fillRandomData(buf,size);
            StringView strView{buf,size};
            sortedStrings.push_back(strView);
            file->insert(strView, stringViewLess );
        }
        std::sort(sortedStrings.begin(),sortedStrings.end());
        size_t i=0;
        for (auto && strView: *file){
            std::string str = strView;
            ASSERT_TRUE( i < sortedStrings.size());
            ASSERT_EQ( str, sortedStrings[i++]);
        }
    }
}

int main (int argc,char**argv){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
