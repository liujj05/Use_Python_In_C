// Test001.cpp: 定义控制台应用程序的入口点。
//

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
	pArgs = PyTuple_New(1);		// 构造函数输入参数
	PyTuple_SetItem(pArgs, 0, PyString_FromString("control_loop_configuration.xml"));
	PyObject* pInstance_ConfigFile = PyInstance_New(pClass_ConfigFile, NULL, NULL);
	/*
	// 构造一个ConfigFile类的对象
	// 1. 获取ConfigFile类
	pClass_ConfigFile = PyDict_GetItemString(pDict, "ConfigFile");
	// 2. 构造ConfigFile类的一个对象
	pArgs = PyTuple_New(1);		// 构造函数输入参数
	PyTuple_SetItem(pArgs, 0, PyString_FromString("control_loop_configuration.xml"));
	PyObject* pInstance_ConfigFile = PyInstance_New(pClass_ConfigFile, NULL, NULL);

	//getchar();			//为了暂停一下看结果
	*/
	
	/*
	// 执行程序
	pValue = PyObject_CallObject(pFunc_ConfigFile, pArgs);

	// 解析出口参数
	*/

	Py_Finalize();
    return 0;
}

