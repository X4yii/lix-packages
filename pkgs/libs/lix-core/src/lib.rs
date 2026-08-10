#![cfg_attr(not(test), no_std)]
#[cfg(test)]
extern crate std;
extern crate alloc;
use alloc::vec::Vec;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct MemoryBuffer {
    data: Vec<u8>,
}

impl MemoryBuffer {
    pub fn new() -> Self {
        Self { data: Vec::new() }
    }

    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            data: Vec::with_capacity(capacity),
        }
    }

    pub fn from_slice(slice: &[u8]) -> Self {
        let mut data = Vec::with_capacity(slice.len());
        data.extend_from_slice(slice);
        Self { data }
    }

    pub fn as_slice(&self) -> &[u8] {
        &self.data
    }

    pub fn as_mut_slice(&mut self) -> &mut [u8] {
        &mut self.data
    }

    pub fn len(&self) -> usize {
        self.data.len()
    }

    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    pub fn append(&mut self, bytes: &[u8]) {
        self.data.extend_from_slice(bytes);
    }

    pub fn clear(&mut self) {
        self.data.clear();
    }
}

impl Default for MemoryBuffer {
    fn default() -> Self {
        Self::new()
    }
}

pub struct ByteReader<'a> {
    data: &'a [u8],
    offset: usize,
}

impl<'a> ByteReader<'a> {
    pub const fn new(data: &'a [u8]) -> Self {
        Self { data, offset: 0 }
    }

    pub fn remaining(&self) -> usize {
        self.data.len().saturating_sub(self.offset)
    }

    pub fn read_u8(&mut self) -> Option<u8> {
        if self.offset < self.data.len() {
            let b = self.data[self.offset];
            self.offset += 1;
            Some(b)
        } else {
            None
        }
    }

    pub fn read_u16_le(&mut self) -> Option<u16> {
        let bytes = self.read_bytes(2)?;
        Some(u16::from_le_bytes([bytes[0], bytes[1]]))
    }

    pub fn read_u32_le(&mut self) -> Option<u32> {
        let bytes = self.read_bytes(4)?;
        Some(u32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]))
    }

    pub fn read_u64_le(&mut self) -> Option<u64> {
        let bytes = self.read_bytes(8)?;
        Some(u64::from_le_bytes([
            bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7],
        ]))
    }

    pub fn read_bytes(&mut self, count: usize) -> Option<&'a [u8]> {
        if self.offset + count <= self.data.len() {
            let slice = &self.data[self.offset..self.offset + count];
            self.offset += count;
            Some(slice)
        } else {
            None
        }
    }
}

pub struct ByteWriter {
    buffer: Vec<u8>,
}

impl ByteWriter {
    pub fn new() -> Self {
        Self { buffer: Vec::new() }
    }

    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            buffer: Vec::with_capacity(capacity),
        }
    }

    pub fn write_u8(&mut self, val: u8) {
        self.buffer.push(val);
    }

    pub fn write_u16_le(&mut self, val: u16) {
        self.buffer.extend_from_slice(&val.to_le_bytes());
    }

    pub fn write_u32_le(&mut self, val: u32) {
        self.buffer.extend_from_slice(&val.to_le_bytes());
    }

    pub fn write_u64_le(&mut self, val: u64) {
        self.buffer.extend_from_slice(&val.to_le_bytes());
    }

    pub fn write_bytes(&mut self, bytes: &[u8]) {
        self.buffer.extend_from_slice(bytes);
    }

    pub fn finish(self) -> Vec<u8> {
        self.buffer
    }
}

impl Default for ByteWriter {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_memory_buffer() {
        let mut buf = MemoryBuffer::new();
        buf.append(b"LixOS");
        assert_eq!(buf.len(), 5);
        assert_eq!(buf.as_slice(), b"LixOS");
    }

    #[test]
    fn test_byte_writer_reader_roundtrip() {
        let mut writer = ByteWriter::new();
        writer.write_u8(42);
        writer.write_u16_le(0x1234);
        writer.write_u32_le(0x89abcdef);
        writer.write_u64_le(0x0123456789abcdef);
        writer.write_bytes(b"LIX");
        let bytes = writer.finish();

        let mut reader = ByteReader::new(&bytes);
        assert_eq!(reader.read_u8(), Some(42));
        assert_eq!(reader.read_u16_le(), Some(0x1234));
        assert_eq!(reader.read_u32_le(), Some(0x89abcdef));
        assert_eq!(reader.read_u64_le(), Some(0x0123456789abcdef));
        assert_eq!(reader.read_bytes(3), Some(&b"LIX"[..]));
        assert_eq!(reader.remaining(), 0);
    }
}
