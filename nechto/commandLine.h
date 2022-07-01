#pragma once
#include "textOut.h"
#include "fileStream.h"
#include "externalConnection.h"
#include <filesystem>

using namespace nechto;

void loadNode(nodePtr v1)
{
	std::cout << nodeProperties(v1) << std::endl;
}
class commandLine
{
	externalConnection v1;
	std::string cutWord				(std::string& line);
	std::string commandNew			(std::string& line);
	std::string commandDelete		(std::string& line);
	std::string commandHubHub		(std::string& line);
	std::string commandNumHub		(std::string& line);
	std::string commandHubNum		(std::string& line);
	std::string commandGo			(std::string& line);
	std::string commandThis			(std::string& line);
	std::string commandStep			(std::string& line);
	std::string commandSet			(std::string& line);
	std::string commandSetType		(std::string& line);
	std::string commandSetSubtype	(std::string& line);
	std::string commandSetData		(std::string& line);
	std::string commandIsCorrect	(std::string& line);
	std::string commandConnections	(std::string& line);
	std::string commandDisconnect	(std::string& line);

	fileStream filehan;
	std::string commandSave			(std::string& line);
	std::string commandIsSaved		(std::string& line);
	std::string commandEnd		(std::string& line);
	std::string commandLoad			(std::string& line);
public:
	nodePtr stoptr(std::string& line);

	commandLine()
		:v1(nullNodePtr, std::string("commandLine")){}
	std::string operator()(std::string line)
	{
		std::string command;
		command = cutWord(line);
		if (command == "new")			return commandNew(line);
		if (command == "delete")		return commandDelete(line);
		if (command == "hubhub")		return commandHubHub(line);
		if (command == "numhub")		return commandNumHub(line);
		if (command == "hubnum")		return commandHubNum(line);
		if (command == "connections")	return commandConnections(line);
		if (command == "disconnect")	return commandDisconnect(line);
		if (command == "go")			return commandGo(line);
		if (command == "this")			return commandThis(line);
		if (command == "step") 			return commandStep(line);
		if (command == "setData") 		return commandSetData(line);
		if (command == "setType")		return commandSetType(line);
		if (command == "setSubtype")	return commandSetSubtype(line);
		if (command == "isCorrect")		return commandIsCorrect(line);

		if (command == "save")			return commandSave(line);
		if (command == "isSaved")		return commandIsSaved(line);
		if (command == "end")			return commandEnd(line);
		if (command == "load")			return commandLoad(line);

		
		return "unknown command";

	}
};

std::string commandLine::cutWord(std::string& line)
{
	size_t separator= line.find(' ');
	if (separator == -1)
		return line;
	std::string word = line.substr(0, separator);
	while ((separator < line.size()) && line[separator] == ' ')
		++separator;
	line.erase(line.begin(), line.begin() + separator);
	return word;
}

nodePtr commandLine::stoptr(std::string& line)
{
	nodePtr address;
	try
	{
		address.first = std::stoi(cutWord(line));
		address.second = std::stoi(cutWord(line));
	}
	catch (std::exception)
	{
		return nullNodePtr;
	}
	if (!address.exist())
		return nullNodePtr;
	return address;
}

std::string commandLine::commandNew(std::string& line)
{
	std::string word;
	v1 = newNode();
	if (line.empty())
		return to_string(v1);
	word = cutWord(line);
	char type = typeName::find(typeName::nodeT, word);
	if (type == 0)
		return (to_string(v1) + "unknownType: " + word);
	if (hasSubType(v1))
	{
		char subtype;
		if (line.empty())
			return to_string(v1);
		word = cutWord(line);
		subtype = typeName::findSubtype(type, word);
		if (type == 0)
			return (to_string(v1) + "unknownSubype: " + word);
		setTypeAndSubtype(v1, type, subtype);
	}
	setTypeAndSubtype(v1, type);
	return to_string(v1);
}

std::string commandLine::commandDelete(std::string& line)
{
	if (line.empty())
	{
		deleteNode(v1);
		return to_string(v1) + " was deleted";
	}
	nodePtr temp = stoptr(line);
	if (temp == nullNodePtr)
		return "error";
	deleteNode(temp);
	return to_string(temp) + " was deleted";
}
std::string commandLine::commandSetType(std::string& line)
{
	if (line.empty())
		return "error";
	char type = typeName::find(typeName::nodeT, line);
	if (type == 0)
		return (to_string(v1) + "unknownType: " + line);
	setTypeAndSubtype(v1, type);
	return nodeProperties(v1);
}
std::string commandLine::commandSetSubtype(std::string& line)
{
	if (line.empty())
		return "error";
	char subtype = typeName::findSubtype(v1->getType(), line);
	if (subtype == 0)
		return (to_string(v1) + "unknownSubype: " + line);
	setTypeAndSubtype(v1, v1->getType(), subtype);
	return nodeProperties(v1);
}
std::string commandLine::commandSetData(std::string& line)
{
	if (line.empty())
		return "error";
	nodePtr ptemp;
	int64_t itemp = 0;
	double  dtemp = 0;
	switch (v1->getType())
	{
	case node::Variable:
		switch (v1->getSubtype())
		{
		case baseValueType::Int64:
			try { itemp = std::stoll(line); }
			catch (...) { return "error"; }
			v1->setData<int64_t>(itemp);
			return commandThis(line);
		case baseValueType::Double:
			try { dtemp = std::stod(line); }
			catch (...) { return "error"; }
			v1->setData<double>(dtemp);
			return commandThis(line);
		default:
			return "error";
		}
	case node::Tag:
		tag::setData(v1, line);
		return nodeProperties(v1);
	case node::ExteralFunction:
		if (!externalFunction::exist(line))
			return "the function named " + line + " isn't exist";
		v1->setData(externalFunction::get(line));
		return nodeProperties(v1);

	default:
		break;
	}
	return commandThis(line);
}
std::string commandLine::commandHubHub(std::string& line)
{
	nodePtr v2 = stoptr(line);
	if (!v2.exist())
		return "error";
	HubHubConnect(v1, v2);
	return to_string(v1);
}
std::string commandLine::commandNumHub(std::string& line)
{
	nodePtr v2 = stoptr(line);
	if (!v2.exist())
		return "error";
	ushort nc1;
	try 
	{ 
		nc1 = std::stoi(cutWord(line)); 
		NumHubConnect(v1, v2, nc1);
	}
	catch (std::exception e)
	{
		return e.what();
	}
	
	return to_string(v1);
}
std::string commandLine::commandHubNum(std::string& line)
{
	nodePtr v2 = stoptr(line);
	if (!v2.exist())
		return "error";
	ushort nc1;
	try 
	{ 
		nc1 = std::stoi(cutWord(line)); 
		NumHubConnect(v2, v1, nc1);
	}
	catch (std::exception e)
	{
		return e.what();
	}
	
	return to_string(v1);
}
std::string commandLine::commandGo(std::string& line)
{
	nodePtr v2 = stoptr(line);
	if (!v2.exist())
		return "error";
	v1 = v2;
	return to_string(v1) + ' ' + nodeType(v1) + ' ' + nodeSubtype(v1);
}
std::string commandLine::commandThis(std::string& line)
{
	return nodeProperties(v1);
}
std::string commandLine::commandConnections(std::string& line)
{
	std::string temp;
	nodePtr vTemp = v1;
	while (true)
	{
		for (int i = 0; i < 4; i++)
			temp = temp + nodeProperties(vTemp->connection[i].load()) + '\n';
		if (!isHubExist(vTemp))
			return temp + "\nnH";
		vTemp = v1->hubConnection;
		temp += "\n";
	}
}
std::string commandLine::commandDisconnect(std::string& line)
{
	nodePtr v2 = stoptr(line);
	if (!v2.exist())
		return "error";
	disconnect(v1, v2);
	return to_string(v1);
}
// 
std::string commandLine::commandStep(std::string& line)
{
	if (!isCorrect(v1))
		return "the node isn't correct";
	nodePtr nPos = step(v1);
	if (nPos.exist())
		v1 = nPos;
	else
		return "end";
	return commandThis(line);
}
//std::string commandLine::commandSet(std::string& line)
std::string commandLine::commandIsCorrect(std::string& line)
{
	if (isCorrect(v1))
		return "the node is correct";
	else
		return "the node isn't correct";
}

std::string commandLine::commandSave(std::string& line)
{
	std::filesystem::path path(line);
	if (!filehan.isOpen())
		filehan.saveStart(path);
	if (!filehan.isOpen())
		return "error";
	if (!filehan.isSaved(v1))
		filehan.save(v1);
	else return "error";
	return "success";
}
std::string commandLine::commandEnd(std::string& line)
{
	if (!filehan.isOpen())
		return "error";
	filehan.end();
	return "success";
}
std::string commandLine::commandLoad(std::string& line)
{
	std::filesystem::path path(line);
	if (!filehan.isOpen())
		filehan.loadStart(path);
	if (!filehan.isOpen())
		return "file " + path.string() + " isn't detected";
	return nodeProperties(filehan.load());
}
std::string commandLine::commandIsSaved(std::string& line)
{
	if (filehan.isSaved(v1))
		return "true";
	else
		return "false";
}