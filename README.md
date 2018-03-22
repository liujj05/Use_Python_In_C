# C调用Python代码

本工程将实现UR机器人提供的Python版本的RTDE功能向C语言的移植

## 1 文件介绍

### 1.1 Easy_Example

这是一个从网上看过来的如何利用Cython从pyx生成C语言可以调用代码的简单例程。原文链接：

> http://www.voidcn.com/article/p-vrcojger-uo.html

但是他并没有详细说明Cython在这里的作用，给我的感觉是，用了Cython编写的pyx并进行了编译生成了.h和.c文件后，对于后续C的调用起不到什么实质性的简化作用。并且编译要采用控制台命令，这对于后续的MFC程序的开发很不利。

针对如何在Visual Studio界面下进行编译，以下文章可能是一个很好的参考，虽然暂时没有尝试是否能够成功：

> http://blog.csdn.net/youngstar70/article/details/70175117

第一个链接的作者称这是因为原函数没有指定变量类型。后续他给出的一个例子中，重新定义了那个great_function，从原来的定义：

```
#great_module.pyx
cdef public great_function(a,index):
     return a[index]
```

变为

```
#great_module.pyx
cdef public char great_function(const char * a,int index):
    return a[index]
```

就可以减少后续调用的C代码当中的Python痕迹。但是当前的任务是已经存在一个或者多个.py，我们要调用它们。所以这个例子权当对Cython的一个了解。更直观的能够实现我们要求的例子可能是下面这个链接的文章：

[C语言调用Python脚本中的函数](http://blog.csdn.net/hexiaomin_1984/article/details/37969193)

### 1.2 rtde_client

这个文件夹中是一个UR机器人官方提供的rtde功能的例程。其中包含了如何配置输入输出项、如何与机器人建立通信等功能。这个文件夹下的rtde文件夹中的内容，就是我们希望能够在C语言中实现的功能，当然我们也可以通过Socket编程直接重新编写，但是相比于移植，这个工作时间恐怕会更长。

### 1.3 Test001

这个文件夹下是一个 Visual Studio 2017 的控制台程序工程，为了进行在C中调用Python的尝试操作。具体的内容在2. 开发步骤 的 2.1节中有详细介绍。

### 1.4 ControlLoop

如果只对开发一个利用RTDE控制UR机器人的C控制台程序感兴趣，可以直接看这一节，这个 Visual Studio 2017 工程会完整实现一个与UR机器人互动的程序，当然需要机器人端编写相关的程序进行配合。

---

## 2 开发步骤

### 2.1 Test001

在C中尝试调用rtde中的一些不需要连接机器人的功能。具体参照
[C语言调用Python脚本中的函数](http://blog.csdn.net/hexiaomin_1984/article/details/37969193)来建立这个功能。起初代码编写完成之后，编译通过但是链接失败，网上检索后根据[在VS2012中C++嵌入python](http://blog.csdn.net/haimengao/article/details/16867547)一文发现，64位Python的机器上，工程必须设定在x64环境下编译，否则就会链接失败。

同时，必须对项目属性进行配置
1. 在VC++包含目录添加包含Python.h的include目录
2. 在VC++库目录添加包含python27.lib的libs目录
3. 在链接器中添加python27.lib为附加依赖项

首先尝试了以下代码：

```
int main()
{
    PyObject *PM_rtde_config;
    Py_Initialize();
    PM_rtde_config = PyImport_Import(PyString_FromString("rtde.rtde_config"));
    Py_Finalize();
    return 0;
}
```

通过设置断点，发现PM_rtde_config非NULL，说明载入成功。进一步添加功能。**注意：rtde这个文件夹需要放在和Test001.cpp这个文件相同的位置下！**

例程的`example_control_loop.py`这个文件中，摘出一段与`rtde.rtde_config`相关的代码：

```python
config_filename = 'control_loop_configuration.xml'
conf = rtde_config.ConfigFile(config_filename)
state_names, state_types = conf.get_recipe('state')
setp_names, setp_types = conf.get_recipe('setp')
watchdog_names, watchdog_types = conf.get_recipe('watchdog')
```
#### 2.1.1 走的弯路

***如果想直接看实现请跳过这一节，这一节是调试过程，是一些犯过错误的记录***

想办法把它用C实现。进一步充实的第一版程序写成了这个样子

```c++
int main()
{
    PyObject *PM_rtde_config;
    PyObject *pFunc_ConfigFile;
    PyObject *pArgs;				//ConfigFile的入口参数
    PyObject *pValue;				//ConfigFile的出口参数

    Py_Initialize();

    // 在python中相当于import了这个文件
    PM_rtde_config = PyImport_Import(PyString_FromString("rtde.rtde_config"));

    // 获取到ConfigFile这个函数-这实际上是一个类的构造函数
    pFunc_ConfigFile = PyObject_GetAttrString(PM_rtde_config, "ConfigFile");

    // ConfigFile函数的入口参数只有一个string：config_filename
    pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, PyString_FromString("control_loop_configuration.xml"));

    // 执行程序
    pValue = PyObject_CallObject(pFunc_ConfigFile, pArgs);

    // 解析出口参数


    Py_Finalize();
    return 0;
}
```

但是，`pValue`作为返回值却是NULL，说明调用出现了错误！
意识到`ConfigFile`实际上是一个类，这是一个构造函数，可能调用方式不是这样的。于是重新检索了方法，具体方法见
> [C++调用Python(3)](http://blog.csdn.net/marising/article/details/2917892)
这个网页的内容包含如何在C中引入一个Python的类并并将其初始化

接下来用了一个小的自编例子，自行写了一个Python的小类，文件叫做`TestMyClass`，代码很简单：

```python
class ConfigFile():
    def say_ur_name(self):
        print 'LiuJiajun'
```

实际上这个类定义是不严格符合Python目前语法的，首先ConfigFile的括号里面应该有一个object的，用来表示这个类继承自object类，但是那样写就不能彻底取消掉这个类的构造函数`__init__`了，为什么要这样做后续会提到。
接下来在C当中：

```c++
// 在python中相当于import了这个文件
PM_rtde_config = PyImport_Import(PyString_FromString("TestMyClass"));

// 调取模块的字典列表
PyObject* pDict = PyModule_GetDict(PM_rtde_config);

// 打印一下看看
printDict(pDict);

// 提取这个类
pClass_ConfigFile = PyDict_GetItemString(pDict, "ConfigFile");

// 利用这个类定义初始化一个实例
PyObject* pInstance_ConfigFile = PyInstance_New(pClass_ConfigFile, NULL, NULL);
```

这样进行初始化后，`pInstance_ConfigFile`这个指针不会为`NULL`
但是，同样是以上这段C代码，在Python代码仅仅加入了一个object的情况下（加一个object表示继承自object类，这是new style class 的定义方法，相对的，不加object的类被称为old style class）：

```python
class ConfigFile(object):
    def say_ur_name(self):
        print 'LiuJiajun'
```

`pInstance_ConfigFile`指针会变成`NULL`，也就是说初始化实例会出错！
检索后发现：[据说](https://www.v2ex.com/t/92546)，这个pInstance_ConfigFile只能实例化old style class？？！！这也太尴尬了……又[据说](https://stackoverflow.com/questions/40351244/create-an-instance-of-python-new-style-class-to-embed-in-c)另外一个叫做`PyObject_CallMethod`和`PyObject_CallObject`能解决这个问题？

针对以上这种 new style class，修改C中的代码：

```c++
PyObject* pInstance_ConfigFile = PyInstance_New(pClass_ConfigFile, NULL, NULL);
```

为

```c++
PyObject* pInstance_ConfigFile = PyObject_CallObject(pClass_ConfigFile, NULL);
```

终于初始化实例成功。那么重新回到正题。

#### 2.1.2 如何将 New Style Class 初始化并调用其中的成员函数

这里可以参考以下这个例子

> [Python嵌入C++详解(3)--Import Class](http://blog.csdn.net/xiadasong007/article/details/4513615)

仍然先以一个简单的例子开始。如果有一个`TestMyClass.py`内容如下：

```python
class ConfigFile(object):
    def __init__(self, filename):
        self.filename = filename
    def say_ur_name(self):
        print self.filename
```

如何在C中将 New Style Class 初始化并调用 say_ur_name 函数？使用以下代码：

```c++
#include "stdafx.h"
#include <Python.h>

void printDict(PyObject* obj) // 打印一个Python对象的Dict，但是似乎做的不太好，原来代码的换行符写错了
{
    if (!PyDict_Check(obj))
        return;
    PyObject *k, *keys;
    keys = PyDict_Keys(obj);
    for (int i = 0; i < PyList_GET_SIZE(keys); i++) {
        k = PyList_GET_ITEM(keys, i);
        char* c_name = PyString_AsString(k);
        //printf("%s/n", c_name);
        printf("%s\n", c_name);
    }
}

int main()
{
    PyObject *PM_rtde_config;
    PyObject *pClass_ConfigFile;
    PyObject *pArgs;				//ConfigFile的入口参数
    PyObject *pValue;				//ConfigFile的出口参数

    Py_Initialize();

    // 在python中相当于import了这个文件
    // PM_rtde_config = PyImport_Import(PyString_FromString("rtde.rtde_config"));
    PM_rtde_config = PyImport_Import(PyString_FromString("TestMyClass"));
    // 调取模块的字典列表
    PyObject* pDict = PyModule_GetDict(PM_rtde_config);

    // 打印一下看看
    printDict(pDict);

    pClass_ConfigFile = PyDict_GetItemString(pDict, "ConfigFile");
    pArgs = Py_BuildValue("(s)", "control_loop_configuration.xml");		// 构造输入参数
    //pArgs = PyTuple_New(1);
    //PyTuple_SetItem(pArgs, 0, PyString_FromString("control_loop_configuration.xml"));
    PyObject* pInstance_ConfigFile = PyObject_CallObject(pClass_ConfigFile, pArgs);

    Py_Finalize();
    return 0;
}

```

注意到：在构造输入参数pArg时有两种方法（包括注释掉的那一种）：
方法1（注意这个s要加括号()否则最终返回的实例为NULL）：

```c++
pArgs = Py_BuildValue("(s)", "control_loop_configuration.xml");
```

方法2：

```c++
pArgs = PyTuple_New(1);
PyTuple_SetItem(pArgs, 0, PyString_FromString("control_loop_configuration.xml"));
```

#### 2.1.3 导入rtde.rtde_config模块

这里只列出关键的代码段。以下代码段的功能是：

- 载入`rtde.rtde_config`模块
- 获取这个模块的`ConfigFile`类
- 配置一个输入参数`pArgs`并用该参数初始化该类的一个实例`pInstance_ConfigFile`
- 调用这个实例的成员函数`get_recipe`，将该成员函数的返回值存入`PyObject*`类型的变量`p_res`之中
- 解析`p_res`并打印第一个字符

```c++
PyObject *PM_rtde_config;
PyObject *pClass_ConfigFile;
PyObject *pArgs;				//ConfigFile的入口参数
PyObject *pValue;				//ConfigFile的出口参数

Py_Initialize();

// 在python中相当于import了这个文件
PM_rtde_config = PyImport_Import(PyString_FromString("rtde.rtde_config"));
// 调取模块的字典列表
PyObject* pDict = PyModule_GetDict(PM_rtde_config);

// 打印一下看看
printDict(pDict);
pClass_ConfigFile = PyDict_GetItemString(pDict, "ConfigFile");
pArgs = Py_BuildValue("(s)", "control_loop_configuration.xml");		// 构造输入参数
PyObject* pInstance_ConfigFile = PyObject_CallObject(pClass_ConfigFile, pArgs);

// 调用成员函数 get_recipe
PyObject* p_Func_get_recipe = PyObject_GetAttrString(pInstance_ConfigFile, "get_recipe");
PyObject* p_input = Py_BuildValue("(s)", "state");
PyObject* p_res = PyEval_CallObject(p_Func_get_recipe, p_input);

// 成员函数 get_recipe 输出结果 p_res 解析
if (!PyTuple_Check(p_res))
{
    printf("p_res is not tuple!");
    return 0;
}

PyObject* py_obj_state_names = PyTuple_GetItem(p_res, 0);
PyObject* py_obj_state_types = PyTuple_GetItem(p_res, 1);

// 这两个 object 在Python中以list的形式存在
if (PyList_Check(py_obj_state_names))
    printf("py_obj_state_names is a list!");

PyObject* state_name1 = PyList_GetItem(py_obj_state_names, 0);
PyObject* state_name2 = PyList_GetItem(py_obj_state_names, 1);
PyObject* state_name3 = PyList_GetItem(py_obj_state_names, 2);

printf(PyString_AsString(state_name1));


// 暂停看结果
//getchar();

// 
Py_Finalize();
return 0;
```

以上代码段是主程序当中的，可以很好地实现列表中的功能。

### 2.2 自编程序与UR机器人展开互动
