#include "_Container.hpp"

ContainerMapBase::ContainerMapBase() = default;
ContainerMapBase::~ContainerMapBase() = default;

ContainerMapBase::value_type ContainerMapBase::find(
	ContainerMapBase::const_key_type key) const
{
	auto const i = this->Items.find(key);
	if(i != this->Items.end()) {
		return i->second;
	}
	return nullptr;
}

void ContainerMapBase::insert(
	ContainerMapBase::const_key_type key, ContainerMapBase::value_type value)
{
	this->Items.emplace(key, value);
}

ContainerMapBase::value_type ContainerMapBase::remove(
	ContainerMapBase::const_key_type key)
{
	auto const i = this->Items.find(key);
	if(i != this->Items.cend()) {
		auto const value = i->second;
		this->Items.erase(i);
		return value;
	}
	return nullptr;
}

void ContainerMapBase::clear()
{
	// this leaks all objects inside. this case is logged.
	this->Items.clear();
}
