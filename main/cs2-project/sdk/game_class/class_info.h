#pragma once

class c_main_class_information
{
public:
	const char* designer_class;
	const char* class_;
	const char* name_class;
	void* unk;
	c_main_class_information* next_class;
};

class c_second_class_information
{
public:
	char pad[0x28];
	c_main_class_information* main_class_information;
};

class c_unkown_information
{
public:
	char pad[0x8];
	c_second_class_information* second_class_information;
	char pad2[0x10];
	const char* first_name;
	char pad3[0x10];
	int number;
};