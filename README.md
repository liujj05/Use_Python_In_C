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

## 2 开发步骤

### 2.1 Test001

在C中尝试调用rtde中的一些不需要连接机器人的功能。具体参照
[C语言调用Python脚本中的函数](http://blog.csdn.net/hexiaomin_1984/article/details/37969193)来建立这个功能。起初代码编写完成之后，编译通过但是链接失败，网上检索后根据[在VS2012中C++嵌入python](http://blog.csdn.net/haimengao/article/details/16867547)一文发现，64位Python的机器上，工程必须设定在x64环境下编译，否则就会链接失败。

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

想办法把它用C实现。