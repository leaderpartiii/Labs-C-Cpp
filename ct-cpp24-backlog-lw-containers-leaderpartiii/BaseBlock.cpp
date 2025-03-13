#include "BaseBlock.hpp"

//  ------------------------------------
//  START OF BASE_BLOCKS DEFINITION
//  ------------------------------------

#include <utility>
namespace details
{
	BaseBlock::BaseBlock() : prev(this), next(this) {}
	BaseBlock::BaseBlock(BaseBlock *prev, BaseBlock *next) : prev(prev), next(next) {}
	BaseBlock::BaseBlock(BaseBlock &&other) noexcept
	{
		moveBaseBlock(std::move(other));
	}
	BaseBlock::BaseBlock(const BaseBlock &other)
	{
		prev = other.prev;
		next = other.next;
	}
	BaseBlock &BaseBlock::operator=(BaseBlock &&other) noexcept
	{
		if (this != &other)
		{
			if (prev)
				prev->next = next;
			if (next)
				next->prev = prev;
			moveBaseBlock(std::move(other));
		}
		return *this;
	}
	BaseBlock &BaseBlock::operator=(const BaseBlock &other)
	{
		if (this != &other)
		{
			prev = other.prev;
			next = other.next;
		}
		return *this;
	}
	void BaseBlock::moveBaseBlock(BaseBlock &&other)
	{
		prev = other.prev;
		next = other.next;
		if (other.prev)
			other.prev->next = this;
		if (other.next)
			other.next->prev = this;
		other.prev = &other;
		other.next = &other;
	}
}	 // namespace details
