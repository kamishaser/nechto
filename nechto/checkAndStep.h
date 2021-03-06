#pragma once
#include "node.h"
#include "tag.h"
#include "mathOperator.h"
#include "nodeOperations.h"
#include "externalFunction.h"
#include "Pointer.h"

namespace nechto
{
	bool isAction(nodePtr v1)
	{
		switch (v1->getType())
		{
		case node::MathOperator:
		case node::ConditionalBranching:
		case node::ExternalFunction:
			return true;
		default:
			return false;
		}
	}
	bool nodePtr::check() const
	{
		nodePtr temp = *this;
		if (temp->correctness?hecked.load())
			return true;
		switch (temp->getType())
		{
		case node::Error:
			temp->correctness?hecked = false;
			break;
		case node::Hub:
			temp->correctness?hecked = hub::getParrent(*this).exist();
			break;
		case node::Variable:
			temp->correctness?hecked = (!temp->hasConnection(0) && !temp->hasConnection(1)
				 && !temp->hasConnection(2) && !temp->hasConnection(3)
				 && temp->getSubtype() != baseValueType::Error);
			break;
		case node::MathOperator:
			temp->correctness?hecked = mathOperator::check(*this);
			break;
		case node::Tag:
			temp->correctness?hecked = tag::check(*this);
			break;
		case node::ConditionalBranching:
			temp->correctness?hecked = ((temp->hasConnection(0)) && (temp->connection[0].load()->getType() == node::Variable));
			break;
		case node::ExternalFunction:
			if (temp->getData<externalFunction*>() == nullptr)
				temp->correctness?hecked = false;
			else if (temp->getData<externalFunction*>()->check((*this)))
				temp->correctness?hecked = true;
			break;
		case node::Pointer:
			temp->correctness?hecked = pointer::check((*this));
			break;
		default:
			assert(false);
		}
		return temp->correctness?hecked;
	}
	
	nodePtr step(nodePtr flag)
	{
		assert(flag.check());
		assert(isAction(flag));
		nodePtr nextPosition;
		switch (flag->getType())
		{
		case node::MathOperator:
			mathOperator::mathOperation(flag);
			nextPosition = flag->connection[3].load();
			if (!nextPosition.exist())
				return nullNodePtr;
			return nextPosition;
		case node::TypeCastOperator:
			typeCast(flag);
			nextPosition = flag->connection[3].load();
			if (!nextPosition.exist())
				return nullNodePtr;
			return nextPosition;
		case node::ConditionalBranching:
			nextPosition = (boolCast(flag->connection[0])) ? flag->connection[1] : flag->connection[2];
			if (!nextPosition.exist())
				return nullNodePtr;
			return nextPosition;
		case node::ExternalFunction:
			(flag->getData<externalFunction*>())->perform(flag);
			nextPosition = flag->connection[3].load();
			if (!nextPosition.exist())
				return nullNodePtr;
			return nextPosition;
		default: throw;
		}
	}
}