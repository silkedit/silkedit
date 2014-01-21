use std::str;
use std::vec;
use std::ptr;

static INITIAL_GAP_SIZE: uint = 128;

pub struct GapBuffer {
  priv buffer: ~[u8],
  priv gap_offset: uint,
  priv gap_size: uint
}

impl GapBuffer {
  pub fn new(initial_text: &str) -> GapBuffer {

    debug!("INITIAL_GAP_SIZE: {}", INITIAL_GAP_SIZE);
    debug!("initial_text.len(): {}", initial_text.len());
    let mut gap_buffer = GapBuffer {
      buffer: vec::with_capacity(initial_text.len() + INITIAL_GAP_SIZE),
      gap_offset: 0,
      gap_size: INITIAL_GAP_SIZE
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

  fn insert(&mut self, offset: uint, byte: u8) {
    self.confirm_gap(offset);
    self.buffer[offset] = byte;
    self.gap_offset += 1;
    self.gap_size -= 1;
  }

  fn delete(&mut self, offset: uint) {
    if (self.len() == 0) { return };

    self.confirm_gap(offset + 1);
    self.gap_offset -= 1;
    self.gap_size += 1;
  }

  fn len(& self) -> uint {
    self.buffer.len() - self.gap_size
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
}

mod tests {
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
}

