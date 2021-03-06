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
}

