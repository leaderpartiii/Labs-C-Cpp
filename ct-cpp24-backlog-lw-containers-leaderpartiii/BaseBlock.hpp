#ifndef BASE_BLOCK_HPP
#define BASE_BLOCK_HPP

// ------------------------------------
// START OF BASE_BLOCK IMPLEMENTATION
// ------------------------------------

template< typename T >
class BucketStorage;

namespace details
{

	class BaseBlock
	{
	  public:
		explicit BaseBlock();
		explicit BaseBlock(BaseBlock *prev, BaseBlock *next);

		BaseBlock(BaseBlock &&other) noexcept;
		BaseBlock(const BaseBlock &other);

		BaseBlock &operator=(BaseBlock &&other) noexcept;
		BaseBlock &operator=(const BaseBlock &other);

		void moveBaseBlock(BaseBlock &&other);

		~BaseBlock() = default;

		template< typename T >
		friend class BucketStorage;

		BaseBlock *prev;
		BaseBlock *next;
	};

}	 // namespace details
#endif	  // BASE_BLOCK_HPP
