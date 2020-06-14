#ifndef DUCKOS_MEMORYBITMAP_HPP
#define DUCKOS_MEMORYBITMAP_HPP

#include <kernel/kstddef.h>
#include <kernel/kstdio.h>

template<int total_pages>
class MemoryBitmap {
public:
	MemoryBitmap() {
		//Doing this because the compiler doesn't like the array initializer ¯\_(ツ)_/¯
		//FIXME: Take a look at this again, may be an issue w/ toolchain
		for(auto & b : bitmap) {
			b = 0;
		}
	}

	void reset_bitmap() {
		for(auto & b : bitmap) {
			b = 0;
		}
	}

	size_t find_pages(size_t num_pages, size_t start_index) {
		//Calculate number of pages (round-up division)
		if(num_pages == 1) return find_one_page(start_index);
		size_t cur_page = find_one_page(start_index);
		while(true) {
			//Check if the chunk of pages starting at cur_page are free.
			bool all_free = true;
			auto page = find_one_page(cur_page) + 1;
			for(;page - cur_page < num_pages; page++) {
				if(is_page_used(page)){
					all_free = false;
					break;
				}
			}

			//If the all_free flag is set, return the first page of the chunk we found.
			if(all_free)
				return cur_page;

			//Otherwise, start the next iteration of the loop at the next page after that chunk we just checked.
			cur_page = find_one_page(page);

			//If we've made it past the end of the memory bitmap, return -1. No memory available.
			if(cur_page + num_pages >= total_pages) {
				return -1;
			}
		}
	}

	size_t find_one_page(size_t start_index){
		uint8_t start_bit = start_index % 32;

		for(size_t bitmap_index = start_index / 32; bitmap_index < total_pages/32; bitmap_index++) {
			for(auto bit = start_bit; bit < 32; bit++) {
				if(!((bitmap[bitmap_index] >> bit) & 1u)) {
					return bitmap_index * 32 + bit;
				}
			}
			start_bit = 0;
		}
		return -1;
	}

	bool is_page_used(size_t page){
		return (bool)((bitmap[page / 32] >> (page % 32u)) & 1u);
	}

	void set_page_used(size_t page){
		_used_pages += 1;
		bitmap[page / 32] |= 1u << (page % 32u);
	}

	void set_page_free(size_t page){
		_used_pages -= 1;
		bitmap[page / 32] &= ~(1u << (page % 32u));
	}

	size_t allocate_pages(size_t num_pages, size_t start_index){
		size_t page = find_pages(num_pages, start_index);
		if(page) {
			for(size_t sPage = page; sPage - page < num_pages; sPage++) {
				set_page_used(sPage);
			}
		}
		return page;
	}

	size_t used_pages(){
		return _used_pages;
	}

private:
	uint32_t bitmap[total_pages / 32];
	size_t _used_pages = 0;
};


#endif //DUCKOS_MEMORYBITMAP_HPP
