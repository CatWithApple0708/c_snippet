###使用教程和注意事项

1.解析

```c++

auto j3 = json::parse("{ \"happy\": true, \"pi\": 3.141 }");


```

2.读

Warning读会抛出异常
use function at() to access the object values rather than operator[]. In case a key does not exist, at throws an exception that you can handle, whereas operator[] exhibits undefined behavior.
使用方法at去读数值,而不是使用`[]`,如果目标不存在,使用`[]`会造成未定义行为,使用at()会抛出一个异常供处理.

```c++
        j.at("name").get_to(p.name);
        j.at("address").get_to(p.address);
        j.at("age").get_to(p.age);
        
        or:

        s2 = j.at("name").get<std::string>();

        如果通过j["aaa"]["bbb"]去读的话,会自动修改该json文件中的aaa,bbb项{"aaa":{"bbb":null}}

```

3.写

```c++
    json["value"] = b1;
```