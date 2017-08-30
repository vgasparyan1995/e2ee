#pragma once

#define SINGLETON_DECL(Class) \
public:\
    static Class* get_instance();\
    static void remove_instance();\
private:\
    static Class* s_instance;\
    Class();\
    ~Class();\
    Class(const Class&) = delete;\
    Class(Class&&) = delete;\
    Class& operator=(const Class&) = delete;\
    Class& operator=(Class&&) = delete;

#define SINGLETON_DEF(Class) \
Class* Class::s_instance = nullptr;\
Class* Class::get_instance()\
{\
    if (s_instance == nullptr) {\
        s_instance = new Class();\
    }\
    return s_instance;\
}\
void Class::remove_instance()\
{\
    delete s_instance;\
    s_instance = nullptr;\
}

