cmake_minimum_required(VERSION 3.12)
project(php-ext-datrie C)

set(CMAKE_C_STANDARD 99)
set(phproot /usr/local/src/php-7.2.11/)
include_directories(${phproot})
include_directories(${phproot}TSRM/)
include_directories(${phproot}main/)
include_directories(${phproot}Zend/)

include_directories(./datrie)
include_directories(./)


add_executable(php-ext-datrie php_datrie.c)