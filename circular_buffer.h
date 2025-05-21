#include <memory>
#include <utility>

template <typename T, std::size_t S>
class circular_buffer {
public:
    typedef T                                       value_type;
    typedef T*                                      pointer;
    typedef const T*                                const_pointer;

    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    // typedef value_type*                             iterator;
    // typedef const value_type*                       const_iterator;
    typedef std::size_t                             size_type;
    typedef std::ptrdiff_t                          difference_type;
    // typedef std::reverse_iterator<iterator>         reverse_iterator;
    // typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

private:
    value_type buf[S];
    pointer buf_end = buf + S;
    pointer head = buf, tail = buf;
    size_type cur_size = 0;

public:
    struct block {
        pointer data;
        size_type size;
    };

    reference at(size_type n) {
        if (n >= cur_size)
            throw std::out_of_range("circular_buffer::at");

        pointer p = head + n;
        if (p >= buf_end)
            p -= S;
        return *p;
    }

    const_reference at(size_type n) const {
        if (n >= cur_size)
            throw std::out_of_range("circular_buffer::at");
            
        pointer p = head + n;
        if (p >= buf_end)
            p -= S;
        return *p;
    }

    reference operator[](size_type n) {
        pointer p = head + n;
        if (p >= buf_end)
            p -= S;
        return *p;
    }

    const_reference operator[](size_type n) const {
        pointer p = head + n;
        if (p >= buf_end)
            p -= S;
        return *p;
    }

    reference front() {
        return *head;
    }
    
    const_reference front() const {
        return *head;
    }

    reference back() {
        pointer p = head + cur_size - 1;
        if (p >= buf_end)
            p -= S;
        return *p;
    }

    const_reference back() const {
        pointer p = head + cur_size - 1;
        if (p >= buf_end)
            p -= S;
        return *p;
    }

    bool empty() const {
        return !cur_size;
    }

    size_type size() const {
        return cur_size;
    }

    size_type max_size() const {
        return S;
    }

    void fill(const_reference value) {
        for (size_type i = 0; i < S; ++i)
            buf[i] = value;

        head = buf;
        tail = buf;
        cur_size = S;
    }

    void clear() {
        head = buf;
        tail = buf;
        cur_size = 0;
    }

    std::pair<const block, const block> get_free_blocks() {
        if (cur_size == S) {
            // Buffer is full, no free space
            return {block{nullptr, 0}, block{nullptr, 0}};
        }

        if (tail >= head) {
            // Case: [----head....tail----]
            size_type space_at_end = buf_end - tail;
            size_type space_at_start = head - buf;

            if (!space_at_start) {
                // Single block
                return {block{tail, space_at_end}, block{nullptr, 0}};
            } else {
                // Split across end and start
                return {
                    block{tail, space_at_end},
                    block{buf, space_at_start}
                };
            }
        } else {
            // Case: [....tail----head....]
            // Single contiguous free block from tail to head-1
            return {block{tail, static_cast<size_type>(head - tail)}, block{nullptr, 0}};
        }
    }

    std::pair<const block, const block> get_used_blocks() {
        if (cur_size == 0) {
            // Buffer is empty
            return {block{nullptr, 0}, block{nullptr, 0}};
        }

        if (head < tail) {
            // Single block: [....head----tail....]
            return {block{head, cur_size}, block{nullptr, 0}};
        } else {
            // Wrapped: [----tail....head----]
            size_type size_to_end = buf_end - head;
            size_type size_from_start = tail - buf;

            if (!size_from_start) {
                return {block{head, size_to_end}, block{nullptr, 0}};
            } else {
                return {
                    block{head, size_to_end},
                    block{buf, size_from_start}
                };
            }
        }
    }

    void add_used_bytes(size_type n) {
        tail += n;
        if (tail >= buf_end)
            tail -= S;
        cur_size += n;
    }

    void remove_used_bytes(size_type n) {
        head += n;
        if (head >= buf_end)
            head -= S;
        cur_size -= n;
    }
};
