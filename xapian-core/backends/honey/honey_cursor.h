/** @file honey_cursor.h
 * @brief HoneyCursor class
 */
/* Copyright (C) 2017,2018 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_HONEY_CURSOR_H
#define XAPIAN_INCLUDED_HONEY_CURSOR_H

#include "honey_table.h"

class HoneyCursor {
    /** Search for @a key.
     *
     *  If @a key isn't present, the behaviour depends on @a greater_than.
     *  If it's true, then the cursor will be left on the first key >
     *  @a key; otherwise it may be left at an unspecified position.
     */
    bool do_find(const std::string& key, bool greater_than);

    /** Handle the value part of the (key,value). */
    bool next_from_index();

  public:
    BufferedFile store;
    std::string current_key, current_tag;
    mutable size_t val_size = 0;
    bool current_compressed = false;
    mutable CompressionStream comp_stream;
    bool is_at_end = false;
    mutable std::string last_key;

    // File offset to start of index and to current position in index.
    off_t root, index;

    // File offset to start of table (zero except for single-file DB).
    off_t offset;

    // Forward to next constructor form.
    explicit HoneyCursor(const HoneyTable* table)
	: HoneyCursor(table->store, table->get_root(), table->get_offset()) {}

    HoneyCursor(const BufferedFile& store_, off_t root_, off_t offset_)
	: store(store_),
	  comp_stream(Z_DEFAULT_STRATEGY),
	  root(root_),
	  index(root_),
	  offset(offset_)
    {
	store.set_pos(offset); // FIXME root
    }

    HoneyCursor(const HoneyCursor& o)
	: store(o.store),
	  current_key(o.current_key),
	  current_tag(o.current_tag), // FIXME really copy?
	  val_size(o.val_size),
	  current_compressed(o.current_compressed),
	  comp_stream(Z_DEFAULT_STRATEGY),
	  is_at_end(o.is_at_end),
	  last_key(o.last_key),
	  root(o.root),
	  index(o.index),
	  offset(o.offset)
    {
	store.set_pos(o.store.get_pos());
    }

    /** Position cursor on the dummy empty key.
     *
     *  Calling next() after this moves the cursor to the first entry.
     */
    void rewind() {
	store.set_pos(offset); // FIXME root
	current_key = last_key = std::string();
	is_at_end = false;
	index = root;
	val_size = 0;
    }

    void to_end() { is_at_end = true; }

    bool after_end() const { return is_at_end; }

    bool next();

    bool read_tag(bool keep_compressed = false);

    bool find_exact(const std::string& key) {
	return do_find(key, false);
    }

    bool find_entry_ge(const std::string& key) {
	return do_find(key, true);
    }

    /** Move to the item before the current one.
     *
     *  If the cursor is after_end(), this moves to the last item.
     *
     *  If the cursor is at the start of the table (on the empty key), do
     *  nothing and return false, otherwise return true.
     *
     *  This method may not be particularly efficient.
     */
    bool prev();

    HoneyCursor * clone() const {
	return new HoneyCursor(*this);
    }

    bool del() { return false; }
};

class MutableHoneyCursor : public HoneyCursor {
  public:
    MutableHoneyCursor(HoneyTable* table_)
	: HoneyCursor(table_->store, table_->get_root(), table_->get_offset())
    { }
};

#endif // XAPIAN_INCLUDED_HONEY_CURSOR_H
