// ControlLoop.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Python.h>
#include <Windows.h>

double* setp_to_list(double* setp)
{
	return NULL;
}


int main()
{
	PyObject *PM_rtde_config;
	PyObject *PM_rtde;

	PyObject *pClass_conf;
	PyObject *pArgs_init_conf;				//ConfigFile的入口参数

	Py_Initialize();

	// 在python中相当于import
	PM_rtde = PyImport_Import(PyString_FromString("rtde.rtde"));
	PM_rtde_config = PyImport_Import(PyString_FromString("rtde.rtde_config"));
	
	// 调取模块的字典列表
	PyObject* pDict_rtde = PyModule_GetDict(PM_rtde);
	PyObject* pDict_config = PyModule_GetDict(PM_rtde_config);

	// 对应原example_control_loop.py 42行，获取ConfigFile并建立conf实例
	pClass_conf = PyDict_GetItemString(pDict_config, "ConfigFile");
	pArgs_init_conf = Py_BuildValue("(s)", "control_loop_configuration_liu_Test001.xml");		// 构造输入参数
	PyObject* pInstance_conf = PyObject_CallObject(pClass_conf, pArgs_init_conf);

	// 对应43行，调用成员函数 get_recipe
	PyObject* p_Func_get_recipe = PyObject_GetAttrString(pInstance_conf, "get_recipe");
	PyObject* p_res_state = PyEval_CallObject(p_Func_get_recipe, Py_BuildValue("(s)", "state"));
	//PyObject* py_obj_state_names = PyTuple_GetItem(p_res_state, 0);
	//PyObject* py_obj_state_types = PyTuple_GetItem(p_res_state, 1);

	// 44行
	PyObject* p_res_setp = PyEval_CallObject(p_Func_get_recipe, Py_BuildValue("(s)", "setp"));
	//PyObject* py_obj_setp_names = PyTuple_GetItem(p_res_setp, 0);
	//PyObject* py_obj_setp_types = PyTuple_GetItem(p_res_setp, 1);

	// 45行
	PyObject* p_res_watchdog = PyEval_CallObject(p_Func_get_recipe, Py_BuildValue("(s)", "watchdog"));
	//PyObject* py_obj_watchdog_names = PyTuple_GetItem(p_res_watchdog, 0);
	//PyObject* py_obj_watchdog_types = PyTuple_GetItem(p_res_watchdog, 1);

	// 47行，获取RTDE这个类并建立实例con
	PyObject* pClass_RTDE = PyDict_GetItemString(pDict_rtde, "RTDE");
	PyObject* pArgs_init_RTDE = PyTuple_New(2);
	PyTuple_SetItem(pArgs_init_RTDE, 0, PyString_FromString("192.168.1.5"));		//这个要根据实际的计算机IP进行设置
	PyTuple_SetItem(pArgs_init_RTDE, 1, PyLong_FromLong(30004));
	PyObject* pInstance_con = PyObject_CallObject(pClass_RTDE, pArgs_init_RTDE);


	// 48行，实现连接！
	PyEval_CallObject(
		PyObject_GetAttrString(pInstance_con, "connect"), 
		NULL);
	
	// 51行那个看版本似乎没什么用啊
	PyEval_CallObject(
		PyObject_GetAttrString(pInstance_con, "get_controller_version"), 
		NULL);
	
	// 54设置输入输出
	PyEval_CallObject(
		PyObject_GetAttrString(pInstance_con, "send_output_setup"),
		p_res_state);
	PyObject* setp = PyEval_CallObject(
		PyObject_GetAttrString(pInstance_con, "send_input_setup"),
		p_res_setp);
	PyObject* watchdog = PyEval_CallObject(
		PyObject_GetAttrString(pInstance_con, "send_input_setup"),
		p_res_watchdog);

	double setp1[] = { -0.12, -0.43, 0.14, 0, 3.11, 0.04 };
	double setp2[] = { -0.12, -0.51, 0.21, 0, 3.11, 0.04 };

	// 尝试设置setp和watchdog的成员变量，这部分完全是瞎猜的，通不过也很正常
	int execute_res;

	// 这是标准版例程的xml
	/*execute_res = PyObject_SetAttrString(setp, "input_double_register_0", PyFloat_FromDouble(0.0));
	printf("Execute result is %d\n", execute_res);
	PyObject_SetAttrString(setp, "input_double_register_1", PyFloat_FromDouble(0.0));
	PyObject_SetAttrString(setp, "input_double_register_2", PyFloat_FromDouble(0.0));
	PyObject_SetAttrString(setp, "input_double_register_3", PyFloat_FromDouble(0.0));
	PyObject_SetAttrString(setp, "input_double_register_4", PyFloat_FromDouble(0.0));
	PyObject_SetAttrString(setp, "input_double_register_5", PyFloat_FromDouble(0.0));
	PyObject_SetAttrString(watchdog, "input_int_register_0", PyInt_FromLong(0));*/

	

	// 这是用于测试的xml
	int input;
	PyObject* send_start_res;
	printf("Do you want to start work?[1 for yes, 0 for no]\n");
	scanf_s("%d", &input);
	if (1 == input)
	{
		execute_res = PyObject_SetAttrString(
			setp,
			"input_bit_registers0_to_31",
			PyInt_FromLong(1));
		send_start_res = 
			PyEval_CallObject(
			PyObject_GetAttrString(pInstance_con, "send_start"),
			NULL);
		printf("Send_start_result is %d\n", PyInt_AsLong(send_start_res));
		if (0 == PyInt_AsLong(send_start_res))
			printf("Failed!!!\n");
		PyObject* tuple_setp = PyTuple_New(1);
		PyTuple_SetItem(tuple_setp, 0, setp);
		PyEval_CallObject(
			PyObject_GetAttrString(pInstance_con, "send"),
			tuple_setp);
	}
	printf("Before while\n");
	PyObject* state;
	PyObject* state_output;
	int state_outputC;
	while (true)
	{
		Sleep(2000);
		printf("In while 1\n");
		state = PyEval_CallObject(
			PyObject_GetAttrString(pInstance_con, "receive"),
			NULL);
		printf("In while 2\n");
		if (NULL == state)
		{
			printf("Cannot get state! Quit!\n");
			printf("In while 21\n");
			PyEval_CallObject(
				PyObject_GetAttrString(pInstance_con, "send_pause"),
				NULL);
			printf("In while 22\n");
			break;
		}
		printf("In while 3\n");
		state_output = 
			PyObject_GetAttrString(state,
			"output_bit_registers0_to_31");
		if (NULL == state_output)
		{
			printf("Cannot get state_output! Quit!");
			PyEval_CallObject(
				PyObject_GetAttrString(pInstance_con, "send_pause"),
				NULL);
			break;
		}
		state_outputC = PyInt_AsLong(state_output);
		printf("state_output is %d\n", state_outputC);
		if (1 == state_outputC)
			break;
	}
	
	PyEval_CallObject(
		PyObject_GetAttrString(pInstance_con, "send_pause"),
		NULL);


	PyEval_CallObject(
		PyObject_GetAttrString(pInstance_con, "disconnect"),
		NULL);

	
	Py_Finalize();

	getchar();

	return 0;
}

