#ifndef BUCKET_STORAGE_HPP
#define BUCKET_STORAGE_HPP

#include "BaseBlock.hpp"
#include <type_traits>

#include <cstddef>
#include <stdexcept>
#include <utility>

// ------------------------------------
// START OF BUCKET_STORAGE INTERFACE
// ------------------------------------

template< typename T >
class BucketStorage
{
	template< bool isConst >
	class BaseIterator;

	using BaseBlock = details::BaseBlock;

  public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type *;
	using reference = value_type &;
	using const_reference = const value_type &;

	explicit BucketStorage(size_type block_capacity = 64);
	BucketStorage(const BucketStorage &other);
	BucketStorage(BucketStorage &&other) noexcept;
	~BucketStorage();

	BucketStorage &operator=(const BucketStorage &other);
	BucketStorage &operator=(BucketStorage &&other) noexcept;

	using iterator = BaseIterator< false >;
	using const_iterator = BaseIterator< true >;

	iterator insert(const value_type &value);
	iterator insert(value_type &&value);
	iterator erase(iterator iterator);

	iterator get_to_distance(iterator it, difference_type distance);

	bool empty() const noexcept;
	size_type size() const noexcept;
	size_type capacity() const noexcept;

	void shrink_to_fit() noexcept;
	void clear() noexcept;
	void swap(BucketStorage &other) noexcept;

	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	const_iterator cbegin() const noexcept;
	iterator end() noexcept;
	const_iterator end() const noexcept;
	const_iterator cend() const noexcept;

	friend class Block;
	friend class details::BaseBlock;

  private:
	class Block;

	BaseBlock fakeNode;
	size_type block_capacity;
	size_type count_elements;
	size_type count_blocks;

	template< typename Z >
	iterator insert_(Z &&value);
	void create_new_block();
	void delete_block(Block *block);
};

template< typename T >
class BucketStorage< T >::Block : public BaseBlock
{
  public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type *;
	using reference = value_type &;
	using const_reference = const value_type &;

	explicit Block(BucketStorage::size_type block_size);

	~Block();

	void swap(Block &other);

	bool empty();

	friend class BucketStorage< T >;

  private:
	pointer data;
	bool *active;
	size_type size;
	size_type block_capacity;
};

// ------------------------------------
// START OF (CONST) ITERATORS INTERFACE
// ------------------------------------

template< typename T >
template< bool isConst >
class BucketStorage< T >::BaseIterator
{
  public:
	using value_type = T;
	using base_pointer = std::conditional_t< isConst, const value_type *, value_type * >;
	using base_reference = std::conditional_t< isConst, const value_type &, value_type & >;
	using difference_type = std::ptrdiff_t;

	BaseIterator();
	BaseIterator(BucketStorage *bucketStorage, BaseBlock *block, size_type index);
	BaseIterator(BucketStorage *bucketStorage, Block *block, size_type index);
	BaseIterator(const BucketStorage *bucketStorage, BaseBlock *block, size_type index);
	BaseIterator(const BucketStorage *bucketStorage, Block *block, size_type index);
	BaseIterator(const BaseIterator< false > &other);
	base_pointer operator->() const;
	base_reference operator*() const;

	BaseIterator &operator++();
	BaseIterator operator++(int);
	BaseIterator &operator--();
	BaseIterator operator--(int);

	bool operator<(const BaseIterator &other) const;
	bool operator>=(const BaseIterator &other) const;
	bool operator>(const BaseIterator &other) const;
	bool operator<=(const BaseIterator &other) const;

	bool operator==(const const_iterator &other) const;
	bool operator!=(const const_iterator &other) const;

	bool operator==(const iterator &other) const;
	bool operator!=(const iterator &other) const;

	friend class BucketStorage;

  private:
	static constexpr size_type npos = -1;
	Block *block;
	BaseBlock *last;
	size_type index;
};

template< typename T >
BucketStorage< T >::Block::Block(BucketStorage::size_type block_size) : size(0), block_capacity(block_size)
{
	active = new bool[block_size]{ false };
	data = static_cast< pointer >(::operator new(block_size * sizeof(value_type)));
}

template< typename T >
void BucketStorage< T >::Block::swap(BucketStorage< T >::Block &other)
{
	using std::swap;
	swap(data, other.data);
	swap(active, other.active);
	swap(size, other.size);
	swap(block_capacity, other.block_capacity);
}

template< typename T >
bool BucketStorage< T >::Block::empty()
{
	return size == 0;
}

template< typename T >
BucketStorage< T >::Block::~Block()
{
	if (data)
	{
		for (size_type i = 0; i < block_capacity; ++i)
			if (active[i])
				data[i].~value_type();

		::operator delete(data);
	}
	delete[] active;
	BaseBlock::prev = nullptr;
	BaseBlock::next = nullptr;
	data = nullptr;
	active = nullptr;
	size = 0;
}
// ------------------------------------
// START OF BUCKET_STORAGE IMPLEMENTATION
// ------------------------------------

template< typename T >
BucketStorage< T >::BucketStorage(size_type block_capacity) :
	block_capacity(block_capacity), fakeNode(), count_elements(0), count_blocks(0)
{
}
template< typename T >
BucketStorage< T >::BucketStorage(const BucketStorage &other) :
	block_capacity(other.block_capacity), fakeNode(), count_elements(0), count_blocks(0)
{
	for (auto it = other.begin(); it != other.end(); ++it)
		insert(*it);
}

template< typename T >
BucketStorage< T >::BucketStorage(BucketStorage &&other) noexcept :
	fakeNode(), block_capacity(0), count_elements(0), count_blocks(0)
{
	using std::swap;
	swap(fakeNode, other.fakeNode);
	swap(block_capacity, other.block_capacity);
	swap(count_elements, other.count_elements);
	swap(count_blocks, other.count_blocks);
}

template< typename T >
BucketStorage< T > &BucketStorage< T >::operator=(BucketStorage< T > &&other) noexcept
{
	if (this != &other)
	{
		clear();
		swap(other);
	}
	return *this;
}
template< typename T >
BucketStorage< T > &BucketStorage< T >::operator=(const BucketStorage< T > &other)
{
	if (this != &other)
	{
		BucketStorage temp = other;
		swap(temp);
	}
	return *this;
}
template< typename T >
void BucketStorage< T >::swap(BucketStorage< T > &other) noexcept
{
	std::swap(fakeNode, other.fakeNode);
	std::swap(block_capacity, other.block_capacity);
	std::swap(count_elements, other.count_elements);
	std::swap(count_blocks, other.count_blocks);
}
template< typename T >
bool BucketStorage< T >::empty() const noexcept
{
	return count_elements == 0;
}
template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::size() const noexcept
{
	return count_elements;
}

template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::capacity() const noexcept
{
	size_type temp = 0;
	Block *curr = static_cast< Block * >(fakeNode.next);
	while (curr != &fakeNode)
	{
		if (!curr->empty())
			temp++;
		curr = static_cast< Block * >(curr->next);
	}
	return temp * block_capacity;
}
template< typename T >
void BucketStorage< T >::clear() noexcept
{
	if (count_elements != 0)
		while (fakeNode.next != &fakeNode)
		{
			Block *temp = static_cast< Block * >(fakeNode.next);
			fakeNode.next = temp->next;
			delete_block(temp);
		}
	fakeNode.prev = &fakeNode;
	fakeNode.next = &fakeNode;
	block_capacity = 0;
	count_elements = 0;
	count_blocks = 0;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::begin() noexcept
{
	Block *temp = static_cast< Block * >(fakeNode.next);
	while (temp != &fakeNode)
	{
		for (int i = 0; i < block_capacity; ++i)
			if (temp->active[i])
				return iterator(this, temp, i);
		temp = static_cast< Block * >(temp->next);
	}
	return end();
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::begin() const noexcept
{
	return const_iterator(const_cast< BucketStorage * >(this)->begin());
}
template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cbegin() const noexcept
{
	return const_iterator(begin());
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::end() noexcept
{
	return iterator(this, &fakeNode, 0);
}
template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::end() const noexcept
{
	return const_iterator(const_cast< BucketStorage * >(this)->end());
}
template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cend() const noexcept
{
	return const_iterator(end());
}
template< typename T >
void BucketStorage< T >::create_new_block()
{
	Block *temp = new Block(block_capacity);
	if (fakeNode.next == &fakeNode)
	{
		fakeNode.next = temp;
		fakeNode.prev = temp;
		temp->next = &fakeNode;
		temp->prev = &fakeNode;
	}
	else
	{
		BaseBlock *tail = fakeNode.prev;
		tail->next = temp;
		temp->prev = tail;
		temp->next = &fakeNode;
		fakeNode.prev = temp;
	}
	count_blocks++;
}

template< typename T >
void BucketStorage< T >::delete_block(BucketStorage::Block *block)
{
	count_blocks--;
	delete block;
}
template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(BucketStorage::value_type &&value)
{
	return insert_(std::move(value));
}
template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(const BucketStorage::value_type &value)
{
	return insert_(value);
}
// Why? Because we have universal reference
// More: https://youtu.be/rDQ_RDe6Ozk?si=6LZZgQkDd48OmeF8&t=4854
template< typename T >
template< typename Z >
typename BucketStorage< T >::iterator BucketStorage< T >::insert_(Z &&value)
{
	Block *temp = static_cast< Block * >(fakeNode.next);
	while (temp != &fakeNode)
	{
		for (size_type i = 0; i < block_capacity; ++i)
			if (temp->active[i] == false)
			{
				new (&temp->data[i]) value_type(std::forward< Z >(value));
				count_elements++;
				temp->active[i] = true;
				temp->size++;
				return iterator(this, temp, i);
			}
		temp = static_cast< Block * >(temp->next);
	}
	create_new_block();
	Block *tail = static_cast< Block * >(fakeNode.prev);
	if (fakeNode.prev == &fakeNode)
		throw std::bad_alloc();
	size_type index_ = tail->size++;
	new (&tail->data[index_]) value_type(std::forward< Z >(value));
	count_elements++;
	tail->active[index_] = true;
	return iterator(this, tail, index_);
}
template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::erase(BucketStorage::iterator it)
{
	if (empty())
		throw std::runtime_error("BucketStorage is empty, nothing to delete");
	it.block->data[it.index].~value_type();
	it.block->active[it.index] = false;
	count_elements--;
	size_type size_ = --it.block->size;
	if (it.block->empty())
	{
		Block *block_next = static_cast< Block * >(it.block->next);
		Block *block_prev = static_cast< Block * >(it.block->prev);

		delete_block(it.block);
		if (block_next == it.last)
		{
			it.block = static_cast< Block * >(it.last);
		}
		else
		{
			block_prev->next = block_next;
			block_next->prev = block_prev;
			it.block = block_next;
			it.index = it.npos;
		}
	}
	return ++it;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::get_to_distance(iterator it, difference_type distance)
{
	while (distance > 0)
	{
		--distance;
		++it;
	}
	while (distance < 0)
	{
		++distance;
		--it;
	}
	return it;
}
template< typename T >
void BucketStorage< T >::shrink_to_fit() noexcept
{
	BucketStorage temp(block_capacity);
	for (auto it = begin(); it != end(); ++it)
	{
		temp.insert(std::move(*it));
	}
	swap(temp);
}
template< typename T >
BucketStorage< T >::~BucketStorage()
{
	clear();
}

// ------------------------------------
// START OF ITERATOR IMPLEMENTATION
// ------------------------------------

template< typename T >
template< bool isConst >
BucketStorage< T >::BaseIterator< isConst >::BaseIterator() : index(0), block(nullptr), last(nullptr)
{
}

template< typename T >
template< bool isConst >
BucketStorage< T >::BaseIterator< isConst >::BaseIterator(BucketStorage *bucketStorage, Block *block, size_type index) :
	index(index), block(block), last(&bucketStorage->fakeNode)
{
}

template< typename T >
template< bool isConst >
BucketStorage< T >::BaseIterator< isConst >::BaseIterator(const BucketStorage *bucketStorage, Block *block, size_type index) :
	index(index), block(block), last(const_cast< BaseBlock * >(&bucketStorage->fakeNode))
{
}

template< typename T >
template< bool isConst >
BucketStorage< T >::BaseIterator< isConst >::BaseIterator(BucketStorage *bucketStorage, BaseBlock *block, size_type index) :
	index(index), block(static_cast< Block * >(block)), last(block)
{
}

template< typename T >
template< bool isConst >
BucketStorage< T >::BaseIterator< isConst >::BaseIterator(const BucketStorage *bucketStorage, BaseBlock *block, size_type index) :
	index(index), block(static_cast< Block * >(block)), last(const_cast< BaseBlock * >(block))
{
}
template< typename T >
template< bool isConst >
BucketStorage< T >::BaseIterator< isConst >::BaseIterator(const BaseIterator< false > &other) :
	index(other.index), block(other.block), last(other.last)
{
}

template< typename T >
template< bool isConst >
typename BucketStorage< T >::template BaseIterator< isConst >::base_reference
	BucketStorage< T >::BaseIterator< isConst >::operator*() const
{
	return block->data[index];
}

template< typename T >
template< bool isConst >
typename BucketStorage< T >::template BaseIterator< isConst >::base_pointer
	BucketStorage< T >::BaseIterator< isConst >::operator->() const
{
	return &block->data[index];
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator==(const const_iterator &other) const
{
	return block == other.block && index == other.index;
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator!=(const const_iterator &other) const
{
	return !(*this == other);
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator==(const iterator &other) const
{
	return block == other.block && index == other.index;
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator!=(const iterator &other) const
{
	return !(*this == other);
}
template< typename T >
template< bool isConst >
typename BucketStorage< T >::template BaseIterator< isConst > &BucketStorage< T >::BaseIterator< isConst >::operator++()
{
	while (block != last)
	{
		if (index + 1 < block->block_capacity | index == npos)
		{
			if (index == npos)
			{
				index = 0;
				if (block->active[index])
				{
					return *this;
				}
			}
			else if (block->active[++index])
				return *this;
		}
		else
		{
			block = static_cast< Block * >(block->next);
			index = npos;
		}
	}
	index = 0;
	return *this;
}

template< typename T >
template< bool isConst >
typename BucketStorage< T >::template BaseIterator< isConst > BucketStorage< T >::BaseIterator< isConst >::operator++(int)
{
	BaseIterator< isConst > temp = *this;
	++(*this);
	return temp;
}
template< typename T >
template< bool isConst >
typename BucketStorage< T >::template BaseIterator< isConst > &BucketStorage< T >::BaseIterator< isConst >::operator--()
{
	while (block != last->next)
	{
		if (index > 0)
		{
			if (block->active[--index])
				return *this;
		}
		else
		{
			block = static_cast< Block * >(block->prev);
			index = block->block_capacity;
		}
	}
	--index;
	block = static_cast< Block * >(last->next);
	return *this;
}

template< typename T >
template< bool isConst >
typename BucketStorage< T >::template BaseIterator< isConst > BucketStorage< T >::BaseIterator< isConst >::operator--(int)
{
	BaseIterator< isConst > temp = *this;
	--(*this);
	return temp;
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator<=(const BaseIterator &other) const
{
	return block == other.block && index <= other.index;
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator>(const BaseIterator &other) const
{
	return !(*this <= other);
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator<(const BaseIterator &other) const
{
	return block == other.block && index < other.index;
}

template< typename T >
template< bool isConst >
bool BucketStorage< T >::BaseIterator< isConst >::operator>=(const BaseIterator &other) const
{
	return !(*this < other);
}

#endif	  //  BUCKET_STORAGE_HPP
