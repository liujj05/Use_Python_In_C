// Test001.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Python.h>

int main()
{
	PyObject *PM_rtde_config;
	PyObject *pFunc_ConfigFile;
	PyObject *pArgs;				//ConfigFile的入口参数

	Py_Initialize();
	
	// 在python中相当于import了这个文件
	PM_rtde_config = PyImport_Import(PyString_FromString("rtde.rtde_config"));
	
	// 获取到ConfigFile这个函数
	pFunc_ConfigFile = PyObject_GetAttrString(PM_rtde_config, "ConfigFile");

	// ConfigFile函数的入口参数只有一个string：config_filename
	pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, PyString_FromString("control_loop_configuration.xml"));

	Py_Finalize();
    return 0;
}

