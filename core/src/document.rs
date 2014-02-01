use std::str;
use std::vec;
use std::ptr;

static INITIAL_GAP_SIZE: uint = 128;

struct DocumentIterator<'a> {
  priv document: &'a mut Document<'a>,
  priv pos: uint
}

impl<'a> Iterator<u8> for DocumentIterator<'a> {
  fn next(&mut self) -> Option<u8> {
    if self.pos < self.document.len() {
      self.pos += 1;
      Some(self.document.get(self.pos - 1))
    } else {
      None
    }
  }
}

pub trait Editable {
  fn insert(&mut self, offset: uint, byte: u8);

  fn delete(&mut self, offset: uint);

  fn subscribe(&mut self, subscriber: fn() -> ());

  fn len(&self) -> uint;

  fn get(&self, index: uint) -> u8;
}

// Abstract an internal buffer implementation since we may change it in the future.
struct Document<'a> {
  priv buffer: ~GapBuffer
}

impl<'a> Document<'a> {
  fn new<'a>(initial_text: &str) -> Document<'a> {
    Document { buffer: ~GapBuffer::new(initial_text) }
  }

  fn iter(&'a mut self) -> DocumentIterator<'a> {
    //self.buffer.iter()
    DocumentIterator { document: self, pos: 0 }
  }
}

impl<'a> Editable for Document<'a> {
  fn insert(&mut self, offset: uint, byte: u8) {
    self.buffer.insert(offset, byte);
  }

  fn delete(&mut self, offset: uint) {
    self.buffer.delete(offset);
  }

  fn subscribe(&mut self, subscriber: fn() -> ()) {
    self.buffer.subscribe(subscriber);
  }

  fn len(&self) -> uint {
    self.buffer.len()
  }

  fn get(&self, index: uint) -> u8 {
    self.buffer.get(index)
  }
}

struct GapBuffer {
  priv buffer: ~[u8],
  priv gap_offset: uint,
  priv gap_size: uint,
  priv subscribers: ~[fn() -> ()]
}

impl GapBuffer {
  fn new(initial_text: &str) -> GapBuffer {
    debug!("INITIAL_GAP_SIZE: {}", INITIAL_GAP_SIZE);
    debug!("initial_text.len(): {}", initial_text.len());
    let mut gap_buffer = GapBuffer {
      buffer: vec::with_capacity(initial_text.len() + INITIAL_GAP_SIZE),
      gap_offset: 0,
      gap_size: INITIAL_GAP_SIZE,
      subscribers: vec::with_capacity(1)
    };

    unsafe {
      gap_buffer.buffer.set_len(initial_text.len() + INITIAL_GAP_SIZE);
    };

    {
      debug!("gap_buffer.gap_size: {}", gap_buffer.gap_size);
      debug!("gap_buffer.buffer.len(): {}", gap_buffer.buffer.len());

      let slice = gap_buffer.buffer.mut_slice_from(gap_buffer.gap_size);
      unsafe {
        slice.copy_memory(initial_text.as_bytes());
      };
    }
    gap_buffer
  }

  fn confirm_gap(&mut self, new_gap_offset: uint) {
    if self.gap_size == 0 {
      self.gap_offset = self.buffer.len();
      self.buffer.grow(INITIAL_GAP_SIZE, &0u8);
      self.gap_size = INITIAL_GAP_SIZE;
    }

    if new_gap_offset < self.gap_offset {
      let len = self.gap_offset - new_gap_offset;
      self.copy(new_gap_offset, new_gap_offset + self.gap_size, len);
    } else {
      let len = new_gap_offset - self.gap_offset;
      self.copy(self.gap_offset + self.gap_size, self.gap_offset, len);
    }
    self.gap_offset = new_gap_offset;
  }

  fn copy(&mut self, src_pos: uint, dest_pos: uint, len: uint) {
    unsafe {
      let dst_ptr = self.buffer.unsafe_mut_ref(dest_pos);
      let src_ptr = self.buffer.unsafe_mut_ref(src_pos);
      ptr::copy_memory(dst_ptr, src_ptr, len);
    }
  }

  fn notify_change(&self) {
    for s in self.subscribers.iter() {
      (*s)();
    }
  }
}

impl Editable for GapBuffer {
  fn insert(&mut self, offset: uint, byte: u8) {
    self.confirm_gap(offset);
    self.buffer[offset] = byte;
    self.gap_offset += 1;
    self.gap_size -= 1;
    self.notify_change();
  }

  fn delete(&mut self, offset: uint) {
    if (self.len() == 0) { return };

    self.confirm_gap(offset + 1);
    self.gap_offset -= 1;
    self.gap_size += 1;
    self.notify_change();
  }

  fn subscribe(&mut self, subscriber: fn() -> ()) {
    self.subscribers.push(subscriber);
  }

  fn len(&self) -> uint {
    self.buffer.len() - self.gap_size
  }

  fn get(&self, index: uint) -> u8 {
    if index < self.gap_offset {
      self.buffer[index]
    } else {
      self.buffer[self.gap_size + index]
    }
  }
}

mod tests {
  use std::str;
  use std::vec;
  // TODO: Why do these need to import explicitly?
  use super::GapBuffer;
  use super::Document;
  use super::Editable;
  use super::INITIAL_GAP_SIZE;

  #[test]
  fn test_new() {
    let str = "abc";
    let buf = GapBuffer::new(str);
    assert_eq!(buf.gap_offset, 0);
    assert_eq!(buf.gap_size, INITIAL_GAP_SIZE);
    assert_eq!(buf.len(), str.len());
    assert_eq!(str::from_utf8(buf.buffer.slice_from(buf.gap_size)), str);
  }

  #[test]
  fn test_copy() {
    let str = "abc";
    let mut buf = GapBuffer::new(str);

    buf.copy(buf.gap_offset + buf.gap_size, buf.gap_offset, str.len());

    assert_eq!(str::from_utf8(buf.buffer.slice(0, str.len())), str);
  }

  #[test]
  fn test_insert() {
    let str = "abc";
    let mut buf = GapBuffer::new(str);

    buf.insert(0, 'd'.to_ascii().to_byte());

    assert_eq!(buf.gap_offset, 1);
    assert_eq!(buf.gap_size, INITIAL_GAP_SIZE - 1);
    assert_eq!(buf.len(), str.len() + 1);
    assert_eq!(str::from_utf8(buf.buffer.slice(0, 1)), "d");
  }

  #[test]
  fn test_delete() {
    let str = "abc";
    let mut buf = GapBuffer::new(str);

    buf.insert(0, 'd'.to_ascii().to_byte());
    buf.insert(0, 'e'.to_ascii().to_byte());
    buf.delete(0);

    assert_eq!(buf.gap_offset, 0);
    assert_eq!(buf.gap_size, INITIAL_GAP_SIZE - 1);
    assert_eq!(buf.len(), str.len() + 1);
    assert_eq!(str::from_utf8(buf.buffer.slice(0, 1)), "e");
  }

  #[test]
  fn test_len() {
    let str = "abc";
    let mut buf = GapBuffer::new(str);

    assert_eq!(buf.len(), 3);

    buf.insert(0, 'd'.to_ascii().to_byte());
    buf.insert(0, 'e'.to_ascii().to_byte());
    buf.delete(0);

    assert_eq!(buf.len(), 4);
  }

  #[test]
  fn test_get() {
    let str = "abc";
    let buf = GapBuffer::new(str);

    assert_eq!(buf.get(0).to_ascii().to_char(), 'a');
  }

  #[test]
  fn test_iter() {
    let str = "abc";
    let mut doc = Document::new(str);

    let mut index = 0;
    for ch in doc.iter() {
      assert_eq!(ch, str[index]);
      index += 1;
    }
  }
}

