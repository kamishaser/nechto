#pragma once
#include "node.h"
#include "baseValueTypes.h"

#include <functional>
#include <set>
//��������! �������������! ��� ��� ������ ��������!!!

namespace nechto
{
	class externalFunction
	{
	private:
		
	public:
		const std::string name;
		mutable std::function<bool(nodePtr)> isCorrect;
		mutable nodeEvent Func = nullptr;

		externalFunction(std::string fname, std::function<bool(nodePtr)> check = [](nodePtr c) {return false; }, nodeEvent Function = nullptr)
			:name(fname),isCorrect(check), Func(Function)
		{}
		
		operator bool(){ return Func != nullptr; }   const

		bool operator==(const externalFunction& fun2)const
		{return name == fun2.name;}
		bool operator!=(const externalFunction& fun2)const
		{return name != fun2.name;}
		bool operator< (const externalFunction& fun2)const
		{return name < fun2.name;}
		bool operator> (const externalFunction& fun2)const
		{return name > fun2.name;}
		bool operator<=(const externalFunction& fun2)const
		{return name <= fun2.name;}
		bool operator>=(const externalFunction& fun2)const
		{return name >= fun2.name;}
	};

	static std::set<externalFunction> funSet{externalFunction("error")};
	static std::mutex setBlock;

	externalFunction addExternalFunction(const externalFunction newFun) noexcept
	{
		setBlock.lock();
		if (funSet.contains(newFun))
		{//����������� ������ �� �����
			auto ExtFun = funSet.find(newFun);//������� �����������(�������� ������ �� �����)
			externalFunction old(newFun.name, ExtFun->isCorrect, ExtFun->Func);
			ExtFun->isCorrect = newFun.isCorrect;
			ExtFun->Func = newFun.Func;
			setBlock.unlock();
			return old;
		}
		funSet.emplace(newFun);
		setBlock.unlock();
		return externalFunction("empty", nullptr, nullptr);
	}
	bool isExternalFunctionExist(const std::string& name) noexcept
	{
		const externalFunction temp(name, nullptr, nullptr);//������ ������� �������
		setBlock.lock();
		bool result = funSet.contains(temp);//����������� ������ �� �����
		setBlock.unlock();
		return result;
	}
	const externalFunction* getExternalFunction(const std::string& name) noexcept
	{
		const externalFunction temp(name, nullptr, nullptr);//������ ������� �������
		setBlock.lock();
		const externalFunction* function = (funSet.contains(temp)) ? &(*funSet.find(temp)) : nullptr;
		//����������� ������ �� �����
		setBlock.unlock();
		return function;
	}
}
