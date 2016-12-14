# Viac语言编译器
Viac语言compiler
主要面对ViaC语言，兼容C89语言大部分，目前不支持while、switch(和前者都可以用for替代)、include(头文件依靠ViaC内部的机制)等语法，
支持指针、sizeof、for、int、double等语法。
新增do、end以及//(行注释符），并且新增了require(头文件)机制。
