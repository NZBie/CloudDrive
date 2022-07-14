#ifndef FORMDATA_PARSER_H
#define FORMDATA_PARSER_H

#include <memory>
#include <vector>
#include <cstring>

#include "FormItem.h"

class FormDataParser {
    private:
        char*_data;                     ///< 指向表单数据的针
        int _dataLength;
        char* _boundary;                ///< 不同元素的分割符串

        bool _lastBoundaryFound;        ///< 是否找到了最后边界
        int _pos;                       ///< 当前解析到的位置
        int _lineStart;                 ///< 当前行的起始位置
        int _lineLength;                ///< 当前行的长度

        char* _partName;                ///< 当前元素的名
        char* _partFileName;            ///< 当前元素的文件名
        char* _partContentType;         ///< 当前元素的类型
        int _partDataStart;             ///< 当前元素的数据表单中的起始位置
        int _partDataLength;            ///< 当前元素的数据长度
    public:
        FormDataParser(char* data, int dataLen, char* boundary, int pos)
        : _data(data), _dataLength(dataLen), _boundary(boundary), _pos(pos) {};
        ~FormDataParser() {};
        /**
         * 调用parse函数后，才会执行对应的解析操作，
         * @return 指向由FormItem组成的vector的unique_ptr
         */
        std::vector<FormItem>* parse();  
    private:
        /**
         * 解析表单数据的头部，即紧跟着boundary后面的一行
         */
        void parseHeaders();
        /**
         * 解析表单元素中的具体数据
         */
        void parseFormData();
        /**
         * 获取下一行的数据，
         * 在此实际上是通过更新类内部的_pos, _lineStart,_lineLength实现的
         * @return 是否成功得到下一行的数据
         */
        bool getNextLine();
        /**
         * 判断是否为边界分割行
         * @return 是边界分割行放回true，否则返回false
         */
        bool atBoundaryLine();
        /**
         * 判断是否到达表单数据的末尾
         */
        inline bool atEndOfData(){
            return _pos >= _dataLength;
        }            
        char* getDispositionValue(
            char* pos, char* attrName);
};

#endif