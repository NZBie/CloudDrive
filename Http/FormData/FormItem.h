#ifndef FORM_ITEN_H
#define FORM_ITEN_H

#include <string>
#include <memory>

class FormItem{
    /**
     * 将MultipartContentElement作为MultipartContentParse的友元类
     * 使得MultipartContentParse对MultipartContentElement具有完的控制权
     * 因为两个类是强相关的，且MultipartContentElement中的数据应由
     * MultipartContentParse进行设置
     */
    friend class FormDataParser;
    private:
        const char* _fileName;                      ///< 表单元为文件时，具有文件名
        const char* _name;                          ///< 表单元的key
        const char* _contentType;                   ///< 表单元该部分的类型
        const char* _content;                       ///< 指request中的表单内容
        int _dataStart;                             ///< 属于本单元素的内容的起始下标
        int _dataLength;                            ///< 属于本单元素的内容的长度
    
        /**
         * MultipartContentElement对象只能由MultipartContentPars生成
         * 将构造函数的访问权设置为private，防止外部创建
         * @param name 表单元素的名
         * @param fileName 文件时fileName不为空
         * @param contentType 类型
         * @param content 指向表单数据的指针
         * @param start 本表单元素的起始位置
         * @param length 本表单元素的数据长度
         * @return MultipartContentElement对象
         */
        FormItem(const char* name, 
            const char* fileName, const char* contentType,
            const char* content, 
            const int start, const int length)
			:_name(name), _fileName(fileName), _contentType(_contentType), _content(content), _dataStart(start), _dataLength(length) {
		}
    public:
        const char* getFileName() const { return _fileName; }
        const char* getName() const { return _name; }
        const char* getContentType() const { return _contentType; }
        bool isFile() const { return !strlen(_fileName); }
        int getSize() const { return _dataLength; }
        /**
         * 获取具体的内容，不返回指向原始内容的指针
         * 而是复制内容，防止外部对请求作出更改，影响到同一表单中同元素的内容
         * @return 新复制的内容的指针
         */
        const char* getContent() const {
            return _content + _dataStart;
        };
};

#endif
