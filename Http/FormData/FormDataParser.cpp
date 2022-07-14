#include "FormDataParser.h"

std::vector<FormItem>* FormDataParser::parse() {
    auto result = new std::vector<FormItem>();
        
    //跳过空白行，直到遇到边界boundary，表示一个表单数据的开始
    while(getNextLine()){
        _data[_lineStart + _lineLength] = '\0';
        if(atBoundaryLine()){
            break;
        }
    }
    do {
        //处理头部
        parseHeaders();
        //头部过后如果没有数据，跳出循环
        if(atEndOfData()){ break; }
        //处理该项表单数据
        parseFormData();
        //将表单数据添加到结果数组中
        FormItem formItem(_partName, _partFileName, _partContentType, _data, _partDataStart, _partDataLength);
        result->push_back(std::move(formItem));
    } while(!atEndOfData());
    
    return result;
}

bool FormDataParser::getNextLine() {

    int i = _pos;
    _lineStart = -1;

    while(i < _dataLength){
        //找到一行的末尾
        if(_data[i] == '\n'){
            _lineStart = _pos;
            _lineLength = i - _pos;
            _pos = i + 1;
            //忽略'\r'
            if(_lineLength > 0 && _data[i - 1] == '\r'){
                _lineLength--;
            }
            break;
        }
        //到达表单数据的末尾了
        if(++i == _dataLength){
            _lineStart = _pos;
            _lineLength = i - _pos;
            _pos = _dataLength;
        }
    }
    return _lineStart >= 0;
}

bool FormDataParser::atBoundaryLine() {
    int boundaryLength = strlen(_boundary);
    //最后的边界会多两个'-'符号
    if(boundaryLength != _lineLength && boundaryLength + 2 != _lineLength) {
        return false;
    }

    for(int i = 0; i < boundaryLength; ++i){
        if(_data[_lineStart + i] != _boundary[i]){ return false; }
    }

    if(_lineLength == boundaryLength){ return true; }
    //判断是否是最后的边界
    if(_data[boundaryLength + _lineStart] != '-' || _data[boundaryLength + _lineStart + 1] != '-'){
        return false;
    }  

    //到达最后的边界
    _lastBoundaryFound = true;
    return true;
}

void FormDataParser::parseHeaders() {
    //清除之前的数据
    // _partFileName.clear();
    // _partName.clear();
    // _partContentType.clear();

    while(getNextLine()){

        _data[_lineStart + _lineLength] = '\0';

        //头部内容结束后，会有一个空白行
        if(_lineLength == 0){ break; }
        char* thisLine = _data + _lineStart;

        char* pos = strpbrk(thisLine, ":");
        if(pos == nullptr){ continue; }
        *(pos++) = '\0';

        char* header = thisLine;
        if(strcmp(header, "Content-Disposition") == 0) {
            _partFileName = getDispositionValue(pos, "filename");
            _partName = getDispositionValue(pos, "name");
        }
        else if(strcmp(header, "Content-Type") == 0) {
            pos += strspn(pos, " ");
            _partContentType = pos;
        }
    }
}

char* FormDataParser::getDispositionValue(char* pos, char* attrName){
    //头部内容：Content-Disposition: form-data; name="projectName"
    //构建模式串
    char* partStart = strstr(pos, attrName);
    if(partStart != nullptr) {
        partStart += strlen(attrName);
        partStart += strspn(partStart, " =\"");
        char* partEnd = strpbrk(partStart, " \"");
        *partEnd = '\0';
    }
    return partStart;
}

void FormDataParser::parseFormData() {
    _partDataStart = _pos;
    _partDataLength = -1;

    while(getNextLine()) {
        if(atBoundaryLine()) {
            //内容数据位于分解线前一行
            int indexOfEnd = _lineStart - 1;
            if(_data[indexOfEnd] == '\n'){ indexOfEnd--; }
            if(_data[indexOfEnd] == '\r'){ indexOfEnd--; }
            _partDataLength = indexOfEnd - _partDataStart + 1;
            _data[_partDataStart + _partDataLength] = '\0';
            break;
        }
    }
}
