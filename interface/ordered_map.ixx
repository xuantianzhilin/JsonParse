export module json:ordered_map;

import std;

export namespace json
{
	template <typename Key,
		typename T,
		typename Hash = std::hash<Key>,
		typename KeyEqual = std::equal_to<Key>
	> class ordered_map
	{
	public:

		struct Item
		{
			std::pair<Key, T> pair;
			bool valid = true;
		};

		struct Slot
		{
			std::size_t hash{};
			std::size_t index{};
		};

		class iterator
		{
		public:

			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = std::pair<Key, T>;
			using difference_type = std::ptrdiff_t;
			using pointer = value_type*;
			using reference = value_type&;

			explicit iterator(Item* current, const Item* end) : current_(current), end_(end) {}

			reference operator*() const { return current_->pair; }
			pointer operator->() const { return &current_->pair; }

			iterator& operator++()
			{
				do
				{
					++current_;
				} while (current_ != end_ && current_->valid == false);
				return *this;
			}

			iterator operator++(int)
			{
				iterator temp = *this;
				++(*this);
				return temp;
			}

			bool operator==(const iterator& other) const { return current_ == other.current_; }
			bool operator!=(const iterator& other) const { return !(*this == other); }

		private:

			Item* current_;
			const Item* const end_;

			friend class const_iterator;

		};

		class const_iterator
		{
		public:

			using iterator_category = std::forward_iterator_tag;
			using iterator_concept = std::forward_iterator_tag;
			using value_type = std::pair<Key, T>;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type*;
			using reference = const value_type&;

			explicit const_iterator(const Item* current, const Item* end) : current_(current), end_(end) {}
			const_iterator(iterator it) : current_(it.current_), end_(it.end_) {}

			reference operator*() const { return current_->pair; }
			pointer operator->() const { return &current_->pair; }

			const_iterator& operator++()
			{
				do
				{
					++current_;
				} while (current_ != end_ && current_->valid == false);
				return *this;
			}

			const_iterator operator++(int)
			{
				const_iterator temp = *this;
				++(*this);
				return temp;
			}

			bool operator==(const const_iterator& other) const { return current_ == other.current_; }
			bool operator!=(const const_iterator& other) const { return !(*this == other); }

		private:

			const Item* current_;
			const Item* const end_;

		};

		iterator begin() { return iterator{ getStart(), getEnd() }; }
		const_iterator begin() const { return cbegin(); }
		const_iterator cbegin() const { return const_iterator{ getStart(), getEnd() }; }

		iterator end() { return iterator{ getEnd(), getEnd() }; }
		const_iterator end() const { return cend(); }
		const_iterator cend() const { return const_iterator{ getEnd(), getEnd() }; }

		bool empty() const noexcept { return size_ == 0; }
		std::size_t size() const noexcept { return size_; }

		template<typename K>
		iterator find(const K& key)
		{
			const std::size_t hash = getHash(key);
			for (std::size_t i = 0; i < slots_.size(); ++i)
			{
				const std::size_t index = probe(hash, i);
				const Slot& slot = slots_[index];
				if (slot.hash == EMPTY_HASH)
				{
					return end();
				}
				if (slot.hash == hash
					&& slot.index != TOMBSTONE_INDEX
					&& items_[slot.index].valid == true
					&& KeyEqual{}(items_[slot.index].pair.first, key))
				{
					return iterator(&items_[slot.index], getEnd());
				}
			}
			return end();
		}

		template<typename K>
		const_iterator find(const K& key) const
		{
			const std::size_t hash = getHash(key);
			for (std::size_t i = 0; i < slots_.size(); ++i)
			{
				const std::size_t index = probe(hash, i);
				const Slot& slot = slots_[index];
				if (slot.hash == EMPTY_HASH)
				{
					return cend();
				}
				if (slot.hash == hash
					&& slot.index != TOMBSTONE_INDEX
					&& items_[slot.index].valid == true
					&& KeyEqual{}(items_[slot.index].pair.first, key))
				{
					return const_iterator(&items_[slot.index], getEnd());
				}
			}
			return cend();
		}

		template<typename K>
		T& at(const K& x)
		{
			return const_cast<T&>(static_cast<const ordered_map*>(this)->at(x));
		}

		template<typename K>
		const T& at(const K& x) const
		{
			auto it = find(x);
			if (it != cend())
			{
				return it->second;
			}
			else
			{
				throw std::out_of_range("Key not found");
			}
		}

		template<typename P>
		std::pair<iterator, bool> insert(P&& value)
		{
			const std::size_t hash = getHash(value.first);
			std::size_t tombstoneIndex = TOMBSTONE_INDEX;

			for (std::size_t i = 0; i < slots_.size(); ++i)
			{
				const std::size_t index = probe(hash, i);
				Slot* slot = &slots_[index];
				if (slot->hash == EMPTY_HASH)
				{
					slot = tombstoneIndex == TOMBSTONE_INDEX ? slot : &slots_[tombstoneIndex];
					items_.emplace_back(std::forward<P>(value), true);
					slot->hash = hash;
					slot->index = items_.size() - 1;

					++size_;
					if (size_ > slots_.size() / 2)
					{
						rehash(slots_.size() * 2);
					}

					return { iterator(&items_.back(), getEnd()), true };
				}
				if (slot->hash == hash
					&& slot->index != TOMBSTONE_INDEX
					&& items_[slot->index].valid == true
					&& KeyEqual{}(items_[slot->index].pair.first, value.first))
				{
					return { iterator(&items_[slot->index], getEnd()), false };
				}
				if (slot->index == TOMBSTONE_INDEX && tombstoneIndex == TOMBSTONE_INDEX)
				{
					tombstoneIndex = index;
				}
			}
			throw std::runtime_error("Hash table is full");
		}

		template<typename K>
		std::size_t erase(K&& key)
		{
			const std::size_t hash = getHash(key);
			for (std::size_t i = 0; i < slots_.size(); ++i)
			{
				const std::size_t index = probe(hash, i);
				Slot& slot = slots_[index];
				if (slot.hash == EMPTY_HASH)
				{
					return 0;
				}
				if (slot.hash == hash
					&& slot.index != TOMBSTONE_INDEX
					&& items_[slot.index].valid == true
					&& KeyEqual{}(items_[slot.index].pair.first, key))
				{
					items_[slot.index].valid = false;
					slot.index = TOMBSTONE_INDEX;
					--size_;
					return 1;
				}
			}
			return 0;
		}

		void rehash(std::size_t count)
		{
			std::vector<Item> newItems;
			newItems.reserve(size_);

			for (std::size_t i = 0; i < items_.size(); ++i)
			{
				if (items_[i].valid)
				{
					newItems.push_back(std::move(items_[i]));
				}
			}
			items_.swap(newItems);

			count = std::max(count, INITIAL_CAPACITY);
			std::size_t newCapacity = 1;
			while (newCapacity < count)
			{
				newCapacity <<= 1;
			}
			std::vector<Slot> newSlots(newCapacity, { EMPTY_HASH, 0 });

			for (std::size_t i = 0; i < items_.size(); ++i)
			{
				const std::size_t hash = getHash(items_[i].pair.first);
				for (std::size_t j = 0; j < newSlots.size(); ++j)
				{
					const std::size_t index = (hash + j) & (newSlots.size() - 1);
					Slot& slot = newSlots[index];
					if (slot.hash == EMPTY_HASH)
					{
						slot.hash = hash;
						slot.index = i;
						break;
					}
				}
			}
			slots_.swap(newSlots);
		}

		template<typename K>
		T& operator[](K&& key)
		{
			auto it = find(key);
			if (it != end())
			{
				return it->second;
			}
			else
			{
				auto [new_it, inserted] = insert(std::pair<Key, T>{ Key{ std::forward<K>(key) }, T{} });
				return new_it->second;
			}
		}

		void clear() noexcept
		{
			items_.clear();
			slots_.assign(INITIAL_CAPACITY, { EMPTY_HASH, 0 });
			size_ = 0;
		}

	private:

		template <typename K>
		std::size_t getHash(K&& key) const noexcept
		{
			std::size_t hash = Hash{}(key);
			return hash == EMPTY_HASH ? 1 : hash;
		}

		std::size_t probe(std::size_t hash, std::size_t i) const noexcept
		{
			return (hash + i) & (slots_.size() - 1);
		}

		Item* getStart() noexcept
		{
			for (std::size_t i = 0; i < items_.size(); ++i)
			{
				if (items_[i].valid)
				{
					return &items_[i];
				}
			}
			return getEnd();
		}

		const Item* getStart() const noexcept
		{
			for (std::size_t i = 0; i < items_.size(); ++i)
			{
				if (items_[i].valid)
				{
					return &items_[i];
				}
			}
			return getEnd();
		}

		Item* getEnd() noexcept
		{
			return items_.data() + items_.size();
		}

		const Item* getEnd() const noexcept
		{
			return items_.data() + items_.size();
		}

		std::vector<Item> items_;
		std::vector<Slot> slots_ = std::vector<Slot>(INITIAL_CAPACITY);

		std::size_t size_ = 0;

		static constexpr std::size_t EMPTY_HASH = 0;
		static constexpr std::size_t TOMBSTONE_INDEX = std::numeric_limits<std::size_t>::max();
		static constexpr std::size_t INITIAL_CAPACITY = 16;
	};
}