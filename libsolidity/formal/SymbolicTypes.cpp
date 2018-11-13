/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libsolidity/formal/SymbolicTypes.h>

#include <libsolidity/ast/Types.h>

#include <memory>

using namespace std;
using namespace dev::solidity;

smt::SortPointer dev::solidity::smtSort(Type const& _type)
{
	auto const& category = _type.category();
	if (isNumber(category))
		return make_shared<smt::Sort>(smt::Kind::Int);
	else if (isBool(category))
		return make_shared<smt::Sort>(smt::Kind::Bool);
	else if (isMapping(category))
	{
		auto mapType = dynamic_cast<MappingType const*>(&_type);
		solAssert(mapType, "");
		return make_shared<smt::ArraySort>(smtSort(*mapType->keyType()), smtSort(*mapType->valueType()));
	}
	else if (isFunction(category))
	{
		auto fType = dynamic_cast<FunctionType const*>(&_type);
		solAssert(fType, "");
		vector<smt::SortPointer> parameterSorts = smtSort(fType->parameterTypes());
		auto returnTypes = fType->returnParameterTypes();
		solAssert(returnTypes.size() == 1, "");
		smt::SortPointer returnSort = smtSort(*returnTypes.at(0));
		return make_shared<smt::FunctionSort>(parameterSorts, returnSort);
	}
	solAssert(false, "Invalid type");
}

vector<smt::SortPointer> dev::solidity::smtSort(vector<TypePointer> const& _types)
{
	vector<smt::SortPointer> sorts;
	for (auto const& type: _types)
		sorts.push_back(smtSort(*type));
	return sorts;
}

smt::Kind dev::solidity::smtKind(Type::Category _category)
{
	if (isNumber(_category))
		return smt::Kind::Int;
	else if (isBool(_category))
		return smt::Kind::Bool;
	else if (isMapping(_category))
		return smt::Kind::Array;
	else if (isFunction(_category))
		return smt::Kind::Function;
	solAssert(false, "Invalid type");
}

bool dev::solidity::isSupportedType(Type::Category _category)
{
	return isNumber(_category) ||
		isArray(_category) ||
		isBool(_category) ||
		isFunction(_category);
}

pair<bool, shared_ptr<SymbolicVariable>> dev::solidity::newSymbolicVariable(
	Type const& _type,
	std::string const& _uniqueName,
	smt::SolverInterface& _solver
)
{
	bool abstract = false;
	shared_ptr<SymbolicVariable> var;
	TypePointer type = _type.shared_from_this();
	if (!isSupportedType(_type))
	{
		abstract = true;
		var = make_shared<SymbolicIntVariable>(make_shared<IntegerType>(256), _uniqueName, _solver);
	}
	else if (isMapping(_type.category()))
		var = make_shared<SymbolicMappingVariable>(type, _uniqueName, _solver);
	else if (isBool(_type.category()))
		var = make_shared<SymbolicBoolVariable>(type, _uniqueName, _solver);
	else if (isFunction(_type.category()))
		var = make_shared<SymbolicIntVariable>(make_shared<IntegerType>(256), _uniqueName, _solver);
	else if (isInteger(_type.category()))
		var = make_shared<SymbolicIntVariable>(type, _uniqueName, _solver);
	else if (isFixedBytes(_type.category()))
	{
		auto fixedBytesType = dynamic_cast<FixedBytesType const*>(type.get());
		solAssert(fixedBytesType, "");
		var = make_shared<SymbolicFixedBytesVariable>(fixedBytesType->numBytes(), _uniqueName, _solver);
	}
	else if (isAddress(_type.category()))
		var = make_shared<SymbolicAddressVariable>(_uniqueName, _solver);
	else if (isRational(_type.category()))
	{
		auto rational = dynamic_cast<RationalNumberType const*>(&_type);
		solAssert(rational, "");
		if (rational->isFractional())
			var = make_shared<SymbolicIntVariable>(make_shared<IntegerType>(256), _uniqueName, _solver);
		else
			var = make_shared<SymbolicIntVariable>(type, _uniqueName, _solver);
	}
	else
		solAssert(false, "");
	return make_pair(abstract, var);
}

bool dev::solidity::isSupportedType(Type const& _type)
{
	return isSupportedType(_type.category());
}

bool dev::solidity::isInteger(Type::Category _category)
{
	return _category == Type::Category::Integer;
}

bool dev::solidity::isRational(Type::Category _category)
{
	return _category == Type::Category::RationalNumber;
}

bool dev::solidity::isFixedBytes(Type::Category _category)
{
	return _category == Type::Category::FixedBytes;
}

bool dev::solidity::isAddress(Type::Category _category)
{
	return _category == Type::Category::Address;
}

bool dev::solidity::isNumber(Type::Category _category)
{
	return isInteger(_category) ||
		isRational(_category) ||
		isFixedBytes(_category) ||
		isAddress(_category);
}

bool dev::solidity::isBool(Type::Category _category)
{
	return _category == Type::Category::Bool;
}

bool dev::solidity::isFunction(Type::Category _category)
{
	return _category == Type::Category::Function;
}

bool dev::solidity::isMapping(Type::Category _category)
{
	return _category == Type::Category::Mapping;
}

bool dev::solidity::isArray(Type::Category _category)
{
	// In the future will also support arrays.
	return isMapping(_category);
}

smt::Expression dev::solidity::minValue(IntegerType const& _type)
{
	return smt::Expression(_type.minValue());
}

smt::Expression dev::solidity::maxValue(IntegerType const& _type)
{
	return smt::Expression(_type.maxValue());
}
